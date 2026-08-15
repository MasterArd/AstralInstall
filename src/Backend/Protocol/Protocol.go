package protocol

import (
    "Backend/Network"
    "bufio"
    "encoding/json"
    "fmt"
    "os"
)

/*
This will talk to C++ front end through stdin-stdout+JSON
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

func ImportTest() {
    fmt.Println("Protocol was Initialized")
}

func RequestTest() {
	// the following line is just an example
    release, err := network.GetLatest("https://github.com/MasterArd/CGTerm")
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


func writeResponse(response Response) {
    data, err := json.Marshal(response)
    if err != nil {
        // fallback error 
        fmt.Println(`{"success":false,"error":"Failed to encode response JSON"}`)
        return
    }
    
    
    fmt.Println(string(data))
}