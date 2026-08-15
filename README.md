# AstralInstall

_A lightweight, cross-platform application installer and updater for GitHub projects._

Built with **C++ (Qt/QML)** for the frontend and **Go** for the backend, AstralInstall provides an elegant interface for discovering, installing, and managing GitHub-hosted applications.

## Features

- 🎨 Modern Qt/QML-based user interface
- ⚡ Fast Go backend for network operations
- 📦 Automatic GitHub release detection and updates
- 🔧 JSON-based IPC for seamless frontend-backend communication
- 📋 Game/application library management

## Architecture

AstralInstall uses a **process-based IPC (Inter-Process Communication)** architecture:

```
┌─────────────────────────┐
│   C++ Frontend (Qt/QML) │
│   User Interface Layer  │
└────────────┬────────────┘
             │ stdin/stdout (JSON)
             ▼
┌─────────────────────────┐
│    Go Backend Server    │
│ Network & Logic Layer   │
└─────────────────────────┘
```

- **Frontend (C++)**: Handles UI rendering, user interactions, and game library management using Qt and QML
- **Backend (Go)**: Manages network requests, GitHub API interactions, and release checking

---

## IPC Protocol: JSON over stdin/stdout

### Overview

The frontend and backend communicate exclusively through **JSON messages over standard input/output streams**. This creates a clean separation of concerns and allows the two components to run as independent processes while maintaining synchronous request-response communication.

### How It Works

#### 1. **Request Flow**

The C++ frontend sends JSON-encoded requests to the backend through `stdin`:

```json
{"action": "check_release", "repo": "https://github.com/MasterArd/CGTerm"}
```

#### 2. **Backend Processing**

The Go backend runs an infinite loop (`FrontendLineReader()`) that:

1. **Reads** each line from `stdin` using a `bufio.Scanner`
2. **Validates** the JSON format using `json.Unmarshal()`
3. **Routes** the request to appropriate handlers based on the `action` field
4. **Processes** the request (e.g., fetching release info from GitHub)
5. **Encodes** the response as JSON
6. **Writes** the response to `stdout` using `fmt.Println()`

#### 3. **Response Flow**

The backend sends JSON-encoded responses back to the frontend through `stdout`:

```json
{"success": true, "version": "v1.2.3"}
```

or in case of error:

```json
{"success": false, "error": "repo field is required"}
```

### Request/Response Structure

#### Request Type

```go
type Request struct {
    Action string `json:"action"`    // Action to perform (e.g., "check_release")
    Repo   string `json:"repo"`      // Repository URL or identifier
}
```

#### Response Type

```go
type Response struct {
    Success bool   `json:"success"`           // True if request succeeded
    Version string `json:"version,omitempty"` // Release version (if applicable)
    Error   string `json:"error,omitempty"`   // Error message (if failed)
}
```

### Supported Actions

#### `check_release`

**Purpose**: Check the latest release version of a GitHub repository

**Request Example**:
```json
{"action": "check_release", "repo": "https://github.com/MasterArd/CGTerm"}
```

**Response (Success)**:
```json
{"success": true, "version": "v1.2.3"}
```

**Response (Failure - Missing Repo)**:
```json
{"success": false, "error": "repo field is required"}
```

**Response (Failure - Network Error)**:
```json
{"success": false, "error": "failed to fetch release data"}
```

### Detailed Message Flow

Here's a step-by-step trace of a typical IPC interaction:

```
FRONTEND (C++)                          BACKEND (Go)
    │                                      │
    ├─ Construct Request ────────────────► │
    │  {"action":"check_release",          │
    │   "repo":"github.com/user/project"}  │
    │                                      │
    │                          ◄───────────┤ Read from stdin
    │                          ◄───────────┤ Unmarshal JSON
    │                          ◄───────────┤ Route to handler
    │                          ◄───────────┤ Call Network.GetLatest()
    │                          ◄───────────┤ Marshal Response
    │                          ◄───────────┤ Write to stdout
    │  {"success":true,                    │
    │   "version":"v2.0.1"}◄───────────────┤
    │                                      │
    └─ Parse Response                      │
       Update UI                           │
```

### Error Handling

The IPC protocol handles errors gracefully:

