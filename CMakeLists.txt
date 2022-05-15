cmake_minimum_required(VERSION 3.5)

project(MediaPlayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FFMPEG_HEADERS_DIR "./ffmpeg/include")
set(FFMPEG_LIBS_DIR "./ffmpeg/lib")

include_directories(${FFMPEG_HEADERS_DIR})

link_directories(${FFMPEG_LIBS_DIR})

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt6 COMPONENTS Multimedia MultimediaWidgets REQUIRED)

set(FFMPEG_LIBS_DIR "./ffmpeg/lib")

set(SDL_HEADERS_DIR "./SDL/include")
set(SDL_LIBS_DIR "./SDL/lib")

include_directories(${FFMPEG_HEADERS_DIR})
include_directories(${SDL_HEADERS_DIR})


link_directories(${SDL_LIBS_DIR})
link_directories(${FFMPEG_LIBS_DIR})

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

file(GLOB_RECURSE VIDEODECODER_INCLUDE_FILE "${PROJECT_SOURCE_DIR}/VideoDecoder/*.h")
file(GLOB_RECURSE VIDEODECODER_SOURCE_FILE "${PROJECT_SOURCE_DIR}/VideoDecoder/*.cpp")
file(GLOB_RECURSE UI_INCLUDE_FILE "${PROJECT_SOURCE_DIR}/UI/*.h")
file(GLOB_RECURSE UI_SOURCE_FILE "${PROJECT_SOURCE_DIR}/UI/*.cpp")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MediaPlayer
        MANUAL_FINALIZATION
        ${VIDEODECODER_INCLUDE_FILE}
        ${VIDEODECODER_SOURCE_FILE}
        ${UI_INCLUDE_FILE}
        ${UI_SOURCE_FILE}
        ${PROJECT_SOURCE_DIR}/UI/mainwindow.ui
        main.cpp
        src.qrc
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET MediaPlayer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(MediaPlayer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(MediaPlayer
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(MediaPlayer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets avcodec avdevice avfilter avformat avutil swresample swscale SDL2 SDL2main)
target_link_libraries(MediaPlayer PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia)
target_link_libraries(MediaPlayer PRIVATE Qt${QT_VERSION_MAJOR}::MultimediaWidgets)

set_target_properties(MediaPlayer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MediaPlayer)
endif()
