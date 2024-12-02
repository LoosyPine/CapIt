#include "../include/x11_screenshot_maker.hpp"

void X11ScreenshotMaker::initialize()
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

void X11ScreenshotMaker::va_initialize()
{
    XGrabKey(this->m_display,
            XKeysymToKeycode(this->m_display, XK_R),
            ControlMask,
            this->m_root_window,
            false,
            GrabModeAsync,
            GrabModeAsync);


    char *filename = "tmp.mp4"; //имя файла
    const AVOutputFormat *fmt; //создаём объект для настройки выходящего файла
    AVFormatContext *fctx; //Объект для настройки формата
    AVCodecContext *cctx; //Объект для настройки кодека
    AVStream *st; //Объект для настройки и заполнения потока(видео)

    //инициализирует libavdevice и регистрирует все входные и выходные устройства.
    avdevice_register_all();

    //auto detect the output format from the name
    fmt = av_guess_format(NULL, filename, NULL);
        // check on error code

    // инициализация структуры AVFormatContext для вывода
    if (avformat_alloc_output_context2(&fctx, fmt, NULL, filename) < 0) { 
        // ERROR
    }

    //stream creation + parameters
    st = avformat_new_stream(fctx, 0);
    if (!st) {
        // ERROR
    }

    // Настройка параметров кодека в потоке(st)
    st->codecpar->codec_id = fmt->video_codec;
    st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    st->codecpar->width = 1366;
    st->codecpar->height = 768;
    st->time_base.num = 1;
    st->time_base.den = 30000;
    //st->avg_frame_rate = {1, 30};

    // Инициализация кодека(поиск доступных кодеков)
    const AVCodec *pCodec = avcodec_find_encoder(st->codecpar->codec_id);
    if (!pCodec) {
        // ERROR
    }

    // Выделение памяти для AVCodecContext и установка значения по умолчанию для его полей
    cctx = avcodec_alloc_context3(pCodec);
    if (!cctx) {
        // ERROR
    }

    // Перенос параметров кодека из AVCodecParameters(st->codecpar) в AVCodecContext(cctx)
    avcodec_parameters_to_context(cctx, st->codecpar);
    // Измнение параметров для кодека
    cctx->bit_rate = 400000000;
    cctx->width = 1366;
    cctx->height = 768;
    cctx->time_base.num = 1;
    cctx->time_base.den = 30000;
    //cctx->gop_size = 0;
    cctx->pix_fmt = AV_PIX_FMT_YUV444P;

    /* ДОПОЛНИТЕЛЬНЫЕ НАСТРОЙКИ КОДЕКА */
    // if (st->codecpar->codec_id == AV_CODEC_ID_H264) {
    //     av_opt_set(cctx->priv_data, "preset", "ultrafast", 0);
    // }
    // if (fctx->oformat->flags & AVFMT_GLOBALHEADER) {
    //     cctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    // }

    // Перенос изменённых настроек обратно в параметры кодека в объекте потока
    avcodec_parameters_from_context(st->codecpar, cctx);


    // Вывод подробной информации оформате ввода и вывода
    av_dump_format(fctx, 0, filename, 1);


    //OPEN FILE + WRITE HEADER
    // Инициализация AVCodecContext с использованием заданного AVCodec
    if (avcodec_open2(cctx, pCodec, NULL) < 0) {
        // ERROR
    }
    // Проверка на флаги(открыт ли какой-либо файл)
    if (!(fmt->flags & AVFMT_NOFILE)) {
        // Открытие выходного файла
        if (avio_open(&fctx->pb, filename, AVIO_FLAG_WRITE) < 0) {
            // ERROR
        }
    }
    // Выделение частных данных потока и запись заголовка потока в выходной медиафайл
    if (avformat_write_header(fctx, NULL) < 0) {
        // ERROR
    }

    // Создание объекта и выделение памяти для структуры AVFrame
    AVFrame *frame = av_frame_alloc();
    // Настройка параметров фрагмента
    frame->format = cctx->pix_fmt;
    frame->width = cctx->width;
    frame->height = cctx->height;
    // Выделение памяти для изображения с заданными размерами и форматом пикселей.
    av_image_alloc(frame->data, frame->linesize, cctx->width, cctx->height, cctx->pix_fmt, 32);

    //std::cout << cctx->width << ' ' << cctx->height << '\n';
    //std::cout << frame->linesize[0] << '\n';
    //std::cout << frame->linesize[1] << '\n';
    //std::cout << frame->linesize[2] << '\n';
    //std::cout << this->m_image->bytes_per_line << '\n';
    
    // Создание объекта AVPacket для хранения сжатых данных
    AVPacket pkt;

   //ТЕСТ
    //uint8_t* img_data[8] = {(uint8_t*)this->m_image->data};
                // this->m_image = XGetImage(this->m_display,
                //             DefaultRootWindow(this->m_display),
                //             0,
                //             0,
                //             this->m_display_width,
                //             this->m_display_height,
                //             AllPlanes,
                //             ZPixmap);

//    AVFrame* frame_BGRA = av_frame_alloc();
//    frame_BGRA->format = AV_PIX_FMT_BGRA;
//    frame_BGRA->width = cctx->width;
//    frame_BGRA->height = cctx->height;
//    av_image_alloc(frame_BGRA->data, frame_BGRA->linesize, cctx->width, cctx->height, AV_PIX_FMT_BGRA, 32);
//    //av_image_fill_pointers(frame_BGRA->data, AV_PIX_FMT_BGR32, frame_BGRA->height, (uint8_t*)this->m_image->data, frame_BGRA->linesize);
    std::cout << "METKA\n";
    const uint8_t* src_data[8] = {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL};

    int scr_image_stride[8] = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0};



   //std::cout << frame_BGRA->linesize[0] << std::endl;
   //frame_BGRA->data[0] = (uint8_t*)this->m_image->data;
