cmake_minimum_required(VERSION 3.10)

project(CaptureIt)

add_compile_options( -O1 -msse3 -mpopcnt)

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

add_executable(${APP} ${APP_PATH} ${X11SM_PATH} ${CONTR_PATH} ${SIMP_PNG_PATH})

target_link_libraries(${APP} PRIVATE png)

target_include_directories(${APP} PUBLIC "framework/include/")
