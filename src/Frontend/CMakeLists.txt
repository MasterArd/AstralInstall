cmake_minimum_required(VERSION 3.25)  # 3.25 fuer 'cmake -E env --modify'
project(MyAppFrontend)

# Hard-coded Qt location for this Windows box. Elsewhere (or when passed on
# the command line via -DQt6_DIR=...) let CMake find Qt itself.
if(WIN32 AND NOT DEFINED Qt6_DIR)
    set(Qt6_DIR "C:/Qt/6.11.1/msvc2022_64/lib/cmake/Qt6")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Quick QuickControls2 Qml REQUIRED)

# Automatisch alle .cpp und .h Dateien sammeln
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(MyAppFrontend
    ${SOURCES}
    ${HEADERS}
    resources.qrc
)

target_link_libraries(MyAppFrontend
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Qml
)

# ---------------------------------------------------------------------------
# Backend: erst die Zig-Bibliothek, dann das Go-Binary.
#
# Beides landet neben der Frontend-Executable, damit BackendBridge es ueber
# applicationDirPath() findet. Das Binary heisst "Backend" / "Backend.exe",
# genau wie es "go build ." erzeugt.
# ---------------------------------------------------------------------------
set(BACKEND_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Backend")

# Common/Convert.go linkt die Bibliothek per '#cgo LDFLAGS: -L. -lformat'.
# Name und Ort sind damit fest vorgegeben: libformat.a neben den Go-Quellen,
# auf jeder Plattform gleich.
set(ZIG_SOURCE "${BACKEND_DIR}/Common/Convert.zig")
set(ZIG_LIBRARY "${BACKEND_DIR}/Common/libformat.a")

find_program(GO_EXECUTABLE go)

# Zig baut Common/libformat.a. Damit weder Windows noch Linux eine
# Handinstallation braucht, holen wir das offizielle Release nach tools/,
# falls zig nicht ohnehin im PATH steht. tools/ liegt neben build/ und
# ueberlebt damit ein 'make clean'.
set(ZIG_VERSION "0.16.0")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
    set(ZIG_ARCH "aarch64")
else()
    set(ZIG_ARCH "x86_64")
endif()

find_program(ZIG_EXECUTABLE zig)

if(NOT ZIG_EXECUTABLE)
    if(WIN32)
        set(ZIG_OS "windows")
        set(ZIG_ARCHIVE_SUFFIX "zip")
        set(ZIG_BINARY_NAME "zig.exe")
    else()
        set(ZIG_OS "linux")
        set(ZIG_ARCHIVE_SUFFIX "tar.xz")
        set(ZIG_BINARY_NAME "zig")
    endif()

    # sha256 der offiziellen Releases, siehe https://ziglang.org/download/index.json
    if(ZIG_ARCH STREQUAL "x86_64" AND ZIG_OS STREQUAL "windows")
        set(ZIG_SHA256 "68659eb5f1e4eb1437a722f1dd889c5a322c9954607f5edcf337bc3684a75a7e")
    elseif(ZIG_ARCH STREQUAL "aarch64" AND ZIG_OS STREQUAL "windows")
        set(ZIG_SHA256 "aee38316ee4111717900f45dd3130145c39289e105541d737eb8c5ed653c78ef")
    elseif(ZIG_ARCH STREQUAL "x86_64" AND ZIG_OS STREQUAL "linux")
        set(ZIG_SHA256 "70e49664a74374b48b51e6f3fdfbf437f6395d42509050588bd49abe52ba3d00")
    elseif(ZIG_ARCH STREQUAL "aarch64" AND ZIG_OS STREQUAL "linux")
        set(ZIG_SHA256 "ea4b09bfb22ec6f6c6ceac57ab63efb6b46e17ab08d21f69f3a48b38e1534f17")
    else()
        set(ZIG_SHA256 "")
    endif()

    set(ZIG_DIST "zig-${ZIG_ARCH}-${ZIG_OS}-${ZIG_VERSION}")
    set(ZIG_TOOLS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../tools")
    set(ZIG_DOWNLOADED "${ZIG_TOOLS_DIR}/${ZIG_DIST}/${ZIG_BINARY_NAME}")

    if(NOT EXISTS "${ZIG_DOWNLOADED}" AND ZIG_SHA256)
        set(ZIG_ARCHIVE "${ZIG_TOOLS_DIR}/${ZIG_DIST}.${ZIG_ARCHIVE_SUFFIX}")
        message(STATUS "Downloading ${ZIG_DIST}")

        file(DOWNLOAD
            "https://ziglang.org/download/${ZIG_VERSION}/${ZIG_DIST}.${ZIG_ARCHIVE_SUFFIX}"
            "${ZIG_ARCHIVE}"
            EXPECTED_HASH SHA256=${ZIG_SHA256}
            TLS_VERIFY ON
            SHOW_PROGRESS
            STATUS ZIG_DOWNLOAD_STATUS
        )

        list(GET ZIG_DOWNLOAD_STATUS 0 ZIG_DOWNLOAD_CODE)
        if(ZIG_DOWNLOAD_CODE EQUAL 0)
            # Das Archiv enthaelt bereits den Ordner ${ZIG_DIST}.
            file(ARCHIVE_EXTRACT INPUT "${ZIG_ARCHIVE}" DESTINATION "${ZIG_TOOLS_DIR}")
            file(REMOVE "${ZIG_ARCHIVE}")
        else()
            list(GET ZIG_DOWNLOAD_STATUS 1 ZIG_DOWNLOAD_ERROR)
            message(WARNING "Downloading Zig failed: ${ZIG_DOWNLOAD_ERROR}")
            file(REMOVE "${ZIG_ARCHIVE}")
        endif()
    elseif(NOT ZIG_SHA256)
        message(WARNING "No pinned Zig build for ${ZIG_ARCH}-${ZIG_OS} - "
                        "install zig manually or put it in PATH")
    endif()

    if(EXISTS "${ZIG_DOWNLOADED}")
        set(ZIG_EXECUTABLE "${ZIG_DOWNLOADED}" CACHE FILEPATH "zig compiler" FORCE)
    endif()
endif()

# Common/Seed.go und Common/Convert.go nutzen cgo, das Backend braucht also
# CGO_ENABLED=1 und einen gcc-kompatiblen Compiler (cgo kann kein MSVC).
if(WIN32)
    # Qt bringt eine MinGW-Toolchain mit - die nehmen, wenn kein gcc im PATH ist.
    file(GLOB CGO_COMPILER_HINTS "${Qt6_DIR}/../../../../../Tools/mingw*/bin")
else()
    # Auf Linux/macOS steht cc ohnehin im PATH, keine Sonderpfade noetig.
    set(CGO_COMPILER_HINTS "")
endif()

find_program(CGO_C_COMPILER NAMES gcc clang HINTS ${CGO_COMPILER_HINTS})
get_filename_component(CGO_COMPILER_DIR "${CGO_C_COMPILER}" DIRECTORY)

# cgo linkt auf Windows mit MinGW. Zigs nativer Default waere die MSVC-ABI,
# die ld.exe nicht lesen kann - darum explizit auf gnu stellen. Ueberall
# sonst ist das native Ziel schon das richtige.
if(WIN32)
    set(ZIG_TARGET_OPTION -target ${ZIG_ARCH}-windows-gnu)
else()
    set(ZIG_TARGET_OPTION "")
endif()

if(GO_EXECUTABLE AND ZIG_EXECUTABLE AND CGO_C_COMPILER)
    add_custom_command(TARGET MyAppFrontend POST_BUILD
        COMMAND "${ZIG_EXECUTABLE}" build-lib -static -O ReleaseFast
                ${ZIG_TARGET_OPTION}
                "-femit-bin=${ZIG_LIBRARY}" "${ZIG_SOURCE}"
        WORKING_DIRECTORY "${BACKEND_DIR}/Common"
        COMMENT "Building Zig format library"
    )

    # gcc startet cc1.exe und collect2.exe, die libwinpthread-1.dll aus dem
    # bin-Verzeichnis des Compilers brauchen. Fehlt das im PATH, brechen sie
    # ohne jede Fehlermeldung mit exit 1 ab. Darum PATH ergaenzen.
    add_custom_command(TARGET MyAppFrontend POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E env
                --modify "PATH=path_list_prepend:${CGO_COMPILER_DIR}"
                CGO_ENABLED=1 "CC=${CGO_C_COMPILER}"
                "${GO_EXECUTABLE}" build
                -o "$<TARGET_FILE_DIR:MyAppFrontend>/Backend${CMAKE_EXECUTABLE_SUFFIX}" .
        WORKING_DIRECTORY "${BACKEND_DIR}"
        COMMENT "Building Go backend"
    )
elseif(NOT GO_EXECUTABLE)
    message(WARNING "go not found in PATH - the backend will not be built")
elseif(NOT ZIG_EXECUTABLE)
    message(WARNING "zig not found in PATH - Common/libformat.a cannot be built, "
                    "so the backend will not link")
else()
    message(WARNING "no gcc/clang found for cgo - the backend will not be built")
endif()

find_program(WINDEPLOYQT_EXECUTABLE windeployqt
    HINTS "${Qt6_DIR}/../../../bin"
)

if(WIN32 AND WINDEPLOYQT_EXECUTABLE)
    add_custom_command(TARGET MyAppFrontend POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --qmldir "${CMAKE_CURRENT_SOURCE_DIR}"
                "$<TARGET_FILE:MyAppFrontend>"
        COMMENT "Deploying Qt runtime"
    )
endif()
