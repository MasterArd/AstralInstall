package protocol

import (
	auth "Backend/Auth"
	network "Backend/Network"
	"bufio"
	"context"
	"encoding/json"
	"fmt"
	"log"
	"os"
)

/*
This will talk to C++ front end through stdin-stdout+JSON
*/
type Request struct {
	Action      string `json:"action"`
	Repo        string `json:"repo"`
	Destination string `json:"destination"`
	Platform    string `json:"platform"`
}

type Response struct {
	Success bool   `json:"success"`
	Version string `json:"version,omitempty"`
	Error   string `json:"error,omitempty"`
}

type LoginResponse struct {
	Success bool `json:"success"`
	Error   string `json:"error,omitempty"`
	URI string `json:"URL"`
	UserCode string `json:"UserCode"`
}


func ImportTest() {
	fmt.Println("Protocol was Initialized")
}

func RequestTest() {
	// the following line is just an example
	release, err := network.GetLatest("https://github.com/hannes-swd/code-miner")
	if err != nil {
		fmt.Println("Error:", err)
		return
	}
	fmt.Println("RequestTest Version:", release.Version)
}

// FrontendLineReader starts the infinite loop listening to standard input for C++ commands
func FrontendLineReader() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		var request Request
		rawBytes := scanner.Bytes()

		// skip empty lines if c++ sends blank line
		if len(rawBytes) == 0 {
			continue
		}

		if err := json.Unmarshal(rawBytes, &request); err != nil {
			writeResponse(Response{
				Success: false,
				Error:   "Invalid JSON: " + err.Error(),
			})
			continue
		}

		switch request.Action {
		case "check_release":
			handleCheckRelease(request)
		case "download_newest":
			handleDownloadRelease(request)
		case "login":
			handleLogin()
		default:
			writeResponse(Response{
				Success: false,
				Error:   "Unknown action: " + request.Action,
			})
		}
	}

	// if scanner stopped because of crash/error
	if err := scanner.Err(); err != nil {
		fmt.Fprintf(os.Stderr, "Backend stdin reading error: %v\n", err)
	}
}

func handleCheckRelease(request Request) {
	if request.Repo == "" {
		writeResponse(Response{Success: false, Error: "repo field is required"})
		return
	}

	release, err := network.GetLatest(request.Repo)
	if err != nil {
		writeResponse(Response{
			Success: false,
			Error:   err.Error(),
		})
		return
	}

	writeResponse(Response{
		Success: true,
		Version: release.Version,
	})
}
func handleLogin() {
	ctx := context.Background()
	token, user, err := auth.Login(ctx, "Ov23liUUhChOmyys6qVH", func(dc *auth.DeviceCode) {
		loginWriteResponse(LoginResponse{
			Success: true,
			Error: "",
			URI: dc.VerificationURI,
			UserCode: dc.UserCode,
		})
	})
	if err != nil {
		log.Fatal(err)
	}
	fmt.Println("Logged in as", user.Login, "id", user.ID, token)

}

func handleDownloadRelease(request Request) {

	if request.Repo == "" {
		writeResponse(Response{Success: false, Error: "repo field is required"})
		return
	}

	err := network.Download(request.Repo, request.Platform, request.Destination)
	if err != nil {
		writeResponse(Response{
			Success: false,
			Error:   err.Error(),
		})
		return
	}
	writeResponse(Response{
		Success: true,
	})
}

func writeResponse(response Response) {
	data, err := json.Marshal(response)
	if err != nil {
		// fallback error
		fmt.Println(`{"success":false,"error":"Failed to encode response JSON"}`)
		return
	}

	fmt.Println(string(data))
}

func loginWriteResponse(response LoginResponse) {
	data, err := json.Marshal(response)
	if err != nil {
		// fallback error
		fmt.Println(`{"success":false,"error":"Failed to encode response JSON"}`)
		return
	}

	fmt.Println(string(data))
}
