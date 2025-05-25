#include "WavParser.h"

#include <filesystem>
#include <cassert>
#include <iostream>

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
    test_wave_parser();
    return 0;
}


