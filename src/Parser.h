//
// Created by 余泓 on 2025/5/26.
//

#ifndef MEDIAFORMATPARSER_PARSER_H
#define MEDIAFORMATPARSER_PARSER_H

#include <string>

class Parser {
public:
    explicit Parser(const std::string& filePath);
    virtual ~Parser();
    int parse();

protected:
    virtual int custom_parse() = 0;
    virtual int dump_info() = 0;
    virtual int dump_data() = 0;
    int open_file();
    std::string get_output_path();

protected:
    std::string file_path_;
    unsigned char *data_ = nullptr;
    size_t data_size_ = 0;
    size_t pos_ = 0;
};


#endif //MEDIAFORMATPARSER_PARSER_H
