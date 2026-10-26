package Auth
import (
	"errors"
)


// DeviceCode is what GitHub returns when you start the flow.
type DeviceCode struct {
	DeviceCode      string `json:"device_code"`
	UserCode        string `json:"user_code"`        // show this to the user
	VerificationURI string `json:"verification_uri"` // open this in their browser
	ExpiresIn       int    `json:"expires_in"`       // seconds
	Interval        int    `json:"interval"`         // min seconds between polls
}

// User is the verified GitHub identity. Key your own accounts on ID,
// not Login: usernames can be changed, IDs never change.
type User struct {
	ID        int64  `json:"id"`
	Login     string `json:"login"`
	Name      string `json:"name"`
	AvatarURL string `json:"avatar_url"`
	HTMLURL   string `json:"html_url"`
}

const (
	deviceCodeURL = "https://github.com/login/device/code"
	tokenURL      = "https://github.com/login/oauth/access_token"
	userURL       = "https://api.github.com/user"
)

var (
	ErrAccessDenied = errors.New("github login: user denied the request")
	ErrCodeExpired  = errors.New("github login: code expired, please try again")
	ErrInvalidToken = errors.New("github login: token is invalid or revoked")
)