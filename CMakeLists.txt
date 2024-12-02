cmake_minimum_required(VERSION 3.10)

project(CaptureIt)

#add_compile_options(-O1 -msse3 -mpopcnt)

set(APP "App")
set(X11SM "X11ScreenshotMaker")
set(ISM "IScreenshotMaker")
set(SIMP_PNG "SimplePNG")
set(CONTR "Controller")

set(APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
set(X11SM_PATH "${CMAKE_CURRENT_SOURCE_DIR}/framework/src/x11_screenshot_maker.cpp")
set(SIMP_PNG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/framework/src/simple_png.cpp")
set(CONTR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/framework/src/controller.cpp")

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
find_library(AVDEVICE_LIBRARY avdevice)

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)
find_library(SWSCALE_LIBRARY swscale)

add_executable(${APP} ${APP_PATH} ${X11SM_PATH} ${CONTR_PATH} ${SIMP_PNG_PATH}
${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${AVDEVICE_INCLUDE_DIR} ${SWSCALE_INCLUDE_DIR})

target_link_libraries(${APP} PRIVATE png ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${AVDEVICE_LIBRARY} ${SWSCALE_LIBRARY})

target_include_directories(${APP} PUBLIC "framework/include/")