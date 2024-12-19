#include "../include/x11_multimedia_centre.hpp"

void X11_MultimediaCentre::_xcb_shm_inittialize()
{
    this->m_connection = xcb_connect(nullptr, nullptr);
    if (xcb_connection_has_error(this->m_connection))
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }

    this->m_setup = xcb_get_setup(this->m_connection);
    this->m_xcb_screen = xcb_setup_roots_iterator(this->m_setup).data;
    this->m_seg = xcb_generate_id(this->m_connection);

    this->m_shmid = shmget(IPC_PRIVATE,
                           this->m_display_width * this->m_display_height * 4,
                           IPC_CREAT | 0777);
    if (this->m_shmid == -1)
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }

    xcb_shm_attach(this->m_connection, this->m_seg, this->m_shmid, false);

    this->m_buffer = static_cast<uint8_t*>(shmat(this->m_shmid, nullptr, 0));
}


void X11_MultimediaCentre::_xcb_shm_create_image()
{
    this->m_cookie = xcb_shm_get_image_unchecked(this->m_connection,
                                                 this->m_xcb_screen->root,
                                                 0,
                                                 0,
                                                 this->m_display_width,
                                                 this->m_display_height,
                                                 ~0,
                                                 XCB_IMAGE_FORMAT_Z_PIXMAP,
                                                 this->m_seg,
                                                 0);
                                             
    //xcb_flush(this->m_connection);
    free(xcb_shm_get_image_reply(this->m_connection, this->m_cookie, nullptr));
}


void X11_MultimediaCentre::set_video_filename(char* name)
{
    this->m_output_filename = name;
}


void X11_MultimediaCentre::set_video_fps(uint16_t fps)
{
    this->m_video_fps = fps;
}


void X11_MultimediaCentre::initialize()
{
    this->m_display = XOpenDisplay(getenv("DISPLAY"));
    if(this->m_display == NULL)
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }
    
    this->m_screen = XDefaultScreen(this->m_display);
    this->m_root_window = XDefaultRootWindow(this->m_display);
    this->m_display_height = XDisplayHeight(this->m_display, this->m_screen);
    this->m_display_width = XDisplayWidth(this->m_display, this->m_screen);
}


void X11_MultimediaCentre::start_video()
{
    std::cout << "\tStart video!\n \tRecording...\n";
    _video_initialize();
    _video_start_all_threads();
    _finish_video();
    system("killall ffmpeg");
    //system("ffmpeg -i testing_video.mp4 -vf 'setpts=1.5*PTS' new_video.mp4");
    system("ffmpeg -i testing_video.mp4 -i microphone_record.mp3 -c:v copy -c:a aac output.mp4");
    std::cout << "\tEnd video!\n";
}


void X11_MultimediaCentre::stop_video()
{
    this->m_program_state.store(false);
}


void X11_MultimediaCentre::set_trigger_key(int key,  unsigned int mask)
{
    XGrabKey(this->m_display,
             XKeysymToKeycode(this->m_display, key),
             mask,
             this->m_root_window,
             false,
             GrabModeAsync,
             GrabModeAsync);
}