1. **Invalid JSON**: If the frontend sends malformed JSON, the backend responds with:
   ```json
   {"success": false, "error": "Invalid JSON: <detailed error>"}
   ```

2. **Empty Lines**: The backend skips empty lines sent by the frontend (defensive programming)

3. **Unknown Actions**: Unrecognized actions receive:
   ```json
   {"success": false, "error": "Unknown action: <action_name>"}
   ```

4. **Missing Required Fields**: Each handler validates required fields and responds with specific error messages

5. **Network Errors**: Network-level errors (API failures, timeouts, etc.) are caught and reported:
   ```json
   {"success": false, "error": "<network error details>"}
   ```

6. **JSON Encoding Failures**: Fallback error response if the response itself fails to marshal:
   ```json
   {"success": false, "error": "Failed to encode response JSON"}
   ```

### Implementation Details

#### Backend Entry Point: `FrontendLineReader()`

Located in [src/Backend/Protocol/Protocol.go](src/Backend/Protocol/Protocol.go#L39):

```go
func FrontendLineReader() {
    scanner := bufio.NewScanner(os.Stdin)  // Create scanner for stdin
    for scanner.Scan() {                    // Infinite loop
        var request Request
        rawBytes := scanner.Bytes()
        
        // Skip empty lines
        if len(rawBytes) == 0 {
            continue
        }
        
        // Unmarshal JSON
        if err := json.Unmarshal(rawBytes, &request); err != nil {
            writeResponse(Response{
                Success: false,
                Error:   "Invalid JSON: " + err.Error(),
            })
            continue
        }
        
        // Route request
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
    
    // Error handling if scanner stops
    if err := scanner.Err(); err != nil {
        fmt.Fprintf(os.Stderr, "Backend stdin reading error: %v\n", err)
    }
}
```

#### Request Routing

The `switch` statement routes each request to its handler based on the `action` field. Currently implemented:

- `"check_release"` → `handleCheckRelease()`

New actions can be easily added by implementing a new handler and adding a case to the switch statement.

#### Response Writing: `writeResponse()`

All responses are written through a single function to ensure consistency:

```go
func writeResponse(response Response) {
    data, err := json.Marshal(response)
    if err != nil {
        // Fallback if JSON encoding fails
        fmt.Println(`{"success":false,"error":"Failed to encode response JSON"}`)
        return
    }
    fmt.Println(string(data))  // Each response is one line
}
```

**Important**: Each response is printed on a single line followed by a newline. This allows the frontend to use `readline()` or equivalent to read complete messages.

#### Handler Example: `handleCheckRelease()`

```go
func handleCheckRelease(request Request) {
    // Validate required fields
    if request.Repo == "" {
        writeResponse(Response{Success: false, Error: "repo field is required"})
        return
    }

    // Call backend network layer
    release, err := network.GetLatest(request.Repo)
    if err != nil {
        writeResponse(Response{
            Success: false,
            Error:   err.Error(),
        })
        return
    }

    // Success response
    writeResponse(Response{
        Success: true,
        Version: release.Version,
    })
}
```

### Why This Design?

✅ **Simple**: JSON is human-readable and language-agnostic  
✅ **Reliable**: One message per line makes parsing predictable  
✅ **Decoupled**: Frontend and backend run independently  
✅ **Debuggable**: Easy to log and inspect communication  
✅ **Portable**: Works across different operating systems  
✅ **Extensible**: New actions and response fields can be added without breaking existing code  

---

## Building

### Frontend (C++)
```bash
cd src/Frontend
mkdir build && cd build
cmake ..
make
```

### Backend (Go)
```bash
cd src/Backend
go build -o MyAppBackend
```

## Project Structure

- **src/Frontend/**: Qt/QML based user interface
  - `main.cpp`, `MainWindow.cpp/h`: Main application entry and window
  - `qml/`: QML files for UI (Library.qml, Main.qml, MyGamesPage.qml)
  - `resources.qrc`: Qt resource file
  
- **src/Backend/**: Go backend server
  - `main.go`: Entry point
  - `Protocol/Protocol.go`: IPC implementation and request routing
  - `Network/`: GitHub API integration and release fetching
  - `Common/`: Shared utilities

---

## Usage

Once built, run the frontend application. It will spawn the backend process and communicate via IPC to fetch and display game/application information from GitHub.