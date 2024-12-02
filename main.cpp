

#include <iostream>
#include <fstream>
//#include <cstdio>
//#include <string>
//#include <vector>
//#include <tuple>
//#include <utility>

#include <pthread.h>
//#include <png.h>
//#include <errno.h>
#include <chrono>
//#include <xmmintrin.h>

#include "framework/include/x11_screenshot_maker.hpp"
#include "framework/include/controller.hpp"



void speed_test(int limit)
{
    //auto prog_start = std::chrono::steady_clock::now();
    // double summ = 0;
    // for(double i = 0; i <= limit; i++)
    // {
    //     summ /= i * (double)(1.0 / i);
    // }
    
    //std::cout << "CORE SIZE = " << sizeof(Controller);
    //std::cout << "\nX11 SIZE = " << sizeof(X11ScreenshotMaker) << std::endl;

    // std::pair<Controller* , X11ScreenshotMaker*> p = {new Controller, new X11ScreenshotMaker};
    // std::pair<Controller* , X11ScreenshotMaker*> v[] = {p};
    // v[0].first->session_definition();
    // v[0].first->session_initialize(v[0].second);
    // v[0].first->session_make_screenshot();
    // v[0].first->save_screenshot_png();

    // Controller core;
    // X11ScreenshotMaker x11;
    // std::tuple<Controller, X11ScreenshotMaker> t = {core, x11};
    // _mm_prefetch(&t, _MM_HINT_NTA);
    // std::get<0>(t).session_definition();
    // std::get<0>(t).session_initialize(&std::get<1>(t));
    // std::get<0>(t).session_make_screenshot();
    // std::get<0>(t).save_screenshot_png();
    
    // struct Base
    // {
    //     Controller core;
    //     X11ScreenshotMaker x11;
    // } base;
    //  _mm_prefetch(&base, _MM_HINT_NTA);
    // base.core.session_definition();
    // base.core.session_initialize(&base.x11);
    // base.core.session_make_screenshot();
    // base.core.save_screenshot_png();


    auto start = std::chrono::steady_clock::now();

    Controller core;
    X11ScreenshotMaker x11;
     //_mm_prefetch(&core, _MM_HINT_NTA);
     //_mm_prefetch(&x11, _MM_HINT_NTA);
    core.session_definition();
    core.session_initialize(&x11);
    x11.start_video();
    //core.session_make_screenshot();
    //core.save_screenshot_png();


    auto end = std::chrono::steady_clock::now();
    //auto prog_end = std::chrono::steady_clock::now();
    //std::cout << "Core_Time:" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
    //std::cout << "Prog_Time:" << std::chrono::duration_cast<std::chrono::milliseconds>(prog_end - prog_start).count() << std::endl;
    //std::cout << "\n";
    //std::cout << sizeof(unsigned short) << std::endl;
}

int main()
{
    //FILE* fip = fopen("proc/loadavg", "r");
    //char* cpu_load;
    //fscanf(fip, "%f", cpu_load);
    //printf("CPU_LOAD:%f", cpu_load);
    //fclose(fip);
    //delete fip;
    //fip = nullptr;

    //std::ifstream ifs("proc/loadavg");
    //int cpu_load;

    //speed_test(200000);
    speed_test(0);
    //speed_test(100);

    //ifs >> cpu_load;
    //std::cout << cpu_load << std::endl;
    

    return 0;
}