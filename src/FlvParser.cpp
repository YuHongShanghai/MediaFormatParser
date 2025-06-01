//
// Created by 余泓 on 2025/5/31.
//

#include <iomanip>

#include "FlvParser.h"
#include "logger/easylogging++.h"
#include "utils.h"

std::ostream& operator << (std::ostream &out, const Header &h) {
    out << "flv header:" << std::endl;
    out << "\tsignature: " << std::string(h.signature, 3) << std::endl;
    out << "\tversion: " << int(h.version) << std::endl;
    out << "\thasAudio: " << std::bitset<1>(h.union_byte.bits.audio_flag) << std::endl;
    out << "\thasVideo: " << std::bitset<1>(h.union_byte.bits.video_flag) << std::endl;
    out << "\theaderSize: " <<h.header_size << std::endl;
    return out;
}

std::ostream& operator << (std::ostream &out, const TagHeader &h) {
    out << "flv tag header:" << std::endl;
    out << "\ttype: " << int(h.type);
    std::string type = "unknown";
    if (h.type == TYPE_AUDIO) {
        type = "audio";
    } else if (h.type == TYPE_VIDEO) {
        type = "video";
    } else if (h.type == TYPE_SCRIPT) {
        type = "script";
    }
    out << " (" + type + ")" << std::endl;
    out << "\tdataSize: " << h.data_size << std::endl;
    out << "\ttimestamp: " << h.timestamp << std::endl;
    out << "\ttimestampExtended: " << int(h.timestamp_extended) << std::endl;
    out << "\tstreamId: " << h.stream_id << std::endl;
    return out;
}

std::ostream& operator << (std::ostream &out, const ScriptTagData &d) {
    out << "script tag data:" << std::endl;
    out << "\tamf1Type: " << int(d.amf1_type) << std::endl;
    out << "\tamf1Length: " << d.amf1_len << std::endl;
    out << "\tamf1Data: " << d.amf1_data << std::endl;
    out << "\tamf2Type: " << int(d.amf2_type) << std::endl;
    out << "\tamf2Length: " << d.amf2_len << std::endl;
    for (int i = 0; i < d.amf2_len; ++i) {
        out<< "\t\t" << d.keys[i] << ": ";
        const AmfValue& amfValue = d.values[i];
        if (amfValue.type == NUMBER) {
            out << std::fixed << std::setprecision(3) << std::get<double>(amfValue.value);
        } else if (amfValue.type == BOOLEAN) {
            out << int(std::get<uint8_t>(amfValue.value));
        } else {
           out << std::get<std::string>(amfValue.value);
        }
        out << std::endl;
    }
    return out;
}

std::ostream& operator << (std::ostream &out, const AudioTagData &h) {
    out << "audio tag data:" << std::endl;
    out << "\tsoundFormat: " << int(h.byte1.bits.sound_format) << std::endl;
    out << "\tsoundRate: " << int(h.byte1.bits.sound_rate) << std::endl;
    out << "\tsoundSize: " << int(h.byte1.bits.sound_size) << std::endl;
    out << "\tsoundType: " << int(h.byte1.bits.sound_type) << std::endl;
    return out;
}

std::ostream& operator << (std::ostream &out, const VideoTagData &h) {
    out << "video tag data:" << std::endl;
    out << "\tframeType: " << int(h.byte1.bits.frame_type) << std::endl;
    out << "\tencodeType: " << int(h.byte1.bits.encode_type) << std::endl;
    return out;
}

FlvParser::FlvParser(const std::string& file_path): Parser(file_path) {

}

FlvParser::~FlvParser() {

}

int FlvParser::custom_parse()  {
    if (parse_header() < 0) {
        return -1;
    }

    if (parse_body() < 0) {
        return -2;
    }

    return 0;
}

