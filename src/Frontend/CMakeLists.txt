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

# Build the Go backend and drop the binary next to the frontend
# executable, so BackendBridge can find it via applicationDirPath().
# Named "Backend" / "Backend.exe" to match what "go build ." produces.
find_program(GO_EXECUTABLE go)

# Common/Seed.go uses cgo, so the backend needs CGO_ENABLED=1 and a
# gcc-compatible compiler (cgo does not work with MSVC). Qt ships a MinGW
# toolchain, so fall back to that when gcc is not in PATH.
file(GLOB CGO_COMPILER_HINTS "${Qt6_DIR}/../../../../../Tools/mingw*/bin")
find_program(CGO_C_COMPILER NAMES gcc clang HINTS ${CGO_COMPILER_HINTS})

# gcc startet cc1.exe und collect2.exe, die libwinpthread-1.dll aus dem
# bin-Verzeichnis des Compilers brauchen. Fehlt das im PATH, brechen sie
# ohne jede Fehlermeldung mit exit 1 ab. Darum PATH ergaenzen.
get_filename_component(CGO_COMPILER_DIR "${CGO_C_COMPILER}" DIRECTORY)

if(GO_EXECUTABLE AND CGO_C_COMPILER)
    add_custom_command(TARGET MyAppFrontend POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E env
                --modify "PATH=path_list_prepend:${CGO_COMPILER_DIR}"
                CGO_ENABLED=1 "CC=${CGO_C_COMPILER}"
                "${GO_EXECUTABLE}" build
                -o "$<TARGET_FILE_DIR:MyAppFrontend>/Backend${CMAKE_EXECUTABLE_SUFFIX}" .
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../Backend"
        COMMENT "Building Go backend"
    )
elseif(NOT GO_EXECUTABLE)
    message(WARNING "go not found in PATH - the backend will not be built")
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