void X11_MultimediaCentre::_video_initialize()
{
    // Initialize xcb-shm.
    _xcb_shm_inittialize();

    // Initializes libavdevice and registers all input and output devices.
    avdevice_register_all();

    // Auto detect the output format from the name
    this->m_output_format = av_guess_format(NULL,
                                            this->m_output_filename,
                                            NULL);

    // Initializing the AVFormatContext structure for output.
    if (avformat_alloc_output_context2(&this->m_format_ctx,
                                        this->m_output_format,
                                        NULL,
                                        this->m_output_filename) < 0)
    { 
        PRINT_DEBUG_ERROR;
    }

    // Stream creation + parameters
    this->m_stream = avformat_new_stream(this->m_format_ctx, 0);
    if (!this->m_stream) {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }

    // Initialize total time_base, based on fps.
    uint16_t time_base_den = this->m_video_fps * 1000;

    // Configuring codec parameters in the stream(m_stream).
    this->m_stream->codecpar->codec_id = this->m_output_format->video_codec;
    this->m_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    this->m_stream->codecpar->width = this->m_display_width;
    this->m_stream->codecpar->height = this->m_display_height;
    this->m_stream->time_base.num = 1;
    this->m_stream->time_base.den = time_base_den;

    // Initializing the codec (searching for available codecs).
    const AVCodec *pCodec = avcodec_find_encoder(
                                            this->m_stream->codecpar->codec_id);
    if (!pCodec)
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }

    /*
    Allocating memory for AVCodecContext
    and setting the default value for its fields.
    */
    this->m_codec_ctx = avcodec_alloc_context3(pCodec);
    if (!this->m_codec_ctx)
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }

    /*
    Transferring codec parameters from AVCodecParameters(m_stream->codecpar)
    to AVCodecContext(m_codec_ctx).
    */
    avcodec_parameters_to_context(this->m_codec_ctx, this->m_stream->codecpar);
    
    // Changing the parameters for the codec.
    this->m_codec_ctx->bit_rate = 5000000;
    this->m_codec_ctx->width = this->m_display_width;
    this->m_codec_ctx->height = this->m_display_height;
    this->m_codec_ctx->time_base.num = 1;
    this->m_codec_ctx->time_base.den = time_base_den;
    this->m_codec_ctx->pkt_timebase.num = 1;
    this->m_codec_ctx->pkt_timebase.den = time_base_den;
    this->m_codec_ctx->gop_size = 0;
    this->m_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    this->m_codec_ctx->has_b_frames = 0;
    this->m_codec_ctx->max_b_frames = 0;

    /*
    Transferring the changed settings back to
    the codec parameters in the stream object.
    */ 
    avcodec_parameters_from_context(this->m_stream->codecpar,this->m_codec_ctx);

    // Output detailed information about the input and output format.
    av_dump_format(this->m_format_ctx, 0, this->m_output_filename, 1);


    /* OPEN FILE + WRITE HEADER */
    // Initialize AVCodecContext with using configured AVCodec.
    if (avcodec_open2(this->m_codec_ctx, pCodec, NULL) < 0)
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }
    // Checking for flags (whether any file is open).
    if (!(this->m_format_ctx->flags & AVFMT_NOFILE))
    {
        // Open output file.
        if (avio_open(&this->m_format_ctx->pb,
            this->m_output_filename,
            AVIO_FLAG_WRITE) < 0)
        {
            PRINT_DEBUG_ERROR;
            exit(EXIT_FAILURE);
        }
    }
    /*
    Allocating private stream data and
    writing the stream header to the output media file.
    */ 
    if (avformat_write_header(this->m_format_ctx, NULL) < 0)
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }


    // Make a screenshot using Xlib(only for init).
    _xlib_get_image();


    /*
    Link buffer with image to array for sws_ctx.
    buffer - pointer to share memory where the image data stored and updated.
    */ 
    this->m_src_img_data[0] = this->m_buffer;
    this->m_scr_img_stride[0] = this->m_image->bytes_per_line;


    // Initialize ring buffer.
    this->m_ring_buffer.resize(this->m_ring_buffer_size, nullptr);
    for(uint8_t i = 0; i < this->m_ring_buffer_size; ++i)
    {
        this->m_ring_buffer[i] = av_frame_alloc();
        this->m_ring_buffer[i]->format = this->m_codec_ctx->pix_fmt;
        this->m_ring_buffer[i]->width = this->m_codec_ctx->width;
        this->m_ring_buffer[i]->height = this->m_codec_ctx->height;
        this->m_ring_buffer[i]->time_base = {1, time_base_den};
        av_image_alloc(this->m_ring_buffer[i]->data,
                       this->m_ring_buffer[i]->linesize,
                       this->m_codec_ctx->width,
                       this->m_codec_ctx->height,
                       this->m_codec_ctx->pix_fmt,
                       32);
    }

    /*
    Initialize the size of bit-offset.
    Check _write_core_func() and _create_core_func().
    */
    uint8_t temp = this->m_ring_buffer_size;
    uint8_t count_of_bits = 0;
    while(temp != 0)
    {
        temp >>= 1;
        ++count_of_bits;
    }
    --count_of_bits;
    this->m_size_of_bit_offset = 8 - count_of_bits; // 8 because uint8_t


    m_sws_ctx = sws_getContext(this->m_ring_buffer[0]->width,
                               this->m_ring_buffer[0]->height,
                               AV_PIX_FMT_BGRA,
                               this->m_ring_buffer[0]->width,
                               this->m_ring_buffer[0]->height,
                               AV_PIX_FMT_YUV420P,
                               SWS_FAST_BILINEAR,
                               NULL, NULL, NULL);

    this->m_packet = av_packet_alloc();
}


