# AstralInstall

AstralInstall is a Qt/QML desktop application with a Go backend for checking GitHub releases and downloading release assets. The current frontend also displays a local JSON game catalog.

## Current Features

- Qt/QML interface with Library and My games pages
- Loads catalog entries from `games.json` beside the frontend executable
- Checks the latest GitHub release tag for a repository
- Downloads a matching asset from the latest GitHub release
- Runs the Go backend as a child process and communicates with it over newline-delimited JSON on stdin/stdout

The catalog is currently sample data copied from `src/Backend/games.json` at build time; it is not fetched from GitHub. The My games page, filtering controls, and GitHub button are not yet connected to install, filtering, or sign-in workflows.

## Architecture

```text
Qt/QML frontend
    | QProcess: JSON lines over stdin/stdout
    v
Go backend
    | GitHub REST API and release asset downloads
```

At startup, the frontend launches the adjacent `Backend` executable and loads the adjacent `games.json`. The backend reads one JSON request per line and processes requests serially. Responses are JSON lines; download progress messages are also printed to stdout and passed through as informational messages by the frontend.

## IPC

Requests are JSON objects with an `action` field. The current request fields are `repo`, `platform`, and `destination` as needed. Generic responses use this shape:

```json
{"success": true, "version": "v1.2.3"}
```

Errors use `{"success": false, "error": "..."}`. Each response corresponds to a request in FIFO order.

### `check_release`

Fetches the latest published GitHub release and returns its tag as `version`.

```json
{"action": "check_release", "repo": "https://github.com/owner/project"}
```

### `download_newest`

Finds a non-source release asset, optionally matching `platform` against the asset name, and downloads it into `destination`. An empty platform uses the first eligible asset; an empty destination uses the backend's working directory. The success response contains `success` only.

```json
{"action": "download_newest", "repo": "https://github.com/owner/project", "platform": "linux", "destination": "/path/to/downloads"}
```

The asset selector currently excludes asset names ending in `.zip`, `.tar.gz`, or `.tgz`; it does not unpack downloaded files.

### `login`

The backend router includes a device-login action, but the frontend does not call it and the GitHub button is a placeholder. The current handler writes authentication data to stdout and `log.log`; this route is not safe for normal use until credential handling is corrected.

Malformed JSON, missing required fields, unknown actions, and network/API failures return an error response. Empty input lines are ignored. Backend diagnostics are written to stderr.

## Build Requirements

- CMake 3.25 or newer
- Qt 6 with Core, Gui, Widgets, Quick, Quick Controls 2, and QML
- Go (the module declares Go 1.26.5)
- GCC or Clang for cgo
- Zig 0.14.1 for the native formatting library; CMake downloads the pinned toolchain for supported Linux and Windows architectures when needed

The build produces the frontend and backend side by side under `build/frontend/`. On Windows, CMake also deploys Qt runtime dependencies when `windeployqt` is available.

## Build and Run

From the repository root:

```bash
make          # Build frontend and backend (default target)
make run      # Build and launch the frontend
make backend  # Build only the backend
make backend-run  # Run the backend in debug mode
make clean    # Remove build outputs and generated native library
```

Useful Make variables:

- `BUILD_TYPE=Debug|Release` (default: `Release`)
- `QT6_DIR=/path/to/Qt6` when CMake cannot locate Qt automatically
- `BUILD_DIR=/path/to/build` to override the build root

The backend executable is built with cgo and links the Zig-generated `src/Backend/Common/libformat.a`. The frontend build performs both backend and native-library build steps.

## Catalog Data

`games.json` must be a JSON array. The frontend reads these fields from each object:

| Field | Type | Description |
| --- | --- | --- |
| `Game_name` | string | Display name |
| `description` | string | Description |
| `developer` | string | Developer name |
| `banner` | string | Banner image URL or resource path |
| `platform_windows` | boolean | Windows availability |
| `platform_linux` | boolean | Linux availability |
| `genres` | string array | Genre labels |
| `Game_version` | string | Displayed version |

The older `platforms` array is not used by the frontend.

## Project Layout

- `src/Frontend/`: Qt/C++ application, QML views, backend process bridge, and catalog model
- `src/Backend/`: Go IPC server, GitHub release operations, authentication, and shared code
- `tools/`: pinned Zig toolchain used by the native formatting library
- `build/`: generated executables and build files