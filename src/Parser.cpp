//
// Created by 余泓 on 2025/5/26.
//

#include "Parser.h"

#include <utility>
#include "logger/easylogging++.h"
#include "utils.h"

Parser::Parser(const std::string& filePath): file_path_(std::move(filePath)) {

}

Parser::~Parser() {
    if (data_) {
        delete[] data_;
    }
}

int Parser::parse() {
    if (open_file() < 0) {
        return -1;
    }

    if (custom_parse() < 0) {
        return -2;
    }

    dump_info();
    dump_data();
    return 0;
}

int Parser::open_file() {
    LOG(DEBUG) << __FUNCTION__;

    std::ifstream file(file_path_, std::ios::binary);
    if (!file) {
        LOG(ERROR) << "open file " << file_path_ << " failed";
        return -1;
    }

    // TODO: 大文件读取优化
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    data_ = new(std::nothrow) unsigned char[size];
    if (!data_) {
        LOG(ERROR) << "alloc memory failed";
        return -2;
    }
    if (file.read(reinterpret_cast<char *>(data_), size)) {
        LOG(DEBUG) << "read file successfully";
    } else {
        LOG(ERROR) << "read file failed";
        return -3;
    }
    data_size_ = size;
    LOG(DEBUG) << "read data size: " << data_size_;
    file.close();
    return 0;
}

std::string Parser::get_output_path() {
    return get_output_dir() + get_filename_without_extension(file_path_);
}
