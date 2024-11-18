#pragma once

#include <iostream>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "i_screenshot_maker.hpp"

#undef CLASS_NAME
#define CLASS_NAME "X11ScreenshotMaker"
#define PRINT_LINE std::cerr << "Line->" << __LINE__ << '\n';
#define PRINT_FUNC std::cerr << "Function: " << __func__ << ' ';
#define PRINT_ERROR_MESSAGE(m) std::cerr << "ERROR::" << m << ' '; 
#define PRINT_DEBUG_ERROR PRINT_ERROR_MESSAGE(CLASS_NAME) PRINT_FUNC PRINT_LINE

class X11ScreenshotMaker : public IScreenshotMaker
{
public:
    void            initialize();
    void            make_screenshot();

    char*           get_screenshot_data();
    int             get_screenshot_row_bytecount();
    unsigned short  get_display_width();
    unsigned short  get_display_height();

    void            set_key(int key);
    void            set_key_coombination(unsigned int mask, int key);

private:
    void _check_img_ptr();
    inline void _key_waiting_loop();



    Window          m_root_window;
    XEvent          m_event;
    XImage*          m_image = nullptr;
    Display*         m_display = nullptr;
    int              m_screen;
    unsigned short   m_display_width = 0;
    unsigned short   m_display_height = 0;
};