//    for (int y = 0; y < cctx->height; y++) {
//             int l = 0;
//             for (int x = 0; x < cctx->width; x++) {
//                 frame_BGRA->data[0][y * frame_BGRA->linesize[0] + x] = (uint8_t)(this->m_image->data[((y * this->m_image->bytes_per_line)) + l]);
//                 //frame_BGRA->data[0][y * frame_BGRA->linesize[0] + (++x)] = (uint8_t)(this->m_image->data[((y * this->m_image->bytes_per_line)) + (++l)]);
//                 //frame_BGRA->data[0][y * frame_BGRA->linesize[0] + (++x)] = (uint8_t)(this->m_image->data[((y * this->m_image->bytes_per_line)) + (++l)]);

//                 frame_BGRA->data[1][y * frame_BGRA->linesize[1] + (x)] = (uint8_t)(this->m_image->data[((y * this->m_image->bytes_per_line)) + (++l)]);
//                 //frame_BGRA->data[1][y * frame_BGRA->linesize[1] + (++x)] = (uint8_t)(this->m_image->data[((y * this->m_image->bytes_per_line)) + (++l)]);
//                 //frame_BGRA->data[1][y * frame_BGRA->linesize[1] + (++x)] = (uint8_t)(this->m_image->data[((y * this->m_image->bytes_per_line)) + (++l)]);

//                 frame_BGRA->data[2][y * frame_BGRA->linesize[2] + (x)] = (uint8_t)(this->m_image->data[((y * this->m_image->bytes_per_line)) + (++l)]);
//                 //frame_BGRA->data[2][y * frame_BGRA->linesize[2] + (++x)] = (uint8_t)(this->m_image->data[((y * this->m_image->bytes_per_line)) + (++l)]);
//                 //frame_BGRA->data[2][y * frame_BGRA->linesize[2] + (++x)] = (uint8_t)(this->m_image->data[((y * this->m_image->bytes_per_line)) + (++l)]);

