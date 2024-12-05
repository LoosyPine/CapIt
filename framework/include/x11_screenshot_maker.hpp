#pragma once

#include <iostream>
#include <xcb/xcb.h>
#include <xcb/shm.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <xcb/xproto.h>
//#include <xcb/xinerama.h>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include <unistd.h>
#include <memory>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/channel_layout.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavdevice/avdevice.h>
}

#include "i_screenshot_maker.hpp"

#undef CLASS_NAME
#define CLASS_NAME "X11ScreenshotMaker"
#define PRINT_LINE std::cerr << "Line->" << __LINE__ << '\n';
#define PRINT_FUNC std::cerr << "Function: " << __func__ << ' ';
#define PRINT_ERROR_MESSAGE(m) std::cerr << "ERROR::" << m << ' '; 
#define PRINT_DEBUG_ERROR PRINT_ERROR_MESSAGE(CLASS_NAME) PRINT_FUNC PRINT_LINE

class X11ScreenshotMaker : public IScreenshotMaker
{
public:
    void            initialize();
    void            make_screenshot();
    void            start_video();

    char*           get_screenshot_data();
    int             get_screenshot_row_bytecount();
    unsigned short  get_display_width();
    unsigned short  get_display_height();

    /*VIDEO*/
    void va_initialize(); 

    void            set_key(int key);
    void            set_key_coombination(unsigned int mask, int key);

private:
    void _check_img_ptr();
    inline void _key_waiting_loop();

    void _video_initialize();
    void _input_key_loop();
    void _start_all_threads();
    void _finish_video();

    void _metronome_loop();

    void _write_image_loop();
    void _write_image(); //main func
    void _write_fake_func();
    void _write_core_func();

    void _create_image_loop();
    void _create_image(); //main func
    void _create_fake_func();
    void _create_core_func();


    void (X11ScreenshotMaker::*m_write_throttle[2])() = {
        &X11ScreenshotMaker::_write_fake_func,
        &X11ScreenshotMaker::_write_core_func};

    void (X11ScreenshotMaker::*m_create_throttle[2])()  = {
        &X11ScreenshotMaker::_create_fake_func,
        &X11ScreenshotMaker::_create_core_func};

    uint8_t* m_src_img_data[8];
    int m_scr_img_stride[8];

    int              m_i = 0;

    std::atomic<bool>     m_metronome_state{true};
    std::atomic<bool>     m_program_state{true};
    double                m_video_pts = 0;
    uint8_t               m_create_id = 1; //head
    uint8_t               m_write_id = 0; //tail
    bool                  m_create_delta_id = 0;
    bool                  m_write_delta_id = 0;
    std::atomic<bool>     m_write_status{false};
    std::atomic<bool>     m_create_status{false};
    char*                 m_output_filename = "tmp.mp4";
    const AVOutputFormat* m_output_format;
    AVPacket*             m_packet;
    AVStream*             m_stream;
    SwsContext*           m_sws_ctx = nullptr;
    AVFormatContext*      m_format_ctx = nullptr;
    AVCodecContext*       m_codec_ctx = nullptr;
    std::vector<AVFrame*> m_ring_buffer;
    uint8_t               m_ring_buffer_size;
    Window                m_root_window;
    XEvent                m_event;
    XImage*               m_image = nullptr;
    Display*              m_display = nullptr;
    int                   m_screen;
    unsigned short        m_display_width = 0;
    unsigned short        m_display_height = 0;


    //xcb-shm
    xcb_connection_t* m_connection = nullptr;
    const xcb_setup_t* m_setup = nullptr;
    xcb_screen_t* m_xcb_screen = nullptr;
    xcb_shm_seg_t m_seg = 0;
    int m_shmid = 0;
    xcb_shm_get_image_cookie_t m_cookie;
    uint8_t* m_buffer = nullptr;

    void _xcb_shm_inittialize();
    void _xcb_shm_create_image();

};