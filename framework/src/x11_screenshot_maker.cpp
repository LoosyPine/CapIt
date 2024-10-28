#include "../include/x11_screenshot_maker.hpp"

void X11ScreenshotMaker::initialize()
{
    this->m_display = XOpenDisplay(getenv("DISPLAY"));

    #if IS_DEBUG
    if(this->m_display == NULL)
    {
        PRINT_ERROR(__func__);
        exit(EXIT_FAILURE);
    }
    #endif
    

    this->m_screen = XDefaultScreen(this->m_display);
    this->m_display_height = XDisplayHeight(this->m_display, this->m_screen);
    this->m_display_width = XDisplayWidth(this->m_display, this->m_screen);
}

void X11ScreenshotMaker::make_screenshot()
{
    this->m_image = XGetImage(this->m_display,
                            DefaultRootWindow(this->m_display),
                            0,
                            0,
                            this->m_display_width,
                            this->m_display_height,
                            AllPlanes,
                            ZPixmap);
}

char* X11ScreenshotMaker::get_screenshot_data()
{
    #if IS_DEBUG
    _check_img_ptr();
    #endif
    return this->m_image->data;
}

int* X11ScreenshotMaker::get_screenshot_row_bytecount()
{
    #if IS_DEBUG
    _check_img_ptr();
    #endif

    return &this->m_image->bytes_per_line;
}

unsigned short* X11ScreenshotMaker::get_display_width()
{
    return &this->m_display_width;
}

unsigned short* X11ScreenshotMaker::get_display_height()
{
    return &this->m_display_height;
}

#if IS_DEBUG
void X11ScreenshotMaker::_check_img_ptr()
{
    if(this->m_image == nullptr)
    {
        PRINT_ERROR(__func__);
        exit(EXIT_FAILURE);
    }
}
#endif

