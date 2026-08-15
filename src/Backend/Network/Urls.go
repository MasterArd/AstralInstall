package network

import (
    "fmt"
    "net/url"
    "strings"
)

// ParseGitHubURL extracts owner and repo from standard github.com urls or API urls
func ParseGitHubURL(rawURL string) (owner, repo string, err error) {
    if !strings.HasPrefix(rawURL, "http://") && !strings.HasPrefix(rawURL, "https://") {
        rawURL = "https://" + rawURL
    }

    u, err := url.Parse(rawURL)
    if err != nil {
        return "", "", fmt.Errorf("invalid URL structure: %w", err)
    }

    host := strings.ToLower(u.Host)
    if host != "github.com" && host != "api.github.com" {
        return "", "", fmt.Errorf("not a GitHub URL")
    }

    path := strings.Trim(u.Path, "/")
    parts := strings.Split(path, "/")

    // Handle api.github.com/repos/{owner}/{repo}
    if host == "api.github.com" {
        if len(parts) < 3 || parts[0] != "repos" || parts[1] == "" || parts[2] == "" {
            return "", "", fmt.Errorf("invalid API endpoint structure")
        }
        return parts[1], strings.TrimSuffix(parts[2], ".git"), nil
    }

    // Handle standard github.com/{owner}/{repo}
    if len(parts) < 2 || parts[0] == "" || parts[1] == "" {
        return "", "", fmt.Errorf("invalid GitHub repository URL")
    }

    return parts[0], strings.TrimSuffix(parts[1], ".git"), nil
}