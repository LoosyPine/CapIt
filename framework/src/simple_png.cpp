#include "../include/simple_png.hpp"

void SimpleLibPNG::initialize(unsigned short *width, unsigned short *height)
{
    _set_display_width(width);
    _set_display_height(height);

    this->m_fp = fopen("outputPNG.png", "wb");
    if(!this->m_fp)
    {
        std::cerr << "\nERROR::CAPIT::ABSTRACT_LIB_PNG::FSTREAM::NOT_OPEN\n";
        exit(EXIT_FAILURE);
    }

    this->m_write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                              nullptr,
                                              nullptr,
                                              nullptr);
    if(!this->m_write_ptr)
    {
        std::cerr << "\nERROR::CAPIT::ABSTRACT_LIB_PNG::PNG_STRUCTP_NOT_CREATE\n";
        exit(EXIT_FAILURE);
    }

    this->m_info_ptr = png_create_info_struct(this->m_write_ptr);
    if(!this->m_info_ptr)
    {
        std::cerr << "\nERROR::CAPIT::ABSTRACT_LIB_PNG::PNG_INFOP_NOT_CREATE\n";
        exit(EXIT_FAILURE);
    }


    if(setjmp(png_jmpbuf(this->m_write_ptr)))
    {
        std::cerr << "\nERROR::CAPTIT::ABSTRACT_LIB_PNG::SETJMP_ERROR\n";
        png_destroy_write_struct(&this->m_write_ptr, &this->m_info_ptr);
        fclose(m_fp);
        exit(EXIT_FAILURE);
    }


    png_init_io(this->m_write_ptr, this->m_fp);
    png_set_IHDR(this->m_write_ptr, this->m_info_ptr,
                *this->m_width, *this->m_height,
                8,
                PNG_COLOR_TYPE_RGB_ALPHA,
                PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT,
                PNG_FILTER_TYPE_DEFAULT
                );
}

void SimpleLibPNG::save_png()
{
    _check_image_data_ptr();
    _check_image_bytes_per_line_ptr();

    this->m_row_pointers = new png_bytep[*this->m_height];

    png_bytep row_data;
    for(int i = 0; i < *this->m_height; ++i) 
    {
        row_data = (png_bytep)((this->m_data) + i * (*this->m_bytes_per_line));
        this->m_row_pointers[i] = row_data;
    }

    png_set_rows(this->m_write_ptr, this->m_info_ptr, this->m_row_pointers);
    png_write_png(this->m_write_ptr, this->m_info_ptr, PNG_TRANSFORM_BGR, nullptr);
    png_write_end(this->m_write_ptr, this->m_info_ptr);
}

void SimpleLibPNG::set_image_data(char *data)
{
    if(data == nullptr)
    {
        std::cerr << "\nERROR::CAPTIT::ABSTRACT_LIB_PNG::SET_IMG_DATA::DATA_NULLPTR\n";
        exit(EXIT_FAILURE);
    }

    this->m_data = data;
}

void SimpleLibPNG::set_image_bytes_per_line(int *bytes_per_line)
{
    if(bytes_per_line == nullptr)
    {
        std::cerr << "\nERROR::CAPTIT::ABSTRACT_LIB_PNG::SET_IMG_BYTES_PER_LINE::BYTES_PER_LINE_NULLPTR\n";
        exit(EXIT_FAILURE);
    }

    this->m_bytes_per_line = bytes_per_line;
}

void SimpleLibPNG::_check_image_data_ptr()
{
    if(this->m_data == nullptr)
    {
        std::cerr << "\nERROR::CAPTIT::ABSTRACT_LIB_PNG::CHECK_IMG_DATA::DATA_NULLPTR\n";
        exit(EXIT_FAILURE);
    }
}

void SimpleLibPNG::_check_image_bytes_per_line_ptr()
{
    if(this->m_bytes_per_line == nullptr)
    {
        std::cerr << "\nERROR::CAPTIT::ABSTRACT_LIB_PNG::CHECK_IMG_BYTES_PER_LINE::BYTES_PER_LINE_NULLPTR\n";
        exit(EXIT_FAILURE);
    }
}

void SimpleLibPNG::_set_display_width(unsigned short *width)
{
    if(width == nullptr)
    {
        std::cerr << "\nERROR::CAPTIT::ABSTRACT_LIB_PNG::DIS_WIDTH_NULLPTR\n";
        exit(EXIT_FAILURE);
    }

    this->m_width = width;
}

void SimpleLibPNG::_set_display_height(unsigned short *height)
{
    if(height == nullptr)
    {
        std::cerr << "\nERROR::CAPTIT::ABSTRACT_LIB_PNG::DIS_HEIGHT_NULLPTR\n";
        exit(EXIT_FAILURE);
    }

    this->m_height = height;
}

SimpleLibPNG::~SimpleLibPNG()
{
    png_destroy_write_struct(&this->m_write_ptr, &this->m_info_ptr);
    fclose(m_fp);
    this->m_fp = nullptr;
}