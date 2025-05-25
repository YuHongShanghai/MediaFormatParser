//
// Created by 余泓 on 2025/5/23.
//

#include "WavParser.h"
#include "utils.h"
#include "logger/easylogging++.h"

#include <fstream>

std::string get_format_str(uint16_t format) {
    std::string audio_format_str = "UNKONWN";
    switch (format) {
        case WAVE_FORMAT_PCM:
            audio_format_str = "PCM";
            break;
        case WAVE_FORMAT_IEEE_FLOAT:
            audio_format_str = "IEEE_FLOAT";
            break;
        case WAVE_FORMAT_ALAW:
            audio_format_str = "ALAW";
            break;
        case WAVE_FORMAT_MULAW:
            audio_format_str = "MULAW";
            break;
        case WAVE_FORMAT_EXTENSIBLE:
            audio_format_str = "EXTENSIBLE";
            break;
    }
    return audio_format_str;
}

std::ostream & operator << (std::ostream &out, const HeaderChunk &c) {
    out << "header chunk:" << std::endl;
    out << "\tid: " << std::string(c.id, 4) << std::endl;
    out << "\tsize: " << c.size << std::endl;
    out << "\ttype: " << std::string(c.type, 4) << std::endl;
    return out;
}

std::ostream & operator << (std::ostream &out, const FormatChunk &c) {
    std::string audio_format_str = get_format_str(c.audio_format);

    out << "format chunk:" << std::endl;
    out << "\tid: " << std::string(c.id, 4) << std::endl;
    out << "\tsize: " << c.size << std::endl;
    out << "\taudioFormat: " << c.audio_format << " (" << audio_format_str << ")" << std::endl;
    out << "\tchannels: " << c.channels << std::endl;
    out << "\tsampleRate: " << c.sample_rate << std::endl;
    out << "\tbyteRate: " << c.byte_rate << std::endl;
    out << "\tblockAlign: " << c.block_align << std::endl;
    out << "\tbitsPerSample: " << c.bits_per_sample << std::endl;
    if (c.size > 16) {
        out << "\textensionSize: " << c.extension_size << std::endl;
        if (c.extension_size > 0) {
            std::string format_str = get_format_str(bytes_to_int2_le(
                    reinterpret_cast<const unsigned char *>(c.sub_format)));
            out << "\tvalidBitsPerSample: " << c.valid_bits_per_sample << std::endl;
            out << "\tchannelMask: " << c.channel_mask << std::endl;
            out << "\tsubFormat: " << std::string(c.sub_format, 16) << " (" + format_str + ") " << std::endl;
        }
    }
    return out;
}

std::ostream & operator << (std::ostream &out, const DataChunk &c) {
    out << "data chunk:" << std::endl;
    out << "\tid: " << std::string(c.id, 4) << std::endl;
    out << "\tsize: " << c.size << std::endl;
    out << "\tpad_byte: " << static_cast<int>(c.pad_byte) << std::endl;
    return out;
}

WavParser::WavParser(const std::string& filePath): file_path_(filePath) {
    
}

WavParser::~WavParser() {
    if (data_) {
        delete[] data_;
    }

    if (header_chunk_) {
        delete header_chunk_;
    }
    
    if (format_chunk_) {
        delete format_chunk_;
    }
    
    if (fact_chunk_) {
        delete fact_chunk_;
    }
    
    if (data_chunk_) {
        if (data_chunk_->data) {
            delete[] data_chunk_->data;
        }
        delete data_chunk_;
    }
}

