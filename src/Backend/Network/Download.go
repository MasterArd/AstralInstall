package network

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"os"
	"strings"
	"time"
)

type Release struct {
	TagName string  `json:"tag_name"`
	Assets  []Asset `json:"assets"`
}

type Asset struct {
	Name               string `json:"name"`
	BrowserDownloadURL string `json:"browser_download_url"`
}

// isSourceArchive filters out GitHub's auto-generated / commonly-uploaded
// source archive naming patterns, not actual release binaries.
func isSourceArchive(name string) bool {
	lower := strings.ToLower(name)
	return strings.HasSuffix(lower, ".zip") ||
		strings.HasSuffix(lower, ".tar.gz") ||
		strings.HasSuffix(lower, ".tgz")
}

// pickAsset returns the first asset that isn't a source archive and,
// if platformHint is non-empty, whose name contains it (e.g. "linux",
// "windows", "darwin"). Returns nil if nothing matches.
func pickAsset(assets []Asset, platformHint string) *Asset {
	hint := strings.ToLower(platformHint)
	for i := range assets {
		a := &assets[i]
		if isSourceArchive(a.Name) {
			continue
		}
		if hint != "" && !strings.Contains(strings.ToLower(a.Name), hint) {
			continue
		}
		return a
	}
	return nil
}


func Download(quickurl, platformHint, destDir string) error {
	owner, repo, err := ParseGitHubURL(quickurl); if err != nil {
		fmt.Errorf("Parsing error: %w", err)
	}
	url := fmt.Sprintf("https://api.github.com/repos/%s/%s/releases/latest", owner, repo)
	client := &http.Client{Timeout: 10 * time.Second}

	req, err := http.NewRequest("GET", url, nil)
	if err != nil {
		return fmt.Errorf("building request: %w", err)
	}
	req.Header.Set("User-Agent", "AstralInstall-Release-Downloader")
	req.Header.Set("Accept", "application/vnd.github+json")

	resp, err := client.Do(req)
	if err != nil {
		return fmt.Errorf("fetching release info: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("fetching release info: status %s", resp.Status)
	}

	var release Release
	if err := json.NewDecoder(resp.Body).Decode(&release); err != nil {
		return fmt.Errorf("decoding release json: %w", err)
	}

	if len(release.Assets) == 0 {
		return fmt.Errorf("no assets found for release %s", release.TagName)
	}

	target := pickAsset(release.Assets, platformHint)
	if target == nil {
		return fmt.Errorf("no matching non-source asset found for release %s (platformHint=%q)", release.TagName, platformHint)
	}

	if destDir == "" {
		destDir = "."
	}
	if err := os.MkdirAll(destDir, 0o755); err != nil {
		return fmt.Errorf("creating dest dir: %w", err)
	}
	destPath := destDir + string(os.PathSeparator) + target.Name

	fmt.Printf("Latest release: %s\n", release.TagName)
	fmt.Printf("Downloading: %s\n", target.Name)

	if err := downloadFile(destPath, target.BrowserDownloadURL); err != nil {
		return fmt.Errorf("downloading asset: %w", err)
	}

	fmt.Println("Download complete.")
	return nil
}

func downloadFile(filepath string, url string) error {
	resp, err := http.Get(url)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("bad status downloading asset: %s", resp.Status)
	}

	out, err := os.Create(filepath)
	if err != nil {
		return err
	}
	defer out.Close()

	_, err = io.Copy(out, resp.Body)
	return err
}