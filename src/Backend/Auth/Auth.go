package Auth
import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"net/http"
	"net/url"
	"strings"
	"time"
)

var httpClient = &http.Client{Timeout: 15 * time.Second}

// RequestDeviceCode starts the device flow. scope can be "" (public info only).
func RequestDeviceCode(ctx context.Context, clientID, scope string) (*DeviceCode, error) {
	form := url.Values{"client_id": {clientID}}
	if scope != "" {
		form.Set("scope", scope)
	}

	var out struct {
		DeviceCode
		Error            string `json:"error"`
		ErrorDescription string `json:"error_description"`
	}
	if err := postForm(ctx, deviceCodeURL, form, &out); err != nil {
		return nil, err
	}
	if out.Error != "" {
		return nil, fmt.Errorf("github device code: %s: %s", out.Error, out.ErrorDescription)
	}
	if out.Interval <= 0 {
		out.Interval = 5
	}
	return &out.DeviceCode, nil
}

// PollForToken blocks until the user approves (or denies / the code expires /
// ctx is cancelled) and returns the access token.
func PollForToken(ctx context.Context, clientID string, dc *DeviceCode) (string, error) {
	interval := time.Duration(dc.Interval) * time.Second
	deadline := time.Now().Add(time.Duration(dc.ExpiresIn) * time.Second)

	form := url.Values{
		"client_id":   {clientID},
		"device_code": {dc.DeviceCode},
		"grant_type":  {"urn:ietf:params:oauth:grant-type:device_code"},
	}

	for {
		timer := time.NewTimer(interval)
		select {
		case <-ctx.Done():
			timer.Stop()
			return "", ctx.Err()
		case <-timer.C:
		}

		if time.Now().After(deadline) {
			return "", ErrCodeExpired
		}

		var out struct {
			AccessToken      string `json:"access_token"`
			Error            string `json:"error"`
			ErrorDescription string `json:"error_description"`
			Interval         int    `json:"interval"`
		}
		if err := postForm(ctx, tokenURL, form, &out); err != nil {
			return "", err
		}

		switch out.Error {
		case "":
			if out.AccessToken == "" {
				return "", errors.New("github login: empty access token in response")
			}
			return out.AccessToken, nil
		case "authorization_pending":
			// user hasn't approved yet, keep polling
		case "slow_down":
			// GitHub asks us to back off
			if out.Interval > 0 {
				interval = time.Duration(out.Interval) * time.Second
			} else {
				interval += 5 * time.Second
			}
		case "expired_token":
			return "", ErrCodeExpired
		case "access_denied":
			return "", ErrAccessDenied
		default:
			return "", fmt.Errorf("github login: %s: %s", out.Error, out.ErrorDescription)
		}
	}
}

// FetchUser asks GitHub who owns the token. This is the actual "verification":
// GitHub itself vouches for the identity.
func FetchUser(ctx context.Context, token string) (*User, error) {
	req, err := http.NewRequestWithContext(ctx, http.MethodGet, userURL, nil)
	if err != nil {
		return nil, err
	}
	req.Header.Set("Authorization", "Bearer "+token)
	req.Header.Set("Accept", "application/vnd.github+json")
	req.Header.Set("X-GitHub-Api-Version", "2022-11-28")

	resp, err := httpClient.Do(req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode == http.StatusUnauthorized {
		return nil, ErrInvalidToken
	}
	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("github /user: unexpected status %s", resp.Status)
	}

	var u User
	if err := json.NewDecoder(resp.Body).Decode(&u); err != nil {
		return nil, err
	}
	if u.ID == 0 || u.Login == "" {
		return nil, errors.New("github /user: response missing id/login")
	}
	return &u, nil
}

// Login runs the whole flow. onCode is called once so your UI can show the
// code and open the browser, e.g. by showing dc.UserCode and opening
// dc.VerificationURI.
func Login(ctx context.Context, clientID string, onCode func(*DeviceCode)) (token string, user *User, err error) {
	dc, err := RequestDeviceCode(ctx, clientID, "") // no scope needed to verify identity
	if err != nil {
		return "", nil, err
	}
	onCode(dc)

	token, err = PollForToken(ctx, clientID, dc)
	if err != nil {
		return "", nil, err
	}
	user, err = FetchUser(ctx, token)
	if err != nil {
		return "", nil, err
	}
	return token, user, nil
}

func postForm(ctx context.Context, endpoint string, form url.Values, out any) error {
	req, err := http.NewRequestWithContext(ctx, http.MethodPost, endpoint, strings.NewReader(form.Encode()))
	if err != nil {
		return err
	}
	req.Header.Set("Content-Type", "application/x-www-form-urlencoded")
	req.Header.Set("Accept", "application/json") // otherwise GitHub replies form-encoded

	resp, err := httpClient.Do(req)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("%s: unexpected status %s", endpoint, resp.Status)
	}
	return json.NewDecoder(resp.Body).Decode(out)
}