int WavParser::parse() {
    if (open_file() < 0) {
        return -1;
    }

    if (data_size_ < HEAD_CHUNK_SIZE) {
        LOG(ERROR) << "not enough header data";
        return -2;
    }

    if (parse_header_chunk() < 0) {
        return -3;
    }

    uint32_t valid_data_size = header_chunk_->size + 8;
    char chunk_id[4];
    while (pos_ + 4 < valid_data_size) {
        memcpy(chunk_id, data_ + pos_, 4);
        std::string chunk_id_str = std::string(chunk_id, 4);
        uint32_t chunk_size = bytes_to_int4_le(data_ + pos_ + 4) + 4 + 4;

        if (pos_ + chunk_size > valid_data_size) {
            LOG(ERROR) << "not enough chunk data";
            return -4;
        }

        LOG(DEBUG) << "get chunk id " << chunk_id_str;
        if (chunk_id_str == FMT_ID) {
            if (parse_format_chunk() < 0) {
                return -5;
            }
        } else if (chunk_id_str == FACT_ID) {
            if (parse_fact_chunk() < 0) {
                return -6;
            }
        } else if (chunk_id_str == DATA_ID) {
            if (parse_data_chunk() < 0) {
                return -7;
            } else {
                break;
            }
        }
        pos_ += chunk_size;
    }

    dump_info();
    dump_data();

    return 0;
}

