#pragma once

#include <iostream>
#include <fstream>
#include <chrono>

#include <png.h>
//#include <xmmintrin.h>

#undef CLASS_NAME
#define CLASS_NAME "SimplePNG"
#define PRINT_LINE std::cerr << "Line->" << __LINE__ << '\n';
#define PRINT_FUNC std::cerr << "Function: " << __func__ << ' ';
#define PRINT_ERROR_MESSAGE(m) std::cerr << "ERROR::" << m << ' '; 
#define PRINT_DEBUG_ERROR PRINT_ERROR_MESSAGE(CLASS_NAME) PRINT_FUNC PRINT_LINE

class SimplePNG
{
public:
    ~SimplePNG();
    void initialize(unsigned short width, unsigned short height);
    void save_png();
    
    void set_image_data(char* data);
    void set_image_row_bytecount(int bytes_per_line);

private:
    void _initialize_write_sctruct();
    void _initialize_info_scturct();
    void _initialize_fstream();
    void _initialize_jmpbuf();

    void _set_display_width(unsigned short width);
    void _set_display_height(unsigned short width);

    void _check_image_data_ptr();

    png_structp     m_write_ptr = nullptr;
    png_infop       m_info_ptr = nullptr;
    FILE*           m_fp = nullptr;
    png_bytep*      m_row_pointers = nullptr;
    int             m_row_bytecount = 0;
    char*           m_data = nullptr;
    unsigned short  m_width = 0;
    unsigned short  m_height = 0;
};