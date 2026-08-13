package protocol

import (
	"Backend/Network"
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
	Message string `json:"message"`
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
