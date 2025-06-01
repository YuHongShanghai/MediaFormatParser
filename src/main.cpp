#include "WavParser.h"
#include "M4aParser.h"
#include "Mp3Parser.h"
#include "FlvParser.h"

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
}

void test_m4a_parser() {
    std::string file_path = "/Users/yuhong/Desktop/MediaFormatParser/test_files/m4a/";
    M4aParser m4a_parser(file_path + "1718873863_sample1.m4a");
    assert(m4a_parser.parse() == 0);
}

void test_mp3_parser() {
    std::string file_path = "/Users/yuhong/Desktop/MediaFormatParser/test_files/mp3/";
    Mp3Parser mp3_parser(file_path + "jay.mp3");
    assert(mp3_parser.parse() == 0);
}

void test_flv_parser() {
    std::string file_path = "/Users/yuhong/Desktop/MediaFormatParser/test_files/flv/";
    FlvParser flv_parser(file_path + "video.flv");
    assert(flv_parser.parse() == 0);
}

int main() {
    init_easyloggingpp();
    // test_wave_parser();
    // test_m4a_parser();
    // test_mp3_parser();
    test_flv_parser();
    return 0;
}