int FlvParser::dump_info() {
    std::string file_path = get_output_path() + ".txt";
    std::ofstream file(file_path);
    if (!file.is_open()) {
        LOG(ERROR) << "open file " << file_path << " failed";
        return -1;
    }

    file << header << std::endl;
    int audio_index = 0, video_index = 0, prev_tag_size_index = 0;
    file << "previous tag size: " << previous_tag_sizes_[prev_tag_size_index++] << std::endl;
    for (auto& tag_header: tag_headers_) {
        file << tag_header;
        if (tag_header.type == TYPE_AUDIO) {
            file << audio_data_[audio_index++];
        } else if (tag_header.type == TYPE_VIDEO) {
            file << video_data_[video_index++];
        } else if (tag_header.type == TYPE_SCRIPT) {
            file << script_tag_data_;
        }
        file << "previous tag size: " << previous_tag_sizes_[prev_tag_size_index++] << std::endl;
        file << std::endl;
    }

    file.close();
    return 0;
}
int FlvParser::dump_data() {
    if (dump_h264_data() < 0) {
        return -1;
    }

    if (dump_aac_data() < 0) {
        return -2;
    }

    return 0;
}

int FlvParser::parse_header() {
    LOG(DEBUG) << __FUNCTION__;

    if (pos_ + HEADER_LEN > data_size_) {
        LOG(ERROR) << "not enough data";
        return -1;
    }

    if (memcmp(data_ + pos_, "FLV", 3) != 0) {
        LOG(ERROR) << "invalid signature";
        return -2;
    }

    memcpy(header.signature, data_ + pos_, 3);
    pos_ += 3;

    memcpy(&header.version, data_ + pos_, 1);
    pos_++;

    memcpy(&header.union_byte.raw, data_ + pos_, 1);
    pos_++;

    header.header_size = bytes_to_int4_be(data_ + pos_);
    pos_ += 4;

    LOG(INFO) << header;

    return 0;
}

int FlvParser::parse_body() {
    LOG(DEBUG) << __FUNCTION__;
    while (true) {
        if (pos_ + 3 >= data_size_) {
            LOG(DEBUG) << "not enough data to parse previous tag size";
            break;
        }

        uint32_t previous_tag_size = bytes_to_int4_be(data_ + pos_);
        pos_ += 4;

        LOG(INFO) << "previous tag size " << previous_tag_size;
        previous_tag_sizes_.push_back(previous_tag_size);

        TagHeader tag_header{};
        memcpy(&tag_header.type, data_ + pos_, 1);
        pos_++;

        tag_header.data_size = bytes_to_int3_be(data_ + pos_);
        pos_ += 3;

        if (pos_ + 7 + tag_header.data_size > data_size_) {
            break;
        }

        tag_header.timestamp = bytes_to_int3_be(data_ + pos_);
        pos_ += 3;

        memcpy(&tag_header.timestamp_extended, data_ + pos_, 1);
        pos_ ++;

        tag_header.stream_id = bytes_to_int3_be(data_ + pos_);
        pos_ += 3;

        tag_headers_.push_back(tag_header);
        LOG(INFO) << tag_header;

        if (tag_header.type == TYPE_AUDIO) {
            parse_audio_tag_data(pos_);
        } else if (tag_header.type == TYPE_VIDEO) {
            parse_video_tag_data(pos_);
        } else if (tag_header.type == TYPE_SCRIPT) {
            if (parse_script_tag_data(pos_) < 0) {
                LOG(ERROR) << "parse script data failed";
                return -1;
            }
        }
        pos_ += tag_header.data_size;
    }

    return 0;
}

