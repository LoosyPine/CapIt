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
}

void Controller::session_initialize(IScreenshotMaker *i_screenshot_maker)
{
    this->m_i_screenshot_maker = i_screenshot_maker;
    this->m_i_screenshot_maker->initialize();

    this->m_display_width = this->m_i_screenshot_maker->get_display_width();
    this->m_display_height = this->m_i_screenshot_maker->get_display_height();
}

void Controller::session_make_screenshot()
{
    if(this->m_i_screenshot_maker == nullptr)
    {
        std::cerr << "ERROR::CAPIT::CONTROLLER::IGS_PTR_NO_INIT\n";
        exit(EXIT_FAILURE);
    }

    this->m_i_screenshot_maker->make_screenshot();
}

void Controller::save_screenshot_png()
{
    SimpleLibPNG png;
    png.initialize(this->m_display_width, this->m_display_height);                   
    png.set_image_data(this->m_i_screenshot_maker->get_screenshot_data());   
    png.set_image_bytes_per_line(
        this->m_i_screenshot_maker->get_screenshot_row_bytecount());
    png.save_png();
}

unsigned short* Controller::get_display_width()
{
    return this->m_display_width;
}

unsigned short* Controller::get_display_height()
{       
    return this->m_display_height;
}

bool Controller::is_wayland()
{       
    return this->m_is_wayland_session;
}