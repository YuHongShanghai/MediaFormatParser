//
// Created by 余泓 on 2025/5/23.
//

// ref: https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html

#ifndef MEDIAFORMATPARSER_WAVPARSER_H
#define MEDIAFORMATPARSER_WAVPARSER_H

#include <stdint.h>
#include <string>
#include <ostream>

#define HEAD_CHUNK_SIZE 12

#define WAVE_FORMAT_PCM 0x0001
#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#define WAVE_FORMAT_ALAW 0x0006
#define WAVE_FORMAT_MULAW 0x0007
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE

#define WAVE_TAG "WAVE"

#define RIFF_ID "RIFF"
#define FMT_ID "fmt "
#define FACT_ID "fact"
#define DATA_ID "data"

struct HeaderChunk {
    char id[4];
    uint32_t size;
    char type[4];
};

struct FormatChunk {
    char id[4];
    uint32_t size;
    uint16_t audio_format;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint16_t extension_size;
    uint16_t valid_bits_per_sample;
    uint32_t channel_mask;
    char sub_format[16];
};

struct FactChunk {
    char id[4];
    uint32_t size;
    uint32_t sample_length;

    friend std::ostream & operator << (std::ostream &out, const FactChunk &c) {
        out << "fact chunk:" << std::endl;
        out << "\tid: " << std::string(c.id, 4) << std::endl;
        out << "\tsize: " << c.size << std::endl;
        out << "\tsampleLength: " << c.sample_length << std::endl;
        return out;
    }
};

struct DataChunk {
    char id[4];
    uint32_t size;
    unsigned char *data = nullptr;
    uint8_t pad_byte = 0;
};

struct AudioData {
    uint8_t *pos;       // 当前播放位置
    uint32_t length;    // 剩余数据长度
};

class WavParser {
public:
    explicit WavParser(const std::string& filePath);
    ~WavParser();
    int parse();
    void print_ffplay_command();

private:
    int open_file();
    int parse_header_chunk();
    int parse_format_chunk();
    int parse_fact_chunk();
    int parse_data_chunk();
    int dump_info();
    int dump_data();

private:
    HeaderChunk *header_chunk_ = nullptr;
    FormatChunk *format_chunk_ = nullptr;
    FactChunk *fact_chunk_ = nullptr;
    DataChunk *data_chunk_ = nullptr;
    std::string file_path_;
    unsigned char *data_ = nullptr;
    size_t data_size_ = 0;
    size_t pos_ = 0;
};


#endif //MEDIAFORMATPARSER_WAVPARSER_H