int FlvParser::parse_script_tag_data(size_t pos) {
    script_tag_data_.amf1_type = int_value(data_[pos]);
    if (script_tag_data_.amf1_type != 2) {
        LOG(ERROR) << "amf1 type error. got " << script_tag_data_.amf1_type << ", expected 2";
        return -1;
    }

    pos++;

    script_tag_data_.amf1_len = bytes_to_int2_be(data_ + pos);
    pos += 2;


    for (int i = 0; i < script_tag_data_.amf1_len; ++i) {
        script_tag_data_.amf1_data.push_back(data_[pos + i]);
    }

    std::string target_data = "onMetaData";
    if (script_tag_data_.amf1_data != target_data) {
        LOG(ERROR) << "amf1 data error. got " << script_tag_data_.amf1_data << ", expected " << target_data;
        return -2;
    }

    pos += script_tag_data_.amf1_len;

    script_tag_data_.amf2_type = int_value(data_[pos]);
    if (script_tag_data_.amf2_type != 8) {
        LOG(ERROR) << "amf2 type error. got " << script_tag_data_.amf1_type << ", expected 8";
        return -1;
    }
    pos++;

    script_tag_data_.amf2_len = bytes_to_int4_be(data_ + pos);
    pos += 4;

    for (int i = 0; i < script_tag_data_.amf2_len; ++i) {
        uint16_t key_len = bytes_to_int2_be(data_ + pos);
        pos += 2;

        script_tag_data_.keys.emplace_back((char*)data_ + pos, key_len);
        pos += key_len;

        auto type = int_value(data_[pos]);
        pos ++;
        AmfValue value;
        if (type == 0) {
            // number
            value.type = NUMBER;
            value.value = bytes_to_double_be(data_ + pos);
            pos += 8;
        } else if (type == 1) {
            // boolean
            value.type = BOOLEAN;
            value.value = int_value(data_[pos]);
            pos += 1;
        } else if (type == 2) {
            // string
            uint16_t str_len = bytes_to_int2_be(data_ + pos);
            pos += 2;
            value.type = STRING;
            value.value = std::string((char*)data_ + pos, str_len);
            pos += str_len;
        } else {
            LOG(ERROR) << "amf2 array value type parse error. got " << type << ", expected 0 or 1 or 2";
            return -4;
        }
        script_tag_data_.values.push_back(value);
    }

    LOG(INFO) << script_tag_data_;
    return 0;
}

int FlvParser::parse_audio_tag_data(size_t pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    AudioTagData audio_data{};
    audio_data.byte1.raw = int_value(data_[pos]);
    pos++;
    audio_data.data = data_ + pos;
    audio_data_.push_back(audio_data);
    LOG(INFO) << audio_data;
    return 0;
}

int FlvParser::parse_video_tag_data(size_t pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    VideoTagData video_data{};
    video_data.byte1.raw = int_value(data_[pos]);
    pos++;
    video_data.data = data_ + pos;
    video_data_.push_back(video_data);
    LOG(INFO) << video_data;
    return 0;
}