//                 ++l;
//                 ++l;
//                 //++l;
            
//             }
//         }

    SwsContext* sws_ctx = sws_getContext(frame->width, frame->height, AV_PIX_FMT_BGRA,
                                        frame->width, frame->height, AV_PIX_FMT_YUV444P, SWS_FAST_BILINEAR,
                                        NULL, NULL, NULL);

    std::cout << "ДО ЦИКЛА\n";
    double video_pts = 0;
    unsigned int i = 0;
    for (i = 0; i < 500; ++i) {
        //video_pts = ((double)cctx->time_base.num / cctx->time_base.den) * (i * 1000);
        // XNextEvent(this->m_display, &this->m_event);
        // if(this->m_event.type == KeyPress)
        // {

        //     break;
        // }

        video_pts = i * 1000;
        ++i;
        std::cout << video_pts << std::endl;

            this->m_image = XGetImage(this->m_display,
                            DefaultRootWindow(this->m_display),
                            0,
                            0,
                            this->m_display_width,
                            this->m_display_height,
                            AllPlanes,
                            ZPixmap);
            src_data[0] = (const uint8_t*)this->m_image->data;
            scr_image_stride[0] = this->m_image->bytes_per_line;
            sws_scale(sws_ctx, src_data, scr_image_stride, 0, frame->height, frame->data, frame->linesize);
         // Заполнение кадра
        // for (int y = 0; y < cctx->height; y++) {
        //     int l = 0;
        //     for (int x = 0; x < cctx->width; x++) {
        //         frame->data[0][y * frame->linesize[0] + x] = (uint8_t)(this->m_image->data[((y * this->m_image->bytes_per_line)) + l]);
        //         frame->data[1][y * frame->linesize[1] + x] = (uint8_t)(this->m_image->data[((y * this->m_image->bytes_per_line)) + (++l)]);
        //         frame->data[2][y * frame->linesize[2] + x] = (uint8_t)(this->m_image->data[((y * this->m_image->bytes_per_line)) + (++l)]);
        //         ++l;
        //         ++l;
            
        //     }
        // }


        // Инициализация необязательных полей пакета(AVPacket) значениями по умолчанию
        av_init_packet(&pkt);
        pkt.flags |= AV_PKT_FLAG_KEY; // AV_PKT_FLAG_KEY — это флаг, который указывает, что пакет содержит ключевой кадр.
        pkt.pts = frame->pts = video_pts;
        //pkt.dts = pkt.pts;
        pkt.data = NULL;
        pkt.size = 0;
        pkt.stream_index = st->index;
        pkt.duration = 0.016;

        // Передаём необработанный кадр в кодировщик(кодек)
        if (avcodec_send_frame(cctx, frame) < 0) {
            // ERROR
        }
        // Чтение закодированных данных от энкодера
        if (avcodec_receive_packet(cctx, &pkt) == 0) {
            // Запись пакета в выходной медиафайл, обеспечивая правильное чередование
            av_interleaved_write_frame(fctx, &pkt);
            // Освобождает пакет(возвращает значения по умполчанию)
            av_packet_unref(&pkt);
        }
    }
    //av_packet_rescale_ts(&pkt, {1, 60000}, st->time_base); //this->m_stream->time_base
    // //DELAYED FRAMES(~Задержка кадров)
    // for (;;) {
    //     avcodec_send_frame(cctx, NULL);
    //     if (avcodec_receive_packet(cctx, &pkt) == 0) {
    //         av_interleaved_write_frame(fctx, &pkt);
    //         av_packet_unref(&pkt);
    //     }
    //     else {
    //         break;
    //     }
    // }

    //FINISH
    // Запись трейлера потока в выходной медиафайл и освобождение частных данных файла.
    av_write_trailer(fctx);
    // Если всё сброшенно, то закрывается io поток
    if (!(fmt->flags & AVFMT_NOFILE)) {
        if (avio_close(fctx->pb) < 0) {
            // ERROR
        }
    }
    // Освобождение всей занятой памяти
    av_frame_free(&frame);
    avcodec_free_context(&cctx);
    avformat_free_context(fctx);
}

