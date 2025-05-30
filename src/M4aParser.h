//
// Created by 余泓 on 2025/5/26.
//

// ref: https://developer.apple.com/documentation/quicktime-file-format

#ifndef MEDIAFORMATPARSER_M4APARSER_H
#define MEDIAFORMATPARSER_M4APARSER_H

#include <string>
#include <cstdint>
#include <vector>

#include "Parser.h"

#define TYPE_FTYP "ftyp"
#define TYPE_FREE "free"
#define TYPE_SKIP "skip"
#define TYPE_MDAT "mdat"
#define TYPE_MOOV "moov"
#define TYPE_PRFL "prfl"
#define TYPE_MVHD "mvhd"
#define TYPE_CLIP "clip"
#define TYPE_TRAK "trak"
#define TYPE_UDTA "udta"
#define TYPE_CTAB "ctab"
#define TYPE_CMOV "cmov"
#define TYPE_RMRA "rmra"


struct Atom {
    uint32_t size;
    char type[4];
    uint64_t extended_size = 0;
    std::vector<Atom*> children;
};

struct FtypAtom: Atom {
    char major_brand[4];
    uint8_t minor_version[4];
    std::vector<char*> compatible_brands;
    ~FtypAtom() {
        for (char* c: compatible_brands) {
            delete[] c;
        }
    }
};

struct FreeAtom: Atom {

};

struct SkipAtom: Atom {

};

struct MdatAtom: Atom {
    unsigned char* data = nullptr;
};

struct MvhdAtom: Atom {
    uint8_t version;
    uint8_t flag[3];
    uint32_t create_time;
    uint32_t modification_time;
    uint32_t time_scale;
    uint32_t duration;
    float preferred_rate;
    float preferred_volume;
    uint32_t matrix_structure[9];
    uint32_t preview_time;
    uint32_t preview_duration;
    uint32_t poster_time;
    uint32_t selection_time;
    uint32_t selection_duration;
    uint32_t current_duration;
    uint32_t next_track_id;
};

class M4aParser: public Parser {
public:
    explicit M4aParser(const std::string& filePath);
    ~M4aParser();

private:
    int custom_parse() override;
    int dump_info() override;
    int dump_data() override;
    int parse_ftyp(uint32_t size, char* type, size_t data_pos);
    int parse_free(uint32_t size, char* type, size_t data_pos);
    int parse_skip(uint32_t size, char* type, size_t data_pos);
    int parse_mdat(uint32_t size, char* type, uint64_t extended_size, size_t data_pos);

    int parse_moov(uint32_t size, char* type, size_t data_pos);

private:
    std::vector<Atom*> atoms;
};


#endif //MEDIAFORMATPARSER_M4APARSER_H