int FlvParser::dump_h264_data() {
    std::string h264_file_path = get_output_path() + ".h264";
    std::ofstream h264_file(h264_file_path);
    if (!h264_file.is_open()) {
        LOG(ERROR) << "open file " << h264_file_path << " failed";
        return -1;
    }

    int index = 0;
    for (auto& tag_header: tag_headers_) {
        if (tag_header.type != TYPE_VIDEO) {
            continue;
        }

        VideoTagData& video_data = video_data_[index++];
        if (video_data.byte1.bits.encode_type == 7) {
            uint8_t start_code[4] = {0x00, 0x00, 0x00, 0x01};
            uint8_t avc_packet_type = int_value(*video_data.data);
            size_t pos = 4;
            // configuration
            if (avc_packet_type == 0) {
                pos += 4;
                // LengthSizeMinusOne
                nalu_len_size_ = (video_data.data[pos] & 0x03) + 1;
                pos++;

                uint8_t sps_num = (video_data.data[pos] & 0x1F);
                pos++;

                for (int i = 0; i < sps_num; ++i) {
                    uint16_t sps_len = bytes_to_int2_be(video_data.data + pos);
                    pos += 2;
                    h264_file.write(reinterpret_cast<char *>(start_code), 4);
                    h264_file.write(reinterpret_cast<char *>(video_data.data + pos), sps_len);
                    pos += sps_len;
                }

                uint8_t pps_num = int_value(video_data.data[pos]);
                pos++;
                for (int i = 0; i < pps_num; ++i) {
                    uint16_t pps_len = bytes_to_int2_be(video_data.data + pos);
                    pos += 2;
                    h264_file.write(reinterpret_cast<char *>(start_code), 4);
                    h264_file.write(reinterpret_cast<char *>(video_data.data + pos), pps_len);
                    pos += pps_len;
                }
            }
            // h264 data
            else if (avc_packet_type == 1) {
                uint32_t data_len = 0;
                if (nalu_len_size_ == 1) {
                    data_len = int_value(video_data.data[pos]);
                } else if (nalu_len_size_ == 2) {
                    data_len = bytes_to_int2_be(video_data.data + pos);
                } else if (nalu_len_size_ == 3) {
                    data_len = bytes_to_int3_be(video_data.data + pos);
                } else if (nalu_len_size_ == 4) {
                    data_len = bytes_to_int4_be(video_data.data + pos);
                }
                pos += nalu_len_size_;

                h264_file.write(reinterpret_cast<const char *>(&start_code), 4);
                h264_file.write(reinterpret_cast<char *>(video_data.data + pos), data_len);
            }
        }
    }
    h264_file.close();
    return 0;
}
int FlvParser::dump_aac_data() {
    std::string aac_file_path = get_output_path() + ".aac";
    std::ofstream aac_file(aac_file_path);
    if (!aac_file.is_open()) {
        LOG(ERROR) << "open file " << aac_file_path << " failed";
        return -1;
    }

    int index = 0;
    int aac_profile, sample_rate_index, channel_config;
    for (auto& tag_header: tag_headers_) {
        if (tag_header.type != TYPE_AUDIO) {
            continue;
        }

        AudioTagData& audio_data = audio_data_[index++];
        size_t pos = 0;
        // 10: AAC
        if (audio_data.byte1.bits.sound_format == 10) {
            uint8_t aac_packet_type = int_value(*audio_data.data);
            pos++;
            // configuration
            if (aac_packet_type == 0) {
                aac_profile = ((audio_data.data[pos]&0xf8)>>3) - 1;
                sample_rate_index = ((audio_data.data[pos]&0x07)<<1) | (audio_data.data[pos+1]>>7);
                pos++;
                channel_config = (audio_data.data[pos]>>3) & 0x0f;
            }
            // AAC raw
            else if (aac_packet_type == 1) {
                uint32_t data_len = tag_header.data_size - 1;
                // construct ADTS header
                uint64_t bits = 0;
                int adts_header_len = 7;

                write_u64(bits, 12, 0xFFF);
                write_u64(bits, 1, 0);
                write_u64(bits, 2, 0);
                write_u64(bits, 1, 1);
                write_u64(bits, 2, aac_profile);
                write_u64(bits, 4, sample_rate_index);
                write_u64(bits, 1, 0);
                write_u64(bits, 3, channel_config);
                write_u64(bits, 1, 0);
                write_u64(bits, 1, 0);
                write_u64(bits, 1, 0);
                write_u64(bits, 1, 0);
                write_u64(bits, 13, adts_header_len + data_len);
                write_u64(bits, 11, 0x7FF);
                write_u64(bits, 2, 0);

                // convert to big end
                unsigned char p64[8];
                p64[0] = (unsigned char)(bits >> 56);
                p64[1] = (unsigned char)(bits >> 48);
                p64[2] = (unsigned char)(bits >> 40);
                p64[3] = (unsigned char)(bits >> 32);
                p64[4] = (unsigned char)(bits >> 24);
                p64[5] = (unsigned char)(bits >> 16);
                p64[6] = (unsigned char)(bits >> 8);
                p64[7] = (unsigned char)(bits);

                // write adts header
                aac_file.write(reinterpret_cast<const char *>(p64 + 1), adts_header_len);

                // write raw aac data
                aac_file.write(reinterpret_cast<const char *>(audio_data.data + pos), data_len);
            }
        }
    }
    aac_file.close();

    return 0;
}