//Video part

void X11ScreenshotMaker::start_video()
{
    std::cout << "Start video!\n";
     _video_initialize();
     _start_all_threads();
     _finish_video();
    //va_initialize();
    std::cout << "End video!\n";
}


void X11ScreenshotMaker::_video_initialize()
{
    //инициализирует libavdevice и регистрирует все входные и выходные устройства.
    avdevice_register_all();

    //auto detect the output format from the name
    this->m_output_format = av_guess_format(NULL, this->m_output_filename, NULL);
        // check on error code

    // инициализация структуры AVFormatContext для вывода
    if (avformat_alloc_output_context2(&this->m_format_ctx, this->m_output_format, NULL, this->m_output_filename) < 0) { 
        // ERROR
    }

    //stream creation + parameters
    this->m_stream = avformat_new_stream(this->m_format_ctx, 0);
    if (!this->m_stream) {
        // ERROR
    }

    // Настройка параметров кодека в потоке(st)
    this->m_stream->codecpar->codec_id = this->m_output_format->video_codec;
    this->m_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    this->m_stream->codecpar->width = this->m_display_width;
    this->m_stream->codecpar->height = this->m_display_height;
    this->m_stream->time_base.num = 1;
    this->m_stream->time_base.den = 24000;
    //this->m_stream->time_base = {1.0, 30000};
    //this->m_stream->r_frame_rate = AVRational{1, 30};

    // Инициализация кодека(поиск доступных кодеков)
    const AVCodec *pCodec = avcodec_find_encoder(this->m_stream->codecpar->codec_id);
    if (!pCodec) {
        // ERROR
    }

    // Выделение памяти для AVCodecContext и установка значения по умолчанию для его полей
    this->m_codec_ctx = avcodec_alloc_context3(pCodec);
    if (!this->m_codec_ctx) {
        // ERROR
    }

    // Перенос параметров кодека из AVCodecParameters(st->codecpar) в AVCodecContext(cctx)
    avcodec_parameters_to_context(this->m_codec_ctx, this->m_stream->codecpar);
    // Измнение параметров для кодека
    this->m_codec_ctx->bit_rate = 4000000;
    this->m_codec_ctx->width = this->m_display_width;
    this->m_codec_ctx->height = this->m_display_height;
    this->m_codec_ctx->time_base.num = 1;
    this->m_codec_ctx->time_base.den = 24000;
    //this->m_codec_ctx->time_base = {1, 30000};
    //this->m_codec_ctx->pkt_timebase = {1, 30000};
    this->m_codec_ctx->pkt_timebase.num = 1;
    this->m_codec_ctx->pkt_timebase.den = 24000;
    this->m_codec_ctx->gop_size = 0;
    this->m_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    //this->m_codec_ctx->framerate = {30, 1};
    this->m_codec_ctx->has_b_frames = 0;
    this->m_codec_ctx->max_b_frames = 0;


    /* ДОПОЛНИТЕЛЬНЫЕ НАСТРОЙКИ КОДЕКА */
    //  if (this->m_stream->codecpar->codec_id == AV_CODEC_ID_H264)
    //  {
    //      av_opt_set(this->m_codec_ctx->priv_data, "vsync", "0", 0);
    //  }
    //  if (this->m_format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
    //  {
    //      this->m_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    //  }

    // Перенос изменённых настроек обратно в параметры кодека в объекте потока
    avcodec_parameters_from_context(this->m_stream->codecpar, this->m_codec_ctx);

    // Вывод подробной информации оформате ввода и вывода
    av_dump_format(this->m_format_ctx, 0, this->m_output_filename, 1);


    //OPEN FILE + WRITE HEADER
    // Инициализация AVCodecContext с использованием заданного AVCodec
    if (avcodec_open2(this->m_codec_ctx, pCodec, NULL) < 0) {
        // ERROR
    }
    // Проверка на флаги(открыт ли какой-либо файл)
    if (!(this->m_format_ctx->flags & AVFMT_NOFILE)) {
        // Открытие выходного файла
        if (avio_open(&this->m_format_ctx->pb, this->m_output_filename, AVIO_FLAG_WRITE) < 0) {
            // ERROR
        }
    }
    // Выделение частных данных потока и запись заголовка потока в выходной медиафайл
    if (avformat_write_header(this->m_format_ctx, NULL) < 0) {
        // ERROR
    }

    for(uint8_t i = 0; i < 8; ++i)
    {
        this->m_src_img_data[i] = NULL;
        this->m_scr_img_stride[i] = 0;
    }
    this->m_image = XGetImage(this->m_display,
                              this->m_root_window,
                              0,
                              0,
                              this->m_display_width,
                              this->m_display_height,
                              AllPlanes,
                              ZPixmap);
    this->m_src_img_data[0] = (uint8_t*)this->m_image->data;
    this->m_scr_img_stride[0] = this->m_image->bytes_per_line;

    this->m_ring_buffer.resize(8, nullptr);
    for(uint8_t i = 0; i < this->m_ring_buffer.size(); ++i)
    {
        this->m_ring_buffer[i] = av_frame_alloc();
        this->m_ring_buffer[i]->format = this->m_codec_ctx->pix_fmt;
        this->m_ring_buffer[i]->width = this->m_codec_ctx->width;
        this->m_ring_buffer[i]->height = this->m_codec_ctx->height;
        // this->m_ring_buffer[i]->time_base.num = 1;
        // this->m_ring_buffer[i]->time_base.den = 60;
        this->m_ring_buffer[i]->time_base = {1, 24000};
        av_image_alloc(this->m_ring_buffer[i]->data,
                       this->m_ring_buffer[i]->linesize,
                       this->m_codec_ctx->width,
                       this->m_codec_ctx->height,
                       this->m_codec_ctx->pix_fmt,
                       32);
    }

    m_sws_ctx = sws_getContext(this->m_ring_buffer[0]->width,
                               this->m_ring_buffer[0]->height,
                               AV_PIX_FMT_BGRA,
                               this->m_ring_buffer[0]->width,
                               this->m_ring_buffer[0]->height,
                               AV_PIX_FMT_YUV420P,
                               SWS_FAST_BILINEAR,
                               NULL, NULL, NULL);
    // std::cout << "CHECK\n";
    // uint8_t j = 0;
    // for(uint16_t i = 0; i < 500; ++i)
    // {
    //     (uint8_t)++j;
    //     std::cout << (int)j << '\n';
    // }
    // std::cout << "END_CHECK\n";
}

