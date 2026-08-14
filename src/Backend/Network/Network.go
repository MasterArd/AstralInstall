package network

import (
	"encoding/json"
	"fmt"
	"net/http"
	"regexp"
	//"text/template/parse"
)

type Release struct {
	TagName string `json:"tag_name"`
	Name    string `json:"name"`
	URL     string `json:"html_url"`
}

// Matches: [https://][www.]github.com/{owner}/{repo}[...]
var githubRegex = regexp.MustCompile(`(?:https?://)?(?:www\.)?github\.com/([^/]+)/([^/.]+)(?:\.git)?`)
func ParseWithRegex(rawURL string) (owner, repo string, err error) {
	matches := githubRegex.FindStringSubmatch(rawURL)
	if len(matches) < 3 {
		return "", "", fmt.Errorf("invalid github URL")
	}
	return matches[1], matches[2], nil
}

func ReturnMatches(url string) {
	owner, repo, err := ParseWithRegex("urlname")
	if err == nil {
		fmt.Printf("Owner: %s, Repo: %s\n", owner, repo)
	}
	
}

func GetLatest(username string, reponame string)  {
	resp, err := http.Get(fmt.Sprintf("https://api.github.com/repos/%s/%s/releases/latest", username, reponame))
	if err != nil {
		panic(err)
	}
	defer resp.Body.Close()


	if resp.StatusCode != http.StatusOK {
		panic(fmt.Sprintf("GitHub returned %s", resp.Status))
	}

	var release Release

	if err := json.NewDecoder(resp.Body).Decode(&release); err != nil {
		panic(err)
	}

	fmt.Println("Latest release:", release.TagName)
	fmt.Println("Name:", release.Name)
	fmt.Println("URL:", release.URL)
}



func ImportTest() {
	println("Network was Initialized")
}

func GithubPingTest() {
	resp, err := http.Get("https://github.com")
	if err != nil {
		panic(err)
	}
	defer resp.Body.Close()
	fmt.Println(resp.Status)
}