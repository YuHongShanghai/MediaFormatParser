//
// Created by 余泓 on 2025/5/27.
//

#include "Mp3Parser.h"
#include "logger/easylogging++.h"
#include "utils.h"
#include <string>
#include <mpg123.h>

std::ostream& operator << (std::ostream &out, const FrameHeaderUnion &c) {
    out << "frame header:" << std::endl;
    out << "\tsync: " << std::bitset<8>(c.bits.sync1) << std::bitset<3>(c.bits.sync2) << std::endl;
    out << "\tversion: " << std::bitset<2>(c.bits.version) << std::endl;
    out << "\tlayer: " << std::bitset<2>(c.bits.layer) << std::endl;
    out << "\tcrc: " << std::bitset<1>(c.bits.error_protection) << std::endl;
    out << "\tbitRateIndex: " << std::bitset<4>(c.bits.bit_rate_index) << std::endl;
    out << "\tsampleRateIndex: " << std::bitset<2>(c.bits.sample_rate_index) << std::endl;
    out << "\tpadding: " << std::bitset<1>(c.bits.padding) << std::endl;
    out << "\textension: " << std::bitset<1>(c.bits.extension) << std::endl;
    out << "\tchannelMode: " << std::bitset<2>(c.bits.channel_mode) << std::endl;
    out << "\tmodeExtension: " << std::bitset<2>(c.bits.mode_extension) << std::endl;
    out << "\tcopyright: " << std::bitset<1>(c.bits.copyright) << std::endl;
    out << "\toriginal: " << std::bitset<1>(c.bits.original) << std::endl;
    out << "\temphasis: " << std::bitset<2>(c.bits.emphasis) << std::endl;
    return out;
}

std::ostream& operator << (std::ostream &out, const Id3v1 &c) {
    out << "Id3TagV1:" << std::endl;
    out << "\tId: " << std::string(c.id, 3) << std::endl;
    out << "\tsongName: " << std::string(c.song_name, 30) << std::endl;
    out << "\tartist: " << std::string(c.artist, 30) << std::endl;
    out << "\talbum: " << std::string(c.album, 30) << std::endl;
    out << "\tyear: " << std::string(c.year, 4) << std::endl;
    out << "\tcomment: " << std::string(c.comment, 30) << std::endl;
    out << "\tgenre: " << static_cast<int>(c.genre) << std::endl;
    return out;
}

std::ostream& operator << (std::ostream &out, const Id3v2Header &c) {
    out << "Id3TagV2 header:" << std::endl;
    out << "\tid: " << std::string(c.id, 3) << std::endl;
    out << "\tversion: " << int(c.version[0]) << " " << int(c.version[1]) << std::endl;
    out << "\tflags: " << std::bitset<8>(c.flags) << std::endl;
    out << "\tsize: " << c.size << std::endl;
    return out;
}

std::ostream& operator << (std::ostream &out, const Id3v2ExtendedHeader &c) {
    out << "Id3TagV2 extended header:" << std::endl;
    out << "\textendedHeaderSize: " << c.header_size << std::endl;
    out << "\textendedFlags: " << c.flags << std::endl;
    out << "\tpaddingSize: " << c.padding_size << std::endl;
    return out;
}

std::ostream& operator << (std::ostream &out, const Id3v2FrameHeader &c) {
    out << "Id3TagV2 frame header:" << std::endl;
    out << "\tframeId: " << std::string(c.frame_id, 4) << std::endl;
    out << "\tsize: " << c.size << std::endl;
    out << "\tflags: " << c.flags << std::endl;
    return out;
}

Mp3Parser::Mp3Parser(const std::string& file_path): Parser(file_path) {

}

Mp3Parser::~Mp3Parser() {

}

