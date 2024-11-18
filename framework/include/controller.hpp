#pragma once

#include <iostream>
#include <cstring>

#include "i_screenshot_maker.hpp"
#include "simple_png.hpp"

#undef CLASS_NAME
#define CLASS_NAME "Controller"
#define PRINT_LINE std::cerr << "Line->" << __LINE__ << '\n';
#define PRINT_FUNC std::cerr << "Function: " << __func__ << ' ';
#define PRINT_ERROR_MESSAGE(m) std::cerr << "ERROR::" << m << ' '; 
#define PRINT_DEBUG_ERROR PRINT_ERROR_MESSAGE(CLASS_NAME) PRINT_FUNC PRINT_LINE

class Controller
{
public:
    void session_definition();
    void session_initialize(IScreenshotMaker *i_screenshot_maker_ptr);
    void session_make_screenshot();
    bool is_wayland();
    void save_screenshot_png();
    void set_key_and_function(char key, void (*func)());
    void input_key_loop_start();

    unsigned short get_display_width();
    unsigned short get_display_height();

private:
    void _cpu_sse_check();
    
    struct Resolution
    {
        unsigned short  width = 0;
        unsigned short  height = 0;
    }; 

    IScreenshotMaker*    m_i_screenshot_maker = nullptr;
    Resolution           m_display_res;
    bool                 m_is_wayland_session = false;
    bool                 m_have_sse_instructions = false;
};