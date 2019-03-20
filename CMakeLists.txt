cmake_minimum_required(VERSION 3.10)
project(gst_camera)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x -g -fpermissive")

if (${APPLE})
    # PKG_CONFIG_PATH=/usr/local/opt/libffi/lib/pkgconfig
    set(ENV{PKG_CONFIG_PATH} "/usr/local/opt/libffi/lib/pkgconfig")
endif ()

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)

include_directories(
        ${GLIB_INCLUDE_DIRS}
)

link_directories(
        ${GLIB_LIBRARY_DIRS}
)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER-BASE REQUIRED gstreamer-base-1.0)
pkg_check_modules(GSTREAMER-APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GSTREAMER-VIDEO REQUIRED gstreamer-video-1.0)

include_directories(
        ${GSTREAMER_INCLUDE_DIRS}
        ${GSTREAMER-BASE_INCLUDE_DIRS}
        ${GSTREAMER-APP_INCLUDE_DIRS}
        ${GSTREAMER-VIDEO_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

link_directories(
        ${GSTREAMER_LIBRARY_DIRS}
        ${GSTREAMER-BASE_LIBRARY_DIRS}
        ${GSTREAMER-APP_LIBRARY_DIRS}
        ${GSTREAMER-VIDEO_LIBRARY_DIRS}
)

set(
        gst_camera_SOURCES
        main.cpp
)

set(
        gst_camera_LIBRARIES
        ${GSTREAMER_LIBRARIES}
        ${GSTREAMER-BASE_LIBRARIES}
        ${GSTREAMER-APP_LIBRARIES}
        ${GSTREAMER-VIDEO_LIBRARIES}

        pthread
        ${GLIB_LIBRARIES}
        ${GLIB_GIO_LIBRARIES}
        ${GLIB_GOBJECT_LIBRARIES}
)

add_executable(gst_camera ${gst_camera_SOURCES})
target_link_libraries(gst_camera ${gst_camera_LIBRARIES})