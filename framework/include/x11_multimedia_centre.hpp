#pragma once

#include <iostream>
#include <xcb/xcb.h>
#include <xcb/shm.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <xcb/xproto.h>
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

#include "i_multimedia_centre.hpp"

#undef CLASS_NAME
#define CLASS_NAME "X11_MultimediaCentre"
#define PRINT_LINE std::cerr << "Line->" << __LINE__ << '\n';
#define PRINT_FUNC std::cerr << "Function: " << __func__ << ' ';
#define PRINT_ERROR_MESSAGE(m) std::cerr << "ERROR::" << m << ' '; 
#define PRINT_DEBUG_ERROR PRINT_ERROR_MESSAGE(CLASS_NAME) PRINT_FUNC PRINT_LINE

class X11_MultimediaCentre : public IMultimediaCentre
{
public:
    void            initialize();
    void            make_screenshot();
    void            make_screenshot_right_now();
    void            start_video();
    void            stop_video();

    void            set_trigger_key(int key,  unsigned int mask = 0);
    void            set_video_filename(char* name);
    void            set_video_fps(uint16_t fps);

    char*           get_screenshot_data();
    int             get_screenshot_row_bytecount();
    unsigned short  get_display_width();
    unsigned short  get_display_height();



private:
    void _xlib_get_image();

    void _check_img_ptr();
    inline void _key_waiting_loop();

    void _video_initialize();
    inline void _video_input_key_loop();
    void _video_start_all_threads();
    void _finish_video();

    void _write_image_loop();//tread func
      void _write_image();//selector func
        void _write_fake_func();//standby func
        void _write_core_func();//main logick func

    void _create_image_loop();//thread func
      void _create_image();//selector func
        void _create_fake_func();//standby func
        void _create_core_func();//main logick func


    void (X11_MultimediaCentre::*m_write_throttle[2])() = {
        &X11_MultimediaCentre::_write_fake_func,
        &X11_MultimediaCentre::_write_core_func};

    void (X11_MultimediaCentre::*m_create_throttle[2])()  = {
        &X11_MultimediaCentre::_create_fake_func,
        &X11_MultimediaCentre::_create_core_func};

    void _xcb_shm_inittialize();
    void _xcb_shm_create_image();

    std::vector<AVFrame*>       m_ring_buffer;
    uint8_t*                    m_src_img_data[8] = {NULL};
    int                         m_scr_img_stride[8] = {0};
    AVStream*                   m_stream = nullptr;
    AVPacket*                   m_packet = nullptr;
    AVCodecContext*             m_codec_ctx = nullptr;
    AVFormatContext*            m_format_ctx = nullptr;
    SwsContext*                 m_sws_ctx = nullptr;
    const AVOutputFormat*       m_output_format;
    XImage*                     m_image = nullptr;
    Display*                    m_display = nullptr;
    XEvent                      m_event;
    Window                      m_root_window;
    char*                       m_output_filename = "video.mp4";
    uint8_t*                    m_buffer = nullptr;
    xcb_connection_t*           m_connection = nullptr;
    const xcb_setup_t*          m_setup = nullptr;
    xcb_screen_t*               m_xcb_screen = nullptr;
    uint64_t                    m_video_pts = 0;
    int                         m_video_key_code = 0;
    int                         m_screenshot_key_code = 0;
    int                         m_screen = 0;
    xcb_shm_seg_t               m_seg;
    int                         m_shmid;
    xcb_shm_get_image_cookie_t  m_cookie;
    unsigned short              m_display_height = 0;
    unsigned short              m_display_width = 0;
    uint16_t                    m_video_fps = 24;
    uint8_t                     m_create_id = 1; //head
    uint8_t                     m_write_id = 0; //tail
    uint8_t                     m_size_of_bit_offset = 6;
    uint8_t                     m_ring_buffer_size = 4;
    std::atomic<bool>           m_metronome_state{true};
    std::atomic<bool>           m_program_state{true};
    std::atomic<bool>           m_create_status{false};
    std::atomic<bool>           m_write_status{false};
    bool                        m_create_delta_id = 0;
    bool                        m_write_delta_id = 0;
};