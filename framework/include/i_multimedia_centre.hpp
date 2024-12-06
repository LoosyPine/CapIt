#pragma once

#include <iostream>
#include <cstdint>

class IMultimediaCentre
{
public:
    virtual void initialize() = 0;
    virtual void make_screenshot() = 0;
    virtual void make_screenshot_right_now() = 0;
    virtual void start_video() = 0;
    virtual void stop_video() = 0;

    virtual void set_trigger_key(int key,  unsigned int mask = 0) = 0;
    virtual void set_video_filename(char* name) = 0;
    virtual void set_video_fps(uint16_t fps) = 0;

    virtual char*           get_screenshot_data() = 0;
    virtual int             get_screenshot_row_bytecount() = 0;
    virtual unsigned short  get_display_width() = 0;
    virtual unsigned short  get_display_height() = 0;
};