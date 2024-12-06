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

    this->m_shmid = shmget(IPC_PRIVATE, this->m_display_width * this->m_display_height * 4, IPC_CREAT | 0777);
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
    this->m_cookie = xcb_shm_get_image_unchecked(this->m_connection, this->m_xcb_screen->root, 0, 0, this->m_display_width,
                                                 this->m_display_height, ~0,
                                                 XCB_IMAGE_FORMAT_Z_PIXMAP, this->m_seg, 0);
                                             
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
    //инициализация xcb-shm
    _xcb_shm_inittialize();

    //инициализирует libavdevice и регистрирует все входные и выходные устройства.
    avdevice_register_all();

    //auto detect the output format from the name
    this->m_output_format = av_guess_format(NULL, this->m_output_filename, NULL);
        // check on error code

    // инициализация структуры AVFormatContext для вывода
    if (avformat_alloc_output_context2(&this->m_format_ctx, this->m_output_format, NULL, this->m_output_filename) < 0) { 
        PRINT_DEBUG_ERROR;
    }

    //stream creation + parameters
    this->m_stream = avformat_new_stream(this->m_format_ctx, 0);
    if (!this->m_stream) {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }

    uint16_t time_base_den = this->m_video_fps * 1000;

    // Настройка параметров кодека в потоке(st)
    this->m_stream->codecpar->codec_id = this->m_output_format->video_codec;
    this->m_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    this->m_stream->codecpar->width = this->m_display_width;
    this->m_stream->codecpar->height = this->m_display_height;
    this->m_stream->time_base.num = 1;
    this->m_stream->time_base.den = time_base_den;

    // Инициализация кодека(поиск доступных кодеков)
    const AVCodec *pCodec = avcodec_find_encoder(this->m_stream->codecpar->codec_id);
    if (!pCodec) {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }

    // Выделение памяти для AVCodecContext и установка значения по умолчанию для его полей
    this->m_codec_ctx = avcodec_alloc_context3(pCodec);
    if (!this->m_codec_ctx) {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }

    // Перенос параметров кодека из AVCodecParameters(st->codecpar) в AVCodecContext(cctx)
    avcodec_parameters_to_context(this->m_codec_ctx, this->m_stream->codecpar);
    // Измнение параметров для кодека
    this->m_codec_ctx->bit_rate = 4000000;
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

    // Перенос изменённых настроек обратно в параметры кодека в объекте потока
    avcodec_parameters_from_context(this->m_stream->codecpar, this->m_codec_ctx);

    // Вывод подробной информации оформате ввода и вывода
    av_dump_format(this->m_format_ctx, 0, this->m_output_filename, 1);


    //OPEN FILE + WRITE HEADER
    // Инициализация AVCodecContext с использованием заданного AVCodec
    if (avcodec_open2(this->m_codec_ctx, pCodec, NULL) < 0) {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }
    // Проверка на флаги(открыт ли какой-либо файл)
    if (!(this->m_format_ctx->flags & AVFMT_NOFILE)) {
        // Открытие выходного файла
        if (avio_open(&this->m_format_ctx->pb, this->m_output_filename, AVIO_FLAG_WRITE) < 0) {
            PRINT_DEBUG_ERROR;
            exit(EXIT_FAILURE);
        }
    }
    // Выделение частных данных потока и запись заголовка потока в выходной медиафайл
    if (avformat_write_header(this->m_format_ctx, NULL) < 0) {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }

    _xlib_get_image();

    this->m_src_img_data[0] = this->m_buffer;
    this->m_scr_img_stride[0] = this->m_image->bytes_per_line;

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
    std::thread key_loop_thr(&X11_MultimediaCentre::_video_input_key_loop, this);
    std::thread create_thr(&X11_MultimediaCentre::_create_image_loop, this);
    std::thread write_thr(&X11_MultimediaCentre::_write_image_loop, this);
    key_loop_thr.join();
    create_thr.join();
    write_thr.join();
}


void X11_MultimediaCentre::_finish_video()
{
    // Запись трейлера потока в выходной медиафайл и освобождение частных данных файла.
    av_write_trailer(this->m_format_ctx);
    // Если всё сброшенно, то закрывается io поток
    if (!(this->m_output_format->flags & AVFMT_NOFILE)) {
        if (avio_close(this->m_format_ctx->pb) < 0) {
            PRINT_DEBUG_ERROR;
            exit(EXIT_FAILURE);
        }
    }
    // Освобождение всей занятой памяти
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
    this->m_create_delta_id = this->m_write_id - this->m_create_id + 1;
    (this->*m_create_throttle[this->m_create_delta_id])();
}


void X11_MultimediaCentre::_create_fake_func()
{
    while(this->m_write_status.load()){}
}


void X11_MultimediaCentre::_create_core_func()
{
    this->m_create_status.store(true);
    _xcb_shm_create_image();
    sws_scale(m_sws_ctx,
              this->m_src_img_data,
              this->m_scr_img_stride,
              0,
              this->m_ring_buffer[this->m_create_id]->height,
              this->m_ring_buffer[this->m_create_id]->data,
              this->m_ring_buffer[this->m_create_id]->linesize);
    ++this->m_create_id;
    this->m_create_id <<= this->m_size_of_bit_offset;
    this->m_create_id >>= this->m_size_of_bit_offset;
    this->m_create_status.store(false);
}


void X11_MultimediaCentre::_write_image_loop()
{
    while(this->m_program_state.load() || this->m_write_status.load())
    {
        _write_image();
    }
}


void X11_MultimediaCentre::_write_image()
{
    this->m_write_delta_id = this->m_create_id - this->m_write_id;
    (this->*m_write_throttle[this->m_write_delta_id])();
}


void X11_MultimediaCentre::_write_fake_func()
{
    this->m_write_status.store(false);
    while(this->m_create_status.load()){}
}


void X11_MultimediaCentre::_write_core_func()
{
    this->m_write_status.store(true);
    // Инициализация необязательных полей пакета(AVPacket) значениями по умолчанию
    this->m_packet->flags |= AV_PKT_FLAG_KEY; // AV_PKT_FLAG_KEY — это флаг, который указывает, что пакет содержит ключевой кадр.
    this->m_packet->pts = this->m_ring_buffer[this->m_write_id]->pts = this->m_video_pts;
    this->m_packet->data = NULL;
    this->m_packet->size = 0;
    this->m_packet->stream_index = m_stream->index;
    this->m_packet->duration = 0;
    // Передаём необработанный кадр в кодировщик(кодек)
    if (avcodec_send_frame(this->m_codec_ctx, m_ring_buffer[this->m_write_id]) < 0) {
        PRINT_DEBUG_ERROR;
    }
    // Чтение закодированных данных от энкодера
    if (avcodec_receive_packet(this->m_codec_ctx, m_packet) == 0) {
        // Запись пакета в выходной медиафайл, обеспечивая правильное чередование
        av_interleaved_write_frame(this->m_format_ctx, m_packet);
        // Освобождает пакет(возвращает значения по умполчанию)
        av_packet_unref(m_packet);
    }
    ++this->m_write_id;
    this->m_write_id <<= this->m_size_of_bit_offset;
    this->m_write_id >>= this->m_size_of_bit_offset;
    this->m_video_pts += 1000;
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