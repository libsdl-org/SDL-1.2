cmake_minimum_required (VERSION 3.14)
project (SDL VERSION 1.2 LANGUAGES C)

file (GLOB SDL_COMMON_SOURCES
    "src/*.c"
    "src/audio/*.c"
    "src/cdrom/*.c"
    "src/cpuinfo/*.c"
    "src/events/*.c"
    "src/file/*.c"
    "src/joystick/*.c"
    "src/stdlib/*.c"
    "src/thread/*.c"
    "src/timer/*.c"
    "src/video/*.c"
)

if (WIN32)
    set (SDL_USE_WINDOWS TRUE)
elseif (UNIX AND NOT APPLE)
    set (SDL_USE_LINUX TRUE)
elseif (APPLE)
    set (SDL_USE_OSX)
else ()
    message (FATAL_ERROR "UNSUPPORTED OS")
endif ()

if (SDL_USE_WINDOWS)
    file (GLOB SDL_EXTRA_SOURCES
        "src/audio/windx5/*.c"
        "src/cdrom/win32/*.c"
        "src/joystick/win32/*.c"
        "src/loadso/win32/*.c"
        "src/main/win32/*.c"
        "src/thread/win32/*.c"
        "src/timer/win32/*.c"
        "src/video/windx5/*.c"
    )
    configure_file ("include/SDL_config_win32.h" "include/SDL_config.h" COPYONLY)
elseif (SDL_USE_LINUX)
    file (GLOB SDL_EXTRA_SOURCES
        "src/audio/pulse/*.c"
        "src/cdrom/linux/*.c"
        "src/joystick/linux/*.c"
        "src/loadso/dlopen/*.c"
        "src/main/dummy/*.c"
        "src/thread/pthread/*.c"
        "src/timer/unix/*.c"
        "src/video/x11/*.c"
    )
    include (cmake/SDL_GenerateConfig.cmake)
    set (SDL_REQUIRED_LIBS Threads::Threads X11::X11 m pulse)
elseif (SDL_USE_OSX)
    file (GLOB SDL_EXTRA_SOURCES
        "src/audio/macosx/*.c"
        "src/cdrom/macosx/*.c"
        "src/joystick/darwin/*.c"
        "src/loadso/macosx/*.c"
        "src/main/macosx/*.c"
        "src/thread/pthread/*.c"
        "src/timer/unix/*.c"
        "src/video/quartz/*.c"
        "src/video/quartz/*.m"
    )
    configure_file ("include/SDL_config_macosx.h" "include/SDL_config.h" COPYONLY)
    find_package(Threads REQUIRED)
    set (SDL_REQUIRED_LIBS
        Threads::Threads
        "-framework CoreAudio"
        "-framework AudioToolbox"
        "-framework CoreFoundation"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreServices"
        "-framework AudioUnit"
    )
endif()

add_library (SDL ${SDL_COMMON_SOURCES} ${SDL_EXTRA_SOURCES})
target_link_libraries (SDL ${SDL_REQUIRED_LIBS})
target_include_directories (SDL PUBLIC include)
target_include_directories (SDL PRIVATE ${CMAKE_BINARY_DIR}/include)