cmake_minimum_required(VERSION 3.20)
project(MyAppFrontend)

# Qt6 Pfad ZUERST setzen!
set(Qt6_DIR "C:/Qt/6.11.1/msvc2022_64/lib/cmake/Qt6")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Gui Quick REQUIRED)

add_executable(MyAppFrontend
    main.cpp
    MainWindow.h
    MainWindow.cpp
    resources.qrc
)

target_link_libraries(MyAppFrontend
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
)

# Qt-DLLs und QML-Plugins neben die Exe kopieren, damit sie startbar ist
find_program(WINDEPLOYQT_EXECUTABLE windeployqt
    HINTS "${Qt6_DIR}/../../../bin"
)

if(WIN32 AND WINDEPLOYQT_EXECUTABLE)
    add_custom_command(TARGET MyAppFrontend POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/qml"
                "$<TARGET_FILE:MyAppFrontend>"
        COMMENT "Deploying Qt runtime next to MyAppFrontend.exe"
    )
endif()