package network

import (
	"fmt"
	"net/http"
	"encoding/json"
)

type Release struct {
	TagName string `json:"tag_name"`
	Name    string `json:"name"`
	URL     string `json:"html_url"`
}





func GetLatest(username string, reponame string) {
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