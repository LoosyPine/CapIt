#pragma once

#include <iostream>
#include <cstring>

#include "i_multimedia_centre.hpp"
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
    void session_initialize(IMultimediaCentre *i_screenshot_maker_ptr);
    void session_make_screenshot();
    void session_make_screenshot_right_now();
    void save_screenshot_png();
    void session_start_video();
    void session_forced_stop_video();
    bool is_wayland();

    void set_trigger_key(int key,  unsigned int mask = 0);
    void set_video_filename(char* name);
    void set_video_fps(uint16_t fps);

    unsigned short get_display_width();
    unsigned short get_display_height();


private:
    void _cpu_sse_check();
    
    struct Resolution
    {
        unsigned short  width = 0;
        unsigned short  height = 0;
    }; 

    IMultimediaCentre*    m_i_multimedia_centre = nullptr;
    Resolution           m_display_res;
    bool                 m_is_wayland_session = false;
    bool                 m_have_sse_instructions = false;
};