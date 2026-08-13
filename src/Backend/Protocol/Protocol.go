package protocol

import (
	"Backend/Network"
	"bufio"
	"encoding/json"
	"fmt"
	"os"
)
/*
This will talk to C++ front end through stdin-stdout/JSON

*/
type Request struct {
	Action string `json:"action"`
	Repo   string `json:"repo"`
}

type Response struct {
	Success bool   `json:"success"`
	Version string `json:"version,omitempty"`
	Error   string `json:"error,omitempty"`
}

type Release struct {
	Tag     string
	Name    string
	Version string
	Assets  []string
}



func ImportTest() {
	println("Protocol was Initialized")
}


func RequestTest() {
	network.GetLatest("MasterArd", "CGTerm")
}

func Talk() {
	scanner := bufio.NewScanner(os.Stdin)

	for scanner.Scan() {
		var request Request

		if err := json.Unmarshal(scanner.Bytes(), &request); err != nil {
			writeResponse(Response{
				Error: err.Error(),
			})
			continue
		}

		if request.Action == "check_release" {
			//network.GetLatest();
			writeResponse(Response{
				Success: true,
				Version: "1.0.0",
			})
		}
	}
}

func writeResponse(response Response) {
	data, _ := json.Marshal(response)
	fmt.Println(string(data))
}