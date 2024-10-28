#pragma once
#include <iostream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>

#include "i_screenshot_maker.hpp"
#include "simple_png.hpp"


class Controller
{
public:
    void session_definition();
    void session_initialize(IScreenshotMaker *i_screenshot_maker_ptr);
    void session_make_screenshot();

    bool is_wayland();
    void save_screenshot_png();

    unsigned short* get_display_width();
    unsigned short* get_display_height();

private:
    bool                 m_is_wayland_session = 0;
    IScreenshotMaker    *m_i_screenshot_maker = nullptr;
    unsigned short      *m_display_width = nullptr;
    unsigned short      *m_display_height = nullptr;
};