

#include <iostream>
#include <fstream>

#include <pthread.h>
#include <chrono>

#include "framework/include/x11_multimedia_centre.hpp"
#include "framework/include/controller.hpp"


int main()
{
    Controller core;
    X11_MultimediaCentre x11;

    core.session_definition();
    core.session_initialize(&x11);

    core.set_trigger_key(XK_R, ControlMask); // use Xlib key_code and key_mask

    core.session_make_screenshot_right_now();
    core.save_screenshot_png();

    core.session_start_video();

    return 0;
}