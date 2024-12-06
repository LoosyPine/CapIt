#include "../include/controller.hpp"

void Controller::session_definition()
{
    /*
     * During the Wayland session, both variables will end with 'd'
     */
    char *xdg_type = std::getenv("XDG_SESSION_TYPE");
    char *xdg_desktop = std::getenv("XDG_SESSION_DESKTOP");

    *xdg_type = xdg_type[strlen(xdg_type) - 1];
    *xdg_desktop = xdg_desktop[strlen(xdg_desktop) - 1];

    this->m_is_wayland_session = !(xdg_desktop - xdg_type);
    
    /*
     * Geting info about cpu
     */
    _cpu_sse_check();
}


void Controller::session_initialize(IMultimediaCentre *i_screenshot_maker)
{
    this->m_i_screenshot_maker = i_screenshot_maker;
    this->m_i_screenshot_maker->initialize();

    this->m_display_res.width = this->m_i_screenshot_maker->get_display_width();
    this->m_display_res.height = this->m_i_screenshot_maker->get_display_height();
}


void Controller::session_make_screenshot()
{
    if(this->m_i_screenshot_maker == nullptr)
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }

    this->m_i_screenshot_maker->make_screenshot();
}


void Controller::session_make_screenshot_right_now()
{
    if(this->m_i_screenshot_maker == nullptr)
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }

    this->m_i_screenshot_maker->make_screenshot_right_now();
}


void Controller::save_screenshot_png()
{
    SimplePNG png;
    png.initialize(this->m_display_res.width, this->m_display_res.height);                 
    png.set_image_data(this->m_i_screenshot_maker->get_screenshot_data()); 
    png.set_image_row_bytecount(
        this->m_i_screenshot_maker->get_screenshot_row_bytecount());
    png.save_png();
}


void Controller::session_start_video()
{
    this->m_i_screenshot_maker->start_video();
}


void Controller::session_forced_stop_video()
{
    this->m_i_screenshot_maker->stop_video();
}


void Controller::set_trigger_key(int key,  unsigned int mask)
{
    this->m_i_screenshot_maker->set_trigger_key(key, mask);
}


unsigned short Controller::get_display_width()
{
    //_mm_prefetch(&(*this->m_display_res.width), _MM_HINT_T0);
    return this->m_display_res.width;
}


unsigned short Controller::get_display_height()
{       
    //_mm_prefetch(&(*this->m_display_res.height), _MM_HINT_T0);
    return this->m_display_res.height;
}


bool Controller::is_wayland()
{       
    return this->m_is_wayland_session;
}


void Controller::_cpu_sse_check()
{
    u_int32_t res;
    __asm__(
        ".intel_syntax;"
        "mov %%eax, %1;"
        "cpuid;"
        "mov %0, %%ecx;"
        ".att_syntax;"
        :"=r"(res)
        :"r"(1)
        :"%eax", "%ecx"
    );
    this->m_have_sse_instructions = res & 1;
}