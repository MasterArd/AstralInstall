package network

import (
    "Backend/Common"
    "encoding/json"
    "fmt"
    "net/http"
)

// internal struct to decode GitHub's JSON payload
type githubReleaseResponse struct {
    TagName string `json:"tag_name"`
    Name    string `json:"name"`
    URL     string `json:"html_url"`
}

// GetLatest() returns the release struct and an error instead of panicking
func GetLatest(url string) (common.Release, error) {
    owner, repo, err := ParseGitHubURL(url)
    if err != nil {
        return common.Release{}, err
    }

    apiURL := fmt.Sprintf("https://api.github.com/repos/%s/%s/releases/latest", owner, repo)
    resp, err := http.Get(apiURL)
    if err != nil {
        return common.Release{}, err
    }
    defer resp.Body.Close()

    if resp.StatusCode != http.StatusOK {
        return common.Release{}, fmt.Errorf("GitHub returned %s", resp.Status)
    }

    var ghResp githubReleaseResponse
    if err := json.NewDecoder(resp.Body).Decode(&ghResp); err != nil {
        return common.Release{}, err
    }

    
    release := common.Release{
        Tag:     ghResp.TagName,
        Name:    ghResp.Name,
        Version: ghResp.TagName,
    }

    return release, nil
}

func ImportTest() {
    fmt.Println("Network was Initialized")
}

func GithubPingTest() {
    resp, err := http.Get("https://github.com")
    if err != nil {
        panic(err)
    }
    defer resp.Body.Close()
    fmt.Println(resp.Status)
}