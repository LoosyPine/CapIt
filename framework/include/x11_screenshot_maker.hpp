#pragma once

#include <iostream>
#include <fstream>
#include <cstdio>

#include <X11/Xlib.h>

#include "i_screenshot_maker.hpp"

#define IS_DEBUG true

#if IS_DEBUG
#define PRINT_ERROR(func_name) std::cerr << "ERROR::X11ScreenshotMaker::" << func_name << "::Line->" << __LINE__ << std::endl;
#endif

class X11ScreenshotMaker : public IScreenshotMaker
{
public:
    void            initialize();
    void            make_screenshot();

    char*           get_screenshot_data();
    int*            get_screenshot_row_bytecount();
    unsigned short* get_display_width();
    unsigned short* get_display_height();

private:
    #if IS_DEBUG
    void _check_img_ptr();
    #endif

    Display         *m_display = nullptr;
    int              m_screen;
    XImage          *m_image = nullptr;
    unsigned short   m_display_height = 0;
    unsigned short   m_display_width = 0;
};