void X11ScreenshotMaker::_input_key_loop()
{
    //std::cout << __func__ << std::endl;
    XGrabKey(this->m_display,
            XKeysymToKeycode(this->m_display, XK_R),
            ControlMask,
            this->m_root_window,
            false,
            GrabModeAsync,
            GrabModeAsync);
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

void X11ScreenshotMaker::_start_all_threads()
{
    //std::cout << __func__ << std::endl;
    std::thread key_loop_thr(&X11ScreenshotMaker::_input_key_loop, this);
    //std::thread metronome_thr(&X11ScreenshotMaker::_metronome_loop, this);
    std::thread create_thr(&X11ScreenshotMaker::_create_image_loop, this);
    std::thread write_thr(&X11ScreenshotMaker::_write_image_loop, this);
    key_loop_thr.join();
    //metronome_thr.join();
    create_thr.join();
    write_thr.join();
}

void X11ScreenshotMaker::_finish_video()
{
    //DELAYED FRAMES(~Задержка кадров)
    // for (;;) {
    //     avcodec_send_frame(this->m_codec_ctx, NULL);
    //     if (avcodec_receive_packet(this->m_codec_ctx, this->m_packet) == 0) {
    //         av_interleaved_write_frame(this->m_format_ctx, this->m_packet);
    //         av_packet_unref(this->m_packet);
    //     }
    //     else {
    //         break;
    //     }
    // }
    //av_packet_rescale_ts(this->m_packet, {1, 60000}, this->m_stream->time_base); //this->m_stream->time_base
    //std::cout << __func__ << std::endl;
    //FINISH
    // Запись трейлера потока в выходной медиафайл и освобождение частных данных файла.
    av_write_trailer(this->m_format_ctx);
    // Если всё сброшенно, то закрывается io поток
    if (!(this->m_output_format->flags & AVFMT_NOFILE)) {
        if (avio_close(this->m_format_ctx->pb) < 0) {
            // ERROR
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

void X11ScreenshotMaker::_metronome_loop()
{
    while(this->m_program_state.load())
    {
        // this->m_metronome_state.store(false);
        // sleep(0.013);
        // this->m_metronome_state.store(true);
    }
}

void X11ScreenshotMaker::_create_image_loop()
{
    //std::cout << __func__ << std::endl;
    while(this->m_program_state.load())
    {
        //if(this->m_metronome_state.load())
        _create_image();
    }
}

void X11ScreenshotMaker::_create_image()
{
    //std::cout << __func__ << std::endl;
     this->m_create_delta_id = this->m_write_id - (this->m_create_id + 1);
     //std::cout << this->m_create_delta_id << '\n';
     //std::cout << this->m_write_delta_id << '\n';
     (this->*m_create_throttle[this->m_create_delta_id])();
    //(this->*m_create_throttle[1])();
    //while(this->m_write_status.load()){}
}

void X11ScreenshotMaker::_create_fake_func()
{
    //std::cout << __func__ << std::endl;
    //something
    //while(this->m_write_status.load()){}
}

void X11ScreenshotMaker::_create_core_func()
{
    //std::cout << this->m_stream->avg_frame_rate << '\n';
    //auto start = std::chrono::steady_clock::now();
    this->m_create_status.store(true);
    //std::cout << __func__ << std::endl;
    this->m_image = XGetImage(this->m_display,
                              this->m_root_window,
                              0,
                              0,
                              this->m_display_width,
                              this->m_display_height,
                              AllPlanes,
                              ZPixmap);
    this->m_src_img_data[0] = (uint8_t*)this->m_image->data;
    sws_scale(m_sws_ctx,
              this->m_src_img_data,
              this->m_scr_img_stride,
              0,
              this->m_ring_buffer[this->m_create_id]->height,
              this->m_ring_buffer[this->m_create_id]->data,
              this->m_ring_buffer[this->m_create_id]->linesize);
    ++this->m_create_id;
    std::cout << int(m_create_id) << std::endl;
    if(this->m_create_id == this->m_ring_buffer.size())
        this->m_create_id = 0;
    this->m_create_status.store(false);
    //auto end = std::chrono::steady_clock::now();
    //std::this_thread::sleep_for(std::chrono::milliseconds(16));

}

// void X11ScreenshotMaker::_time_loop()
// {
//     auto start = std::chrono::steady_clock::now();
//     while(this->m_program_state.load())
//     {
//         auto end = std::chrono::steady_clock::now();
//     }
// }

void X11ScreenshotMaker::_write_image_loop()
{
    //std::cout << __func__ << std::endl;
    while(this->m_program_state.load() || this->m_write_status.load())
    {
        _write_image();
    }
}

void X11ScreenshotMaker::_write_image()
{
    //std::cout << __func__ << std::endl;
     this->m_write_delta_id = this->m_create_id - (this->m_write_id + 0);
     (this->*m_write_throttle[this->m_write_delta_id])();
    //(this->*m_write_throttle[1])();
    //while(this->m_create_status.load()){}
}

void X11ScreenshotMaker::_write_fake_func()
{
    //std::cout << __func__ << std::endl;
    this->m_write_status.store(false);
    //while(this->m_create_status.load()){}

}

void X11ScreenshotMaker::_write_core_func()
{
    //std::cout << __func__ << std::endl;
    this->m_write_status.store(true);

    //this->m_video_pts /= this->m_codec_ctx->time_base.den;
    //this->m_video_pts = (1.0 / 60) * 90 * m_frame_count;
    //std::cout << this->m_video_pts << '\n';
    //this->m_video_pts += this->m_ring_buffer[this->m_write_id]->repeat_pict * (this->m_video_pts * 0,5);

    // Инициализация необязательных полей пакета(AVPacket) значениями по умолчанию
    //av_init_packet(&this->m_packet);
    this->m_packet = av_packet_alloc();
    this->m_packet->flags |= AV_PKT_FLAG_KEY; // AV_PKT_FLAG_KEY — это флаг, который указывает, что пакет содержит ключевой кадр.
    this->m_packet->pts = this->m_ring_buffer[this->m_write_id]->pts = this->m_video_pts;
    //m_ring_buffer[this->m_write_id]->pts = av_rescale_q(this->m_packet.pts, this->m_packet.time_base, m_ring_buffer[this->m_write_id]->time_base);
    //this->m_packet.dts = this->m_packet.pts;
    this->m_packet->data = NULL;
    this->m_packet->size = 0;
    this->m_packet->stream_index = m_stream->index;
    this->m_packet->duration = 1000;
    // Передаём необработанный кадр в кодировщик(кодек)
    if (avcodec_send_frame(this->m_codec_ctx, m_ring_buffer[this->m_write_id]) < 0) {
        // ERROR
    }
    // Чтение закодированных данных от энкодера
    if (avcodec_receive_packet(this->m_codec_ctx, m_packet) == 0) {
        //this->m_packet->stream_index = this->m_stream->index;
        // Запись пакета в выходной медиафайл, обеспечивая правильное чередование
        av_interleaved_write_frame(this->m_format_ctx, m_packet);
        // Освобождает пакет(возвращает значения по умполчанию)
        av_packet_unref(m_packet);
    }
    ++this->m_write_id;
     std::cout << int(m_write_id) << std::endl;
    //std::cout << int(m_write_id) << std::endl;
    //std::cout << (double)1/30 << '\n';
    if(this->m_write_id == this->m_ring_buffer.size())
        this->m_write_id = 0;
    //this->m_write_status.store(false);
    this->m_video_pts += 1000;
}

void X11ScreenshotMaker::make_screenshot()
{
    XGrabKey(this->m_display,
            XKeysymToKeycode(this->m_display, XK_Y),
            ControlMask,
            this->m_root_window,
            false,
            GrabModeAsync,
            GrabModeAsync);

    _key_waiting_loop();

    this->m_image = XGetImage(this->m_display,
                            DefaultRootWindow(this->m_display),
                            0,
                            0,
                            this->m_display_width,
                            this->m_display_height,
                            AllPlanes,
                            ZPixmap);
    //start_video();
}

inline void X11ScreenshotMaker::_key_waiting_loop()
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

char* X11ScreenshotMaker::get_screenshot_data()
{
    _check_img_ptr();
    return this->m_image->data;
}

int X11ScreenshotMaker::get_screenshot_row_bytecount()
{
    _check_img_ptr();
    return this->m_image->bytes_per_line;
}

unsigned short X11ScreenshotMaker::get_display_width()
{
    return this->m_display_width;
}

unsigned short X11ScreenshotMaker::get_display_height()
{
    return this->m_display_height;
}

void X11ScreenshotMaker::_check_img_ptr()
{
    if(this->m_image == nullptr)
    {
        PRINT_DEBUG_ERROR;
        exit(EXIT_FAILURE);
    }
}