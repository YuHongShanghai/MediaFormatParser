//
// Created by 余泓 on 2025/5/31.
//

#ifndef MEDIAFORMATPARSER_FLVPARSER_H
#define MEDIAFORMATPARSER_FLVPARSER_H

#include <utility>
#include <vector>
#include <variant>

#include "Parser.h"

#define HEADER_LEN 9
#define TYPE_AUDIO 8
#define TYPE_VIDEO 9
#define TYPE_SCRIPT 18

union Header5thByte {
    uint8_t raw;
    struct {
        uint8_t video_flag:1;
        uint8_t reserved2:1;
        uint8_t audio_flag:1;
        uint8_t reserved1:5;
    } bits;
};

struct Header {
    char signature[3];
    uint8_t version;
    Header5thByte union_byte;
    uint32_t header_size;
};

struct TagHeader {
    uint8_t type;
    uint32_t data_size;
    uint32_t timestamp;
    uint8_t timestamp_extended;
    uint32_t stream_id;
};

enum AmfValueType {
    NUMBER,
    BOOLEAN,
    STRING
};

struct AmfValue {
    AmfValueType type = NUMBER;
    std::variant<double, uint8_t, std::string> value;
};

struct ScriptTagData {
    uint8_t amf1_type;
    uint16_t amf1_len;
    std::string amf1_data;

    uint8_t amf2_type;
    uint32_t amf2_len;
    std::vector<std::string> keys;
    std::vector<AmfValue> values;
};

union AudioData1thByte {
    uint8_t raw;
    struct {
        uint8_t sound_type:1;
        uint8_t sound_size:1;
        uint8_t sound_rate:2;
        uint8_t sound_format:4;
    } bits;
};

struct AudioTagData {
    AudioData1thByte byte1;
    unsigned char* data;
};

union VideoData1thByte {
    uint8_t raw;
    struct {
        uint8_t encode_type:4;
        uint8_t frame_type:4;
    } bits;
};

struct VideoTagData {
    VideoData1thByte byte1;
    unsigned char* data;
};

class FlvParser: public Parser {
public:
    FlvParser(const std::string& file_path);
    ~FlvParser();

private:
    int custom_parse() override;
    int dump_info() override;
    int dump_data() override;

    int parse_header();
    int parse_body();
    int parse_script_tag_data(size_t pos);
    int parse_audio_tag_data(size_t pos);
    int parse_video_tag_data(size_t pos);
    int dump_h264_data();
    int dump_aac_data();

private:
    Header header;
    std::vector<uint32_t> previous_tag_sizes_;
    std::vector<TagHeader> tag_headers_;
    ScriptTagData script_tag_data_;
    std::vector<AudioTagData> audio_data_;
    std::vector<VideoTagData> video_data_;
    int nalu_len_size_;
};


#endif //MEDIAFORMATPARSER_FLVPARSER_H
