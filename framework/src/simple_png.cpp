#include "../include/simple_png.hpp"

void SimplePNG::initialize(unsigned short width, unsigned short height)
{
    _set_display_width(width);
    _set_display_height(height);

    _initialize_fstream();
    _initialize_write_sctruct();
    _initialize_info_scturct();
    _initialize_jmpbuf();

    png_init_io(this->m_write_ptr, this->m_fp);
    png_set_IHDR(this->m_write_ptr, this->m_info_ptr,
                this->m_width, this->m_height,
                8,
                PNG_COLOR_TYPE_RGB_ALPHA,
                PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT,
                PNG_FILTER_TYPE_DEFAULT
                );
}

void SimplePNG::_initialize_write_sctruct()
{
    this->m_write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                nullptr,
                                                nullptr,
                                                nullptr);
    if(!this->m_write_ptr)
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }
}

void SimplePNG::_initialize_fstream()
{
    this->m_fp = fopen("outputPNG.png", "wb");
    if(!this->m_fp)
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }
}

void SimplePNG::_initialize_info_scturct()
{
    this->m_info_ptr = png_create_info_struct(this->m_write_ptr);
    if(!this->m_info_ptr)
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }
}

void SimplePNG::_initialize_jmpbuf()
{
    if(setjmp(png_jmpbuf(this->m_write_ptr)))
    {
        PRINT_DEBUG_ERROR;
        png_destroy_write_struct(&this->m_write_ptr, &this->m_info_ptr);
        fclose(m_fp);
        exit(EXIT_FAILURE);
    }
}
void SimplePNG::save_png()
{
    auto start = std::chrono::steady_clock::now();
    _check_image_data_ptr();

    this->m_row_pointers = new png_bytep[this->m_height];

    png_bytep  row_data;
    for(unsigned short i = 0; i < this->m_height - (unsigned short)1; ++i)
    {
        row_data = (png_bytep)((this->m_data) + i * (this->m_row_bytecount));
        this->m_row_pointers[i] = row_data;

        ++i;
        row_data = (png_bytep)((this->m_data) + i * (this->m_row_bytecount));
        this->m_row_pointers[i] = row_data;
    }

    png_set_rows(this->m_write_ptr, this->m_info_ptr, this->m_row_pointers);
    png_write_png(this->m_write_ptr, this->m_info_ptr, PNG_TRANSFORM_BGR, nullptr);
    png_write_end(this->m_write_ptr, this->m_info_ptr);
    auto end = std::chrono::steady_clock::now();
    std::cout << "SimplePNG::save_png()->Time:" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
}


void SimplePNG::set_image_data(char* data)
{
    if(data == nullptr)
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }

    this->m_data = data;
}

void SimplePNG::set_image_row_bytecount(int bytes_per_line)
{
    this->m_row_bytecount = bytes_per_line;
}

void SimplePNG::_check_image_data_ptr()
{
    if(this->m_data == nullptr)
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }
}

void SimplePNG::_set_display_width(unsigned short width)
{
    this->m_width = width;
}

void SimplePNG::_set_display_height(unsigned short height)
{
    this->m_height = height;
}

SimplePNG::~SimplePNG()
{
    png_destroy_write_struct(&this->m_write_ptr, &this->m_info_ptr);
    fclose(m_fp);
    this->m_fp = nullptr;
}