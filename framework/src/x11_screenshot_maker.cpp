#include "../include/x11_screenshot_maker.hpp"

void X11ScreenshotMaker::initialize()
{
    this->m_display = XOpenDisplay(getenv("DISPLAY"));
    if(this->m_display == NULL)
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }
    
    this->m_screen = XDefaultScreen(this->m_display);
    this->m_display_height = XDisplayHeight(this->m_display, this->m_screen);
    this->m_display_width = XDisplayWidth(this->m_display, this->m_screen);
}

void X11ScreenshotMaker::make_screenshot()
{
    int y = XKeysymToKeycode(this->m_display, XK_Y);
    this->m_root_window = XDefaultRootWindow(this->m_display);
    XGrabKey(this->m_display,
            y,
            ControlMask,
            this->m_root_window,
            false,
            GrabModeAsync,
            GrabModeAsync);

    XGrabKey(this->m_display,
            XKeysymToKeycode(this->m_display, XK_N),
            ControlMask,
            this->m_root_window,
            false,
            GrabModeAsync,
            GrabModeAsync);

    _key_waiting_loop();

    this->m_image = XGetImage(this->m_display,
                            DefaultRootWindow(this->m_display),
                            0,
                            0,
                            this->m_display_width,
                            this->m_display_height,
                            AllPlanes,
                            ZPixmap);
}

inline void X11ScreenshotMaker::_key_waiting_loop()
{
    XNextEvent(this->m_display, &this->m_event);
    std::cout << this->m_event.xkey.keycode << '\n';
    while(true)
    {
        if(this->m_event.type == KeyPress)
        {
            //std::cout << XK_Y << '\n';
            break;
        }
    }
}

char* X11ScreenshotMaker::get_screenshot_data()
{
    _check_img_ptr();
    return this->m_image->data;
}

int X11ScreenshotMaker::get_screenshot_row_bytecount()
{
    _check_img_ptr();
    return this->m_image->bytes_per_line;
}

unsigned short X11ScreenshotMaker::get_display_width()
{
    return this->m_display_width;
}

unsigned short X11ScreenshotMaker::get_display_height()
{
    return this->m_display_height;
}

void X11ScreenshotMaker::_check_img_ptr()
{
    if(this->m_image == nullptr)
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }
}