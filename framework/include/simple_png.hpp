#pragma once

#include <iostream>
#include <fstream>
#include <cstdio>

#include <png.h>


class SimpleLibPNG
{
public:
    ~SimpleLibPNG();
    void initialize(unsigned short *width, unsigned short *height);
    void save_png();
    
    void set_image_data(char *data);
    void set_image_bytes_per_line(int *bytes_per_line);

private:
    void _set_display_width(unsigned short* width);
    void _set_display_height(unsigned short* width);
    void _check_image_data_ptr();
    void _check_image_bytes_per_line_ptr();

    FILE            *m_fp = NULL;
    png_structp      m_write_ptr = NULL;
    png_infop        m_info_ptr = NULL;
    png_bytep       *m_row_pointers = NULL;
    char            *m_data = nullptr;
    int             *m_bytes_per_line = nullptr;
    unsigned short  *m_width = nullptr;
    unsigned short  *m_height = nullptr;
};