int WavParser::open_file() {
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

int WavParser::parse_header_chunk() {
    LOG(DEBUG) << __FUNCTION__ ;
    
    header_chunk_ = new HeaderChunk();

    memcpy(header_chunk_->id, data_ + pos_, 4);
    if (std::string(header_chunk_->id, 4) != RIFF_ID) {
        return -1;
    }
    pos_ += 4;

    header_chunk_->size = bytes_to_int4_le(data_ + pos_);
    pos_ += 4;

    memcpy(header_chunk_->type, data_ + pos_, 4);
    pos_ += 4;

    if (std::string(header_chunk_->type, 4) != WAVE_TAG) {
        LOG(ERROR) << "parse id error. got " << std::string(header_chunk_->type, 4) << ", expected " << WAVE_TAG;
        return -2;
    }

    LOG(INFO) << *header_chunk_;
    return 0;
}

int WavParser::parse_format_chunk() {
    LOG(DEBUG) << __FUNCTION__ ;

    uint32_t pos = pos_;

    format_chunk_ = new FormatChunk;

    memcpy(format_chunk_->id, data_ + pos, 4);
    pos += 4;

    format_chunk_->size = bytes_to_int4_le(data_ + pos);
    pos += 4;

    format_chunk_->audio_format = bytes_to_int2_le(data_ + pos);
    pos += 2;

    format_chunk_->channels = bytes_to_int2_le(data_ + pos);
    pos += 2;

    format_chunk_->sample_rate = bytes_to_int4_le(data_ + pos);
    pos += 4;

    format_chunk_->byte_rate = bytes_to_int4_le(data_ + pos);
    pos += 4;

    format_chunk_->block_align = bytes_to_int2_le(data_ + pos);
    pos += 2;

    format_chunk_->bits_per_sample = bytes_to_int2_le(data_ + pos);
    pos += 2;

    if (format_chunk_->size > 16) {
        format_chunk_->extension_size = bytes_to_int2_le(data_ + pos);
        pos += 2;

        if (format_chunk_->extension_size > 0) {
            format_chunk_->valid_bits_per_sample = bytes_to_int2_le(data_ + pos);
            pos += 2;

            format_chunk_->channel_mask = bytes_to_int4_le(data_ + pos);
            pos += 4;

            memcpy(format_chunk_->sub_format, data_ + pos, 16);
            pos += 16;
        }
    }

    LOG(INFO) << *format_chunk_;
    return 0;
}

int WavParser::parse_fact_chunk() {
    LOG(DEBUG) << __FUNCTION__ ;

    uint32_t pos = pos_;

    fact_chunk_ = new FactChunk;

    memcpy(fact_chunk_->id, data_ + pos, 4);
    pos += 4;

    fact_chunk_->size = bytes_to_int4_le(data_ + pos);
    pos += 4;

    fact_chunk_->sample_length = bytes_to_int4_le(data_ + pos);
    pos += 4;

    LOG(INFO) << *fact_chunk_;
    return 0;
}

int WavParser::parse_data_chunk() {
    LOG(DEBUG) << __FUNCTION__ ;

    uint32_t pos = pos_;

    data_chunk_ = new DataChunk;

    memcpy(data_chunk_->id, data_ + pos, 4);
    pos += 4;

    if (std::string(data_chunk_->id, 4) != DATA_ID) {
        LOG(ERROR) << "parse id error. got " << std::string(data_chunk_->id, 4) << ", expected " << DATA_ID;
        return -1;
    }

    data_chunk_->size = bytes_to_int4_le(data_ + pos);
    pos += 4;

    data_chunk_->data = new(std::nothrow) unsigned char[data_chunk_->size];
    if (data_chunk_->data == nullptr) {
        LOG(ERROR) << "alloc memory failed";
        return -2;
    }

    memcpy(data_chunk_->data, data_ + pos, data_chunk_->size);
    pos += data_chunk_->size;

    if (data_chunk_->size % 2) {
        data_chunk_->pad_byte = int_value(*(data_ + pos));
        pos += 1;
    }

    LOG(INFO) << *data_chunk_;
    return 0;
}

int WavParser::dump_info() {
    std::string file_path = get_output_dir() + get_filename_without_extension(file_path_) + ".txt";
    std::ofstream file(file_path);
    if (!file.is_open()) {
        return -1;
    }

    if (header_chunk_) {
        file << *header_chunk_ << std::endl;
    }
    if (format_chunk_) {
        file << *format_chunk_ << std::endl;
    }
    if (fact_chunk_) {
        file << *fact_chunk_ << std::endl;
    }
    if (data_chunk_) {
        file << *data_chunk_ << std::endl;
    }
    file.close();
    LOG(INFO) << "file info has dumped to " << file_path;
    return 0;
}

int WavParser::dump_data() {
    if (data_chunk_->data == nullptr) {
        LOG(ERROR) << "no data to dump";
        return -1;
    }
    std::string dump_file_name = get_filename_without_extension(file_path_) + ".pcm";
    std::string file_path = get_output_dir() + dump_file_name;
    int ret = write_data(file_path, data_chunk_->data, data_chunk_->size);
    if (ret == 0) {
        LOG(INFO) << "data has dumped to " << file_path;
    }
    return ret;
}

void WavParser::print_ffplay_command() {
    std::string dump_file_name = get_filename_without_extension(file_path_) + ".pcm";
    int audio_format = format_chunk_->audio_format;
    int bit_depth = format_chunk_->bits_per_sample;

    if (audio_format == WAVE_FORMAT_EXTENSIBLE) {
        audio_format = bytes_to_int2_le(reinterpret_cast<const unsigned char *>(format_chunk_->sub_format));
        bit_depth = format_chunk_->valid_bits_per_sample;
    }
    std::string format_str;
    if (audio_format == WAVE_FORMAT_PCM) {
        if (bit_depth == 8) {
            format_str = "u8";
        } else if (bit_depth == 16) {
            format_str = "s16le";
        } else if (bit_depth == 32) {
            format_str = "s32le";
        } else if (bit_depth == 64) {
            format_str = "s64le";
        }
    } else if (audio_format == WAVE_FORMAT_IEEE_FLOAT) {
        if (bit_depth == 32) {
            format_str = "f32le";
        } else if (bit_depth == 64) {
            format_str = "f64le";
        }
    } else if (audio_format == WAVE_FORMAT_ALAW) {
        format_str = "alaw";
    } else if (audio_format == WAVE_FORMAT_MULAW) {
        format_str = "mulaw";
    }

    LOG(INFO) << "ffplay command:";
    LOG(INFO) << "ffplay -autoexit -f " << format_str << " -ar " << format_chunk_->sample_rate << " -ac "
        << format_chunk_->channels << " " << dump_file_name;

}