inline void X11_MultimediaCentre::_video_input_key_loop()
{
    XNextEvent(this->m_display, &this->m_event);
    while(true)
    {
        if(this->m_event.type == KeyPress)
        {
            this->m_program_state.store(false);
            break;
        }
    }
}


void X11_MultimediaCentre::_video_start_all_threads()
{
    std::thread key_loop_thr(&X11_MultimediaCentre::_video_input_key_loop,this);
    std::thread create_thr(&X11_MultimediaCentre::_create_image_loop, this);
    std::thread write_thr(&X11_MultimediaCentre::_write_image_loop, this);
    std::thread audio_thr(&X11_MultimediaCentre::_audio_record_start, this);
    audio_thr.detach();
    key_loop_thr.join();
    create_thr.join();
    write_thr.join();
}

void X11_MultimediaCentre::_audio_record_start()
{
system("ffmpeg -f pulse -i alsa_input.pci-0000_00_1b.0.analog-stereo -ac 1 microphone_record.mp3");
}

void X11_MultimediaCentre::_finish_video()
{
    /*
    Recording the stream trailer to the output media file and
    releasing the private data of the file.
    */
    av_write_trailer(this->m_format_ctx);
    // If everything is reset, the io stream is closed.
    if (!(this->m_output_format->flags & AVFMT_NOFILE))
    {
        if (avio_close(this->m_format_ctx->pb) < 0)
        {
            PRINT_DEBUG_ERROR;
            exit(EXIT_FAILURE);
        }
    }
    // Freeing up all occupied memory.
    for(int i = 0; i < this->m_ring_buffer.size(); ++i)
    {
        av_frame_free(&this->m_ring_buffer[i]);
    }
    avcodec_free_context(&this->m_codec_ctx);
    avformat_free_context(this->m_format_ctx);
}

void X11_MultimediaCentre::_create_image_loop()
{
    while(this->m_program_state.load())
    {
        _create_image();
    }
}


void X11_MultimediaCentre::_create_image()
{
    /*
    If m_create_delta_id == 0, then call the fake_func(waiting func).
    If m_create_delta_id == 1, then call the core_func(func for creating image).
    This replaces the if-else construct.
    No branch prediction, it`s good.
    */
    this->m_create_delta_id = this->m_write_id - this->m_create_id + 1;
    (this->*m_create_throttle[this->m_create_delta_id])();
}


void X11_MultimediaCentre::_create_fake_func()
{
    //nothing
}


void X11_MultimediaCentre::_create_core_func()
{
    // Create image start.
    this->m_create_status.store(true);
    // Get the image by xcb-shm.
    _xcb_shm_create_image();
    // Transform bgra image data to yuv420, and load his in the AVFrame.
    sws_scale(m_sws_ctx,
              this->m_src_img_data,
              this->m_scr_img_stride,
              0,
              this->m_ring_buffer[this->m_create_id]->height,
              this->m_ring_buffer[this->m_create_id]->data,
              this->m_ring_buffer[this->m_create_id]->linesize);
    // Select next element of ring_buffer.
    ++this->m_create_id;
    /*
    Reset the bits that should not be used.
    This is how the maximum ID is limited.
    The number of zeroed bits depends on the size of the ring buffer.
    See _video_initialize().
    */
    this->m_create_id <<= this->m_size_of_bit_offset;
    this->m_create_id >>= this->m_size_of_bit_offset;
    // Create image end.
    this->m_create_status.store(false);
}


void X11_MultimediaCentre::_write_image_loop()
{
    while(this->m_program_state.load()|| this->m_write_status.load())
    {
        _write_image();
    }
}


void X11_MultimediaCentre::_write_image()
{
    /*
    If m_write_delta_id == 0, then call the fake_func(waiting func).
    If m_write_delta_id == 1, then call the core_func(func for write).
    This replaces the if-else construct.
    No branch prediction, it`s good.
    */
    this->m_write_delta_id = this->m_create_id - this->m_write_id;
    (this->*m_write_throttle[this->m_write_delta_id])();
}


void X11_MultimediaCentre::_write_fake_func()
{
    this->m_write_status.store(false);
}


void X11_MultimediaCentre::_write_core_func()
{
    // Write image start.
    this->m_write_status.store(true);
    /*
    Initialization of optional package fields (AVPacket) with default values.
    AV_PKT_FLAG_KEY — this is a flag that
    indicates that the packet contains a keyframe.
    */
    this->m_packet->flags |= AV_PKT_FLAG_KEY; 
    this->m_packet->pts = this->m_video_pts;
    this->m_ring_buffer[this->m_write_id]->pts = this->m_video_pts;
    this->m_packet->data = NULL;
    this->m_packet->size = 0;
    this->m_packet->stream_index = m_stream->index;
    // Transfer the raw frame to the encoder(codec).
    if (avcodec_send_frame(
                        this->m_codec_ctx, m_ring_buffer[this->m_write_id]) < 0)
    {
        PRINT_DEBUG_ERROR;
    }
    // Reading encoded data from the encoder.
    if (avcodec_receive_packet(this->m_codec_ctx, m_packet) == 0)
    {
        /*
        Recording a packet to an output media file,
        ensuring proper interleaving.
        */
        //av_interleaved_write_frame(this->m_format_ctx, m_packet); // mb slow
        av_write_frame(this->m_format_ctx, m_packet);
        // Releases the package(returns the default values).
        av_packet_unref(m_packet);
    }
    // Select next element of ring_buffer.
    ++this->m_write_id;
    /*
    Reset the bits that should not be used.
    This is how the maximum ID is limited.
    The number of zeroed bits depends on the size of the ring buffer.
    See _video_initialize().
    */
    this->m_write_id <<= this->m_size_of_bit_offset;
    this->m_write_id >>= this->m_size_of_bit_offset;
    // Increase video_pts for next frame.
    this->m_video_pts += 1000;

    /*
    End write will be when all frames in ring buffer will be writed.
    P.S. Then _write_fake_func() will be active.
    */
}


void X11_MultimediaCentre::make_screenshot()
{
    _key_waiting_loop();
    _xlib_get_image();
}


void X11_MultimediaCentre::make_screenshot_right_now()
{
    _xlib_get_image();
}


inline void X11_MultimediaCentre::_key_waiting_loop()
{
    XNextEvent(this->m_display, &this->m_event);
    while(true)
    {
        if(this->m_event.type == KeyPress)
        {
            break;
        }
    }
}


void X11_MultimediaCentre::_xlib_get_image()
{
    this->m_image = XGetImage(this->m_display,
                              this->m_root_window,
                              0,
                              0,
                              this->m_display_width,
                              this->m_display_height,
                              AllPlanes,
                              ZPixmap);
}


char* X11_MultimediaCentre::get_screenshot_data()
{
    _check_img_ptr();
    return this->m_image->data;
}


int X11_MultimediaCentre::get_screenshot_row_bytecount()
{
    _check_img_ptr();
    return this->m_image->bytes_per_line;
}


unsigned short X11_MultimediaCentre::get_display_width()
{
    return this->m_display_width;
}


unsigned short X11_MultimediaCentre::get_display_height()
{
    return this->m_display_height;
}


void X11_MultimediaCentre::_check_img_ptr()
{
    if(this->m_image == nullptr)
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }
}