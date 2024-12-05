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
    uint8_t               m_create_id = 2; //head
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

    // //xcb
    // xcb_connection_t* connection = nullptr;
    // xcb_window_t root;
    // uint16_t width = 0;
    // uint16_t height = 0;
    // xcb_get_geometry_reply_t* geometry = nullptr; 
    // xcb_shm_get_image_reply_t* xcb_image = nullptr;
    // xcb_shm_seg_t* shmseg = nullptr;
    // xcb_shm_create_segment_reply_t* shm_reply = nullptr;
    // void* shmem = nullptr;
    // const int SHM_SIZE = 4 * 1024 * 1024;

    // void _xcb_init();

    // void _xcb_make_screenshot();

    // uint8_t* _xcb_get_data();

    // int _xcb_get_linesize();


    //normal xcb-shm
    xcb_connection_t* m_connection = nullptr;
    const xcb_setup_t* m_setup = nullptr;
    xcb_screen_t* m_xcb_screen = nullptr;
    xcb_shm_seg_t m_seg = 0;
    int m_shmid = 0;
    xcb_shm_get_image_cookie_t m_cookie;
    uint8_t* m_buffer = nullptr;
    //xcb_shm_get_image_reply_t* m_xcb_image = nullptr;


    void _xcb_shm_inittialize()
    {
        this->m_connection = xcb_connect(nullptr, nullptr);
        if (xcb_connection_has_error(this->m_connection)) 
        {
            /* do something on error*/
            std::cerr << "ERROR\n";
            exit(0);
        }

        this->m_setup = xcb_get_setup(this->m_connection);
        this->m_xcb_screen = xcb_setup_roots_iterator(this->m_setup).data;
        this->m_seg = xcb_generate_id(this->m_connection);

        this->m_shmid = shmget(IPC_PRIVATE, 1366 * 768 * 4, IPC_CREAT | 0777);
        if (this->m_shmid == -1)
        {
            /* do something on error*/
            std::cerr << "ERROR\n";
            exit(0);
        }

        xcb_shm_attach(this->m_connection, this->m_seg, this->m_shmid, false);

        this->m_buffer = static_cast<uint8_t*>(shmat(this->m_shmid, nullptr, 0));
    }

    void _xcb_shm_create_image()
    {
        this->m_cookie = xcb_shm_get_image_unchecked(this->m_connection, this->m_xcb_screen->root, 0, 0, 1366,
                                             768, ~0,
                                             XCB_IMAGE_FORMAT_Z_PIXMAP, this->m_seg, 0);
                                             
        //this->m_xcb_image = xcb_shm_get_image_reply(this->m_connection, this->m_cookie, nullptr);
        //xcb_flush(this->m_connection);
        //std::cout << *m_buffer << '\n';
        free(xcb_shm_get_image_reply(this->m_connection, this->m_cookie, nullptr));
    }

};