#include "WavParser.h"

#include <filesystem>
#include <cassert>
#include <iostream>

#define ELPP_NO_DEFAULT_LOG_FILE
#define ELPP_THREAD_SAFE
#include "logger/easylogging++.h"

INITIALIZE_EASYLOGGINGPP

void init_easyloggingpp() {
    el::Configurations conf;
    conf.setToDefault();

    // 禁用写入日志文件
    conf.set(el::Level::Global, el::ConfigurationType::ToFile, "false");
    // 启用输出到控制台
    conf.set(el::Level::Global, el::ConfigurationType::ToStandardOutput, "true");
    // 格式化日志
    conf.set(el::Level::Global, el::ConfigurationType::Format,
             "[%datetime] [%level] %msg");
    // 日志颜色
    conf.set(el::Level::Info, el::ConfigurationType::Format,
             "\033[32m[%datetime] [%level] %msg\033[0m");  // 绿
    conf.set(el::Level::Warning, el::ConfigurationType::Format,
             "\033[33m[%datetime] [%level] %msg\033[0m");  // 黄
    conf.set(el::Level::Error, el::ConfigurationType::Format,
             "\033[31m[%datetime] [%level] %msg\033[0m");  // 红
    conf.set(el::Level::Fatal, el::ConfigurationType::Format,
             "\033[31m[%datetime] [%level] %msg\033[0m");  // 红
    el::Loggers::reconfigureAllLoggers(conf);
}


// todo addf8-GSM-GW.wav gsm数据适配

void test_wave_parser() {
    std::string normal_file_path = "/Users/yuhong/Desktop/MediaFormatParser/test_files/wav/normal/";
    WavParser wav_parser(normal_file_path + "drmapan.wav");
    assert(wav_parser.parse() == 0);
    wav_parser.print_ffplay_command();

//    std::string perverse_file_path = "/Users/yuhong/Desktop/MediaFormatParser/test_files/wav/perverse/";
//    WavParser wav_parser(perverse_file_path + "Ptjunk.wav");
//    assert(wav_parser.parse() < 0);
//    wav_parser.print_ffplay_command();

//    for (const auto& entry : std::filesystem::directory_iterator(normal_file_path)) {
//        if (entry.is_regular_file()) {
//            std::cout << "================= TEST " << entry.path().string() << " =================" << std::endl;
//            WavParser wav_parser(entry.path());
//            assert(wav_parser.parse() == 0);
//            std::cout << "================= " << entry.path().string() << " PASS =================" << std::endl;
//        }
//    }
}

int main() {
    init_easyloggingpp();
    test_wave_parser();
    return 0;
}


