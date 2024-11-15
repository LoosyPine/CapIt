#pragma once

#include <iostream>

class IScreenshotMaker
{
public:
    virtual void initialize() = 0;
    virtual void make_screenshot() = 0;
    virtual char* get_screenshot_data() = 0;
    virtual int get_screenshot_row_bytecount() = 0;
    virtual unsigned short get_display_width() = 0;
    virtual unsigned short get_display_height() = 0;
};