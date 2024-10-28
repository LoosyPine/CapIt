#include <iostream>
#include <fstream>
#include <cstdio>

#include <pthread.h>
#include <png.h>
#include <errno.h>

#include "framework/include/x11_screenshot_maker.hpp"
#include "framework/include/controller.hpp"


int main()
{

    Controller core;
    X11ScreenshotMaker x11;
    core.session_definition();
    core.session_initialize(&x11);
    core.session_make_screenshot();
    core.save_screenshot_png();

    return 0;
}