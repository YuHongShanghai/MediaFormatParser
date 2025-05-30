//
// Created by 余泓 on 2025/5/27.
//

// ref: https://www.cnblogs.com/ranson7zop/p/7655474.html
//      https://www.multiweb.cz/twoinches/mp3inside.htm
//      https://id3.org/Home https://id3.org/id3v2.3.0
//      https://www.datavoyage.com/mpgscript/mpeghdr.htm

#ifndef MEDIAFORMATPARSER_MP3PARSER_H
#define MEDIAFORMATPARSER_MP3PARSER_H

#include <vector>

#include "Parser.h"

union FrameHeaderUnion {
    uint32_t raw; // 原始4字节数据

    struct {
        unsigned int sync1:8; //同步信息 1

        unsigned int error_protection:1; //CRC 校验
        unsigned int layer:2; //层
        unsigned int version:2; //版本
        unsigned int sync2:3; //同步信息 2

        unsigned int extension:1; //私有位
        unsigned int padding:1; //填充空白字
        unsigned int sample_rate_index:2; //采样率索引
        unsigned int bit_rate_index:4; //位率索引

        unsigned int emphasis:2; //强调方式
        unsigned int original:1; //原始媒体
        unsigned int copyright:1; //版权标志
        unsigned int mode_extension:2; //扩展模式,仅用于联合立体声
        unsigned int channel_mode:2; //声道模式
    } bits;
};

struct Id3v1 {
    char id[3];
    char song_name[30];
    char artist[30];
    char album[30];
    char year[4];
    char comment[30];
    uint8_t genre;
};

struct Id3v2Header {
    char id[3];
    uint8_t version[2];
    uint8_t flags;
    uint32_t size;
};

struct Id3v2ExtendedHeader {
    uint32_t header_size;
    uint16_t flags;
    uint32_t padding_size;
};

struct Id3v2FrameHeader {
    char frame_id[4];
    uint32_t size;
    uint16_t flags;
};

class Mp3Parser: public Parser {
public:
    Mp3Parser(const std::string& file_path);
    ~Mp3Parser();

private:
    int custom_parse() override;
    int dump_info() override;
    int dump_data() override;

    void parse_id3tag_v2_header();
    void parse_frame_headers();
    void parse_id3tag_v1();

private:
    size_t last_frame_pos_ = 0;
    std::vector<FrameHeaderUnion> frame_headers;
    Id3v1 id3v1;
    Id3v2Header id3v2_header;
    Id3v2ExtendedHeader id3v2_extended_header;
    bool has_extended_header_;
    std::vector<Id3v2FrameHeader> id3v2_frame_headers;
};


#endif //MEDIAFORMATPARSER_MP3PARSER_H