void Mp3Parser::parse_id3tag_v2_header() {
    while (pos_ + 2 < data_size_) {
        if (data_[pos_] == 'I' && data_[pos_ + 1] == 'D' && data_[pos_ + 2] == '3') {
            LOG(DEBUG) << "got tag v2";
            memcpy(&id3v2_header.id, data_ + pos_, 3);
            pos_ += 3;
            memcpy(id3v2_header.version, data_ + pos_, 2);
            pos_ += 2;
            if (id3v2_header.version[0] != 3) {
                LOG(WARNING) << "not tag v2 version 3, got " << int(id3v2_header.version[0]);
                break;
            }

            memcpy(&id3v2_header.flags, data_ + pos_, 1);
            has_extended_header_ = ((id3v2_header.flags && 0b01000000) >> 6) == 1;
            pos_++;

            id3v2_header.size =
            (data_[pos_]&0x7F)*0x200000
            + (data_[pos_+1]&0x7F)*0x4000
            + (data_[pos_+2]&0x7F)*0x80
            + (data_[pos_+3]&0x7F);
            pos_ += 4;

            if (has_extended_header_) {
                id3v2_extended_header.header_size = bytes_to_int4_be(data_ + pos_);
                pos_ += 4;

                memcpy(&id3v2_extended_header.flags, data_ + pos_, 2);
                pos_ += 2;

                id3v2_extended_header.padding_size = bytes_to_int4_be(data_ + pos_);
                pos_ += 4;

                pos_ += id3v2_extended_header.header_size;
                LOG(INFO) << id3v2_extended_header;
            }
            LOG(INFO) << id3v2_header;

            size_t start_pos = pos_;
            while (pos_ - start_pos < id3v2_header.size) {
                Id3v2FrameHeader header{};
                memcpy(header.frame_id, data_ + pos_, 4);
                bool valid_frame_id_ch = false;
                for (int i =0; i< 4; ++i) {
                    valid_frame_id_ch = header.frame_id[i] >= 'A' && header.frame_id[i] <= 'Z';
                    if (i > 0) {
                        valid_frame_id_ch = valid_frame_id_ch ||
                                isdigit(header.frame_id[i]);
                    }
                    if (!valid_frame_id_ch) {
                        break;
                    }
                }
                if (!valid_frame_id_ch) {
                    break;
                }
                pos_ += 4;
                header.size = bytes_to_int4_be(data_ + pos_);
                pos_ += 4;
                memcpy(&header.flags, data_ + pos_, 2);
                pos_ += 2;
                LOG(INFO) << header;

//                std::string str(reinterpret_cast<const char*>(data_ + pos_), header.size);
//                LOG(INFO) << str;

                pos_ += header.size;
                id3v2_frame_headers.push_back(header);
            }
            pos_ = start_pos + id3v2_header.size;
            break;
        } else {
            pos_ ++;
        }
    }
}

void Mp3Parser::parse_frame_headers() {
    while (pos_ + 1 < data_size_) {
        if (data_[pos_] == 0xff && ((data_[pos_ + 1] & 0xe0) == 0xe0)) {
            // LOG(DEBUG) << "got a frame header at " << pos_;
            FrameHeaderUnion frame_header{};
            memcpy(&frame_header.raw, data_ + pos_, 4);
            // LOG(INFO) << frame_header;
            frame_headers.push_back(frame_header);
            last_frame_pos_ = pos_;
            pos_ += 4;
        } else {
            pos_++;
        }
    }
}

void Mp3Parser::parse_id3tag_v1() {
    size_t pos = last_frame_pos_;
    while (pos + 2 < data_size_) {
        if (data_[pos] == 'T' && data_[pos + 1] == 'A' && data_[pos + 2] == 'G') {
            LOG(DEBUG) << "got tag v1";
            memcpy(&id3v1.id, data_ + pos, 3);
            pos += 3;
            memcpy(&id3v1.song_name, data_ + pos, 30);
            pos += 30;
            memcpy(&id3v1.artist, data_ + pos, 30);
            pos += 30;
            memcpy(&id3v1.album, data_ + pos, 30);
            pos += 30;
            memcpy(&id3v1.year, data_ + pos, 4);
            pos += 4;
            memcpy(&id3v1.comment, data_ + pos, 30);
            pos += 30;
            memcpy(&id3v1.genre, data_ + pos, 1);
            LOG(INFO) << id3v1;
            break;
        } else {
            pos += 1;
        }
    }
}

int Mp3Parser::custom_parse() {
    parse_id3tag_v2_header();
    parse_frame_headers();
    parse_id3tag_v1();

    return 0;
}

int Mp3Parser::dump_info() {
    std::string file_path = get_output_path() + ".txt";
    std::ofstream file(file_path);
    if (!file.is_open()) {
        return -1;
    }

    for (auto header: id3v2_frame_headers) {
        file << header;
    }
    file << std::endl;

    file << id3v1 << std::endl;

    if (frame_headers.empty()) {
        return 0;
    }

    int start_index = 1;
    for (int i = 1; i < frame_headers.size(); ++i) {
        if (frame_headers[i].raw != frame_headers[i-1].raw) {
            if (start_index == i) {
                file << "[" << start_index << "] " << frame_headers[i-1];
            } else {
                file << "[" << start_index << " - " << i << "] " << frame_headers[i-1];
            }
            start_index = i + 1;
        }
    }

    file.close();
    return 0;
}

int Mp3Parser::dump_data() {
    std::string output_file_path = get_output_path() + ".pcm";

    mpg123_init();
    mpg123_handle *mh = mpg123_new(NULL, NULL);
    mpg123_open(mh, file_path_.c_str());

    long rate;
    int channels, encoding;
    if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK) {
        LOG(ERROR) << "Failed to get audio format!" << std::endl;
        return -1;
    }

    LOG(INFO) << "Sample rate: " << rate << ", Channels: " << channels << ", encoding: " << encoding << std::endl;

    const size_t buffer_size = 8192;
    unsigned char buffer[buffer_size];
    size_t done;

    std::ofstream out(output_file_path.c_str(), std::ios::binary);
    while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK) {
        out.write(reinterpret_cast<char *>(buffer), done);
    }

    out.close();
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();

    return 0;
}