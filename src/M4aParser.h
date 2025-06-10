//
// Created by 余泓 on 2025/5/26.
//

// ref: https://developer.apple.com/documentation/quicktime-file-format

#ifndef MEDIAFORMATPARSER_M4APARSER_H
#define MEDIAFORMATPARSER_M4APARSER_H

#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <functional>
#include <arm_neon.h>
#include <ostream>
#include <iomanip>

#include "Parser.h"

#define TYPE_FTYP "ftyp"
#define TYPE_FREE "free"
#define TYPE_SKIP "skip"
#define TYPE_WIDE "wide"
#define TYPE_PNOT "pnot"
#define TYPE_MDAT "mdat"
#define TYPE_MVHD "mvhd"
#define TYPE_CTAB "ctab"
#define TYPE_TKHD "tkhd"
#define TYPE_TXAS "txas"
#define TYPE_CLEF "clef"
#define TYPE_PROF "prof"
#define TYPE_ENOF "enof"
#define TYPE_ELST "elst"
#define TYPE_LOAD "load"
#define TYPE_MDHD "mdhd"
#define TYPE_ELNG "elng"
#define TYPE_HDLR "hdlr"
#define TYPE_VMHD "vmhd"
#define TYPE_SMHD "smhd"
#define TYPE_GMIN "gmin"
#define TYPE_DREF "dref"
#define TYPE_STSD "stsd"
#define TYPE_STTS "stts"
#define TYPE_CTTS "ctts"
#define TYPE_CSLG "cslg"
#define TYPE_STSS "stss"
#define TYPE_STPS "stps"
#define TYPE_STSC "stsc"
#define TYPE_STSZ "stsz"
#define TYPE_STCO "stco"
#define TYPE_SDTP "sdtp"

struct Atom {
    uint32_t size;
    char type[4];
    uint64_t extended_size = 0;
    std::vector<Atom*> children;

    virtual void print(std::ostream &out) const {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\ttype: " << std::string(type, 4) << std::endl;
        out << "\textendedSize" << extended_size << std::endl;
    }

    virtual ~Atom() {
        for (auto *c: children) {
            delete c;
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Atom& a);
};

struct FtypAtom: Atom {
    char major_brand[4];
    uint8_t minor_version[4];
    std::vector<char*> compatible_brands;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\ttype: " << std::string(type, 4) << std::endl;
        out << "\tmajorBrand: " << std::string(major_brand, 4) << std::endl;
        out << "\tminorVersion: " << static_cast<int>(minor_version[0]) << " "
            << static_cast<int>(minor_version[1]) << " "
            << static_cast<int>(minor_version[2]) << " "
            << static_cast<int>(minor_version[3]) << std::endl;
        out << "\tcompatibleBrands: ";
        for (char* c: compatible_brands) {
            out << std::string(c, 4) << " ";
        }
        out << std::endl;
    }

    FtypAtom() {
        memcpy(type, TYPE_FTYP, 4);
    }
    ~FtypAtom() {
        for (char* c: compatible_brands) {
            delete[] c;
        }
    }
};

struct FreeAtom: Atom {
    uint32_t free_space;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\ttype: " << std::string(type, 4) << std::endl;
        out << "\tfreeSpace:" << free_space << std::endl;
    }

    FreeAtom() {
        memcpy(type, TYPE_FREE, 4);
    }
};

struct SkipAtom: Atom {
    uint32_t free_space;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\ttype: " << std::string(type, 4) << std::endl;
        out << "\tfreeSpace:" << free_space << std::endl;
    }

    SkipAtom() {
        memcpy(type, TYPE_SKIP, 4);
    }
};

struct WideAtom: Atom {
    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\ttype: " << std::string(type, 4) << std::endl;
    }

    WideAtom() {
        memcpy(type, TYPE_WIDE, 4);
    }
};

struct MdatAtom: Atom {
    unsigned char* data = nullptr;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\ttype: " << std::string(type, 4) << std::endl;
        out << "\textendedSize: " << extended_size << std::endl;
        size_t data_size = extended_size == 0 ? size - 8: extended_size - 16;
        out << "\tdata:" << "(" << data_size << " bytes)" << std::endl;
    }

    MdatAtom() {
        memcpy(type, TYPE_MDAT, 4);
    }
};

struct PnotAtom: Atom {
    uint32_t modification_date;
    uint16_t version_number;
    uint32_t atom_type;
    uint32_t atom_index;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\ttype: " << std::string(type, 4) << std::endl;
        out << "\tmodificationDate: " << modification_date << std::endl;
        out << "\tversionNumber: " << version_number << std::endl;
        out << "\tatomType: " << atom_type << std::endl;
        out << "\tatomIndex: " << atom_index << std::endl;
    }

    PnotAtom() {
        memcpy(type, TYPE_PNOT, 4);
    }
};

struct MvhdAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    uint32_t creation_time;
    uint32_t modification_time;
    uint32_t time_scale;
    uint32_t duration;
    float preferred_rate;
    float preferred_volume;
    int32_t matrix_structure[9];
    uint32_t preview_time;
    uint32_t preview_duration;
    uint32_t poster_time;
    uint32_t selection_time;
    uint32_t selection_duration;
    uint32_t current_time;
    uint32_t next_track_id;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\ttype: " << std::string(type, 4) << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tcreationTime: " << creation_time << std::endl;
        out << "\tmodificationTime: " << modification_time << std::endl;
        out << "\ttimeScale: " << time_scale << std::endl;
        out << "\tduration: " << duration << std::endl;
        out << "\tpreferredRate: " << std::fixed << std::setprecision(3) << preferred_rate << std::endl;
        out << "\tpreferredVolume: " << std::fixed << std::setprecision(3) << preferred_volume << std::endl;
        out << "\tmatrixStructure: ";
        for (auto &i : matrix_structure) {
            out << i << " ";
        }
        out << std::endl;
        out << "\tpreviewTime: " << preview_time << std::endl;
        out << "\tpreviewDuration: " << preview_duration << std::endl;
        out << "\tposterTime: " << poster_time << std::endl;
        out << "\tselectionTime: " << selection_time << std::endl;
        out << "\tselectionDuration: " << selection_duration << std::endl;
        out << "\tcurrentTime: " << current_time << std::endl;
        out << "\tnextTrackId: " << next_track_id << std::endl;
    }

    MvhdAtom() {
        memcpy(type, TYPE_MVHD, 4);
    }
};

struct Color {
    uint16_t first;
    uint16_t red;
    uint16_t green;
    uint16_t blue;
};

struct CtabAtom: Atom {
    uint32_t color_table_seed;
    uint32_t color_table_flags;
    uint32_t color_table_size;
    std::vector<Color> color_array;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\ttype: " << std::string(type, 4) << std::endl;
        out << "\tcolorTableSeed: " << color_table_seed << std::endl;
        out << "\tcolorTableFlags: " << color_table_flags << std::endl;
        out << "\tcolorTableSize: " << color_table_size << std::endl;
        out << "\tcolorArray: " ;
        for (auto& color: color_array) {
            out << color.first << ", " << color.red << ", " << color.green << ", " << color.blue;
            out << " ";
        }
        out << std::endl;
    }

    CtabAtom() {
        memcpy(type, TYPE_CTAB, 4);
    }
};

struct TkhdAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    uint32_t creation_time;
    uint32_t modification_time;
    uint32_t track_id;
    uint32_t reserved1;
    uint32_t duration;
    uint64_t reserved2;
    uint16_t layer;
    uint16_t alternate_group;
    float volume;
    uint16_t reserved3;
    int32_t matrix_structure[9];
    float track_width;
    float track_height;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\ttype: " << std::string(type, 4) << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tcreationTime: " << creation_time << std::endl;
        out << "\tmodificationTime: " << modification_time << std::endl;
        out << "\ttrackId: " << track_id << std::endl;
        out << "\tduration: " << duration << std::endl;
        out << "\tlayer: " << layer << std::endl;
        out << "\talternateGroup: " << alternate_group << std::endl;
        out << "\tvolume: " << std::fixed << std::setprecision(3) << volume << std::endl;
        out << "\tmatrixStructure: ";
        for (auto &i : matrix_structure) {
            out << i << " ";
        }
        out << std::endl;
        out << "\ttrackWidth: " << std::fixed << std::setprecision(3) << track_width << std::endl;
        out << "\ttrackHeight: " << std::fixed << std::setprecision(3) << track_height << std::endl;
    }

    TkhdAtom() {
        memcpy(type, TYPE_TKHD, 4);
    }
};

struct TxasAtom: Atom {
    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\ttype: " << std::string(type, 4) << std::endl;
    }

    TxasAtom() {
        memcpy(type, TYPE_TXAS, 4);
    }
};

struct ClefAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    float width;
    float height;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\ttype: " << std::string(type, 4) << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\twidth: " << std::fixed << std::setprecision(3) << width << std::endl;
        out << "\theight: " << std::fixed << std::setprecision(3) << height << std::endl;
    }

    ClefAtom() {
        memcpy(type, TYPE_CLEF, 4);
    }
};

struct ProfAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    float width;
    float height;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\ttype: " << std::string(type, 4) << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\twidth: " << std::fixed << std::setprecision(3) << width << std::endl;
        out << "\theight: " << std::fixed << std::setprecision(3) << height << std::endl;
    }

    ProfAtom() {
        memcpy(type, TYPE_PROF, 4);
    }
};

struct EnofAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    float width;
    float height;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\ttype: " << std::string(type, 4) << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\twidth: " << std::fixed << std::setprecision(3) << width << std::endl;
        out << "\theight: " << std::fixed << std::setprecision(3) << height << std::endl;
    }

    EnofAtom() {
        memcpy(type, TYPE_ENOF, 4);
    }
};

struct ElstEntry {
    uint32_t track_duration;
    uint32_t media_time;
    float media_rate;
};

struct ElstAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    uint32_t entry_num;
    std::vector<ElstEntry> edit_list_table;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\ttype: " << std::string(type, 4) << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tnumberOfEntries: " << entry_num << std::endl;
        out << "\teditListTable: \n";
        for (auto &e : edit_list_table) {
            out << "\t  trackDuration: " << e.track_duration << std::endl;
            out << "\t  mediaTime: " << e.media_time << std::endl;
            out << "\t  mediaRate: " << std::fixed << std::setprecision(3) << e.media_rate << std::endl;
        }
        out << std::endl;
    }

    ElstAtom() {
        memcpy(type, TYPE_ELST, 4);
    }
};

struct LoadAtom: Atom {
    uint32_t preload_start_time;
    uint32_t preload_duration;
    uint32_t preload_flags;
    uint32_t default_hints;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tpreloadStarTime: " << preload_start_time << std::endl;
        out << "\tpreloadDuration: " << preload_duration << std::endl;
        out << "\tpreloadFlags: " << preload_flags << std::endl;
        out << "\tdefaultHints: " << default_hints << std::endl;
    }

    LoadAtom() {
        memcpy(type, TYPE_LOAD, 4);
    }
};

struct MdhdAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    uint32_t creation_time;
    uint32_t modification_time;
    uint32_t time_scale;
    uint32_t duration;
    uint16_t language;
    uint16_t quality;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tcreationTime: " << creation_time << std::endl;
        out << "\tmodificationTime: " << modification_time << std::endl;
        out << "\ttimeScale: " << time_scale << std::endl;
        out << "\tduration: " << duration << std::endl;
        out << "\tlanguage: " << language << std::endl;
        out << "\tquality: " << quality << std::endl;
    }

    MdhdAtom() {
        memcpy(type, TYPE_MDHD, 4);
    }
};

struct ElngAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    std::string language_tag_string;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tlanguageTagString" << language_tag_string << std::endl;
    }

    ElngAtom() {
        memcpy(type, TYPE_ELNG, 4);
    }
};

struct HdlrAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    char component_type[4];
    char component_subtype[4];
    uint32_t component_manufacturer;
    uint32_t component_flags;
    uint32_t component_flags_mask;
    std::string component_name;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tcomponentType: " << std::string(component_type, 4) << std::endl;
        out << "\tcomponentSubtype: " << std::string(component_subtype, 4) << std::endl;
        out << "\tcomponentManufacturer: (Reserved)" << std::endl;
        out << "\tcomponentFlags: (Reserved)" << std::endl;
        out << "\tcomponentFlagsMask: (Reserved)" << std::endl;
        out << "\tcomponentName: " << component_name << std::endl;
    }

    HdlrAtom() {
        memcpy(type, TYPE_HDLR, 4);
    }
};

struct VmhdAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    uint16_t graphics_mode;
    uint16_t opcolor_red;
    uint16_t opcolor_green;
    uint16_t opcolor_blue;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tgraphicsMode: " << graphics_mode << std::endl;
        out << "\topcolor: (" << opcolor_red << " " << opcolor_green << " " << opcolor_blue << ")" << std::endl;
    }

    VmhdAtom() {
        memcpy(type, TYPE_VMHD, 4);
    }
};

struct SmhdAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    uint16_t balance;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tbalance: " << balance << std::endl;
    }

    SmhdAtom() {
        memcpy(type, TYPE_SMHD, 4);
    }
};

struct GminAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    uint16_t graphics_mode;
    uint16_t opcolor_red;
    uint16_t opcolor_green;
    uint16_t opcolor_blue;
    uint16_t balance;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tgraphicsMode: " << graphics_mode << std::endl;
        out << "\topcolor: (" << opcolor_red << " " << opcolor_green << " " << opcolor_blue << ")" << std::endl;
        out << "\tbalance: " << balance << std::endl;
    }

    GminAtom() {
        memcpy(type, TYPE_GMIN, 4);
    }
};

struct DrefAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    uint32_t entry_num;
    // todo: parse child atom

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tnumberOfEntries: " << entry_num << std::endl;
    }

    DrefAtom() {
        memcpy(type, TYPE_DREF, 4);
    }
};

struct MediaDataAtom {
    uint32_t sample_description_size;
    char data_format[4];
    // reserved: 6
    uint16_t data_reference_index;
};

struct StsdAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    uint32_t entry_num;
    // todo: parse child atom
    std::vector<MediaDataAtom*> sample_description_table;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tnumberOfEntries: " << entry_num << std::endl;
        int i = 0;
        for (auto a: sample_description_table) {
            out << "\tmediaDataAtom [" << ++i << "]:" << std::endl;
            out << "\t\tsampleDescriptionSize: " << a->sample_description_size << std::endl;
            out << "\t\tdataFormat: " << std::string(a->data_format, 4) << std::endl;
            out << "\t\tdataReferenceIndex: " << a->data_reference_index << std::endl;
        }
    }

    ~StsdAtom() {
        for (auto a: sample_description_table) {
            delete a;
        }
    }

    StsdAtom() {
        memcpy(type, TYPE_STSD, 4);
    }
};

struct SttsEntry {
    uint32_t sample_count;
    uint32_t sample_duration;
};

struct SttsAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    uint32_t entry_num;
    std::vector<SttsEntry> time_to_sample_table;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tnumberOfEntries: " << entry_num << std::endl;
        out << "\ttimeToSampleTable: " << std::endl;
        int i = 0;
        for (auto a: time_to_sample_table) {
            out << "\t[" << ++i << "]:" << std::endl;
            out << "\t\tsampleCount: " << a.sample_count << std::endl;
            out << "\t\tsampleDuration: " << a.sample_duration << std::endl;
        }
    }

    SttsAtom() {
        memcpy(type, TYPE_STTS, 4);
    }
};

struct CttsEntry {
    uint32_t sample_count;
    uint32_t composition_offset;
};

struct CttsAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    uint32_t entry_count;
    std::vector<CttsEntry> composition_offset_table;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tentryCount: " << entry_count << std::endl;
        out << "\tcompositionOffsetTable: " << std::endl;
        int i = 0;
        for (auto a: composition_offset_table) {
            out << "\t[" << ++i << "]:" << std::endl;
            out << "\t\tsampleCount: " << a.sample_count << std::endl;
            out << "\t\tcompositionOffset: " << a.composition_offset << std::endl;
        }
    }

    CttsAtom() {
        memcpy(type, TYPE_CTTS, 4);
    }
};

struct CslgAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    uint32_t composition_offset_to_display_offset_shift;
    uint32_t least_display_offset;
    uint32_t greatest_display_offset;
    uint32_t display_start_time;
    uint32_t display_end_time;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tcompositionOffsetToDisplayOffsetShift: " << composition_offset_to_display_offset_shift << std::endl;
        out << "\tleastDisplayOffset: " << least_display_offset << std::endl;
        out << "\tgreatestDisplayOffset: " << greatest_display_offset << std::endl;
        out << "\tdisplayStartTime: " << display_start_time << std::endl;
        out << "\tdisplayEndTime: " << display_end_time << std::endl;
    }

    CslgAtom() {
        memcpy(type, TYPE_CSLG, 4);
    }
};

struct StssAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    uint32_t entry_num;
    std::vector<uint32_t> sample_numbers;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tnumberOfEntries: " << entry_num << std::endl;
        out << "\tsampleNumbers: " << std::endl;
        uint32_t i = 0;
        for (auto a: sample_numbers) {
            if (i++ % 10 == 0) {
                out << "\t\t";
            }
            out << a << " ";
            if (i % 10 == 0) {
                out << std::endl;
            }
        }
    }

    StssAtom() {
        memcpy(type, TYPE_STSS, 4);
    }
};

struct StpsAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    uint32_t entry_num;
    std::vector<uint32_t> sample_numbers;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tnumberOfEntries: " << entry_num << std::endl;
        out << "\tsampleNumbers: " << std::endl;
        uint32_t i = 0;
        for (auto a: sample_numbers) {
            if (i++ % 10 == 0) {
                out << "\t\t";
            }
            out << a << " ";
            if (i % 10 == 0) {
                out << std::endl;
            }
        }
    }

    StpsAtom() {
        memcpy(type, TYPE_STSS, 4);
    }
};

struct StscEntry {
    uint32_t first_chunk;
    uint32_t sample_per_chunk;
    uint32_t sample_description_id;
};

struct StscAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    uint32_t entry_num;
    std::vector<StscEntry> sample_to_chunk_table;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tnumberOfEntries: " << entry_num << std::endl;
        out << "\tsampleToChunkTable: " << std::endl;
        int i = 0;
        for (auto a: sample_to_chunk_table) {
            out << "\t[" << ++i << "]:" << std::endl;
            out << "\t\tfirstChunk: " << a.first_chunk << std::endl;
            out << "\t\tsamplePerChunk: " << a.sample_per_chunk << std::endl;
            out << "\t\tsampleDescriptionId: " << a.sample_description_id << std::endl;
        }
    }

    StscAtom() {
        memcpy(type, TYPE_STSC, 4);
    }
};

struct StszAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    uint32_t sample_size;
    uint32_t entry_num;
    std::vector<uint32_t> sample_size_table;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tsampleSize: " << sample_size << std::endl;
        out << "\tnumberOfEntries: " << entry_num << std::endl;
        out << "\tsampleSizeTable: " << std::endl;
        uint32_t i = 0;
        for (auto a: sample_size_table) {
            if (i++ % 10 == 0) {
                out << "\t\t";
            }
            out << a << " ";
            if (i % 10 == 0) {
                out << std::endl;
            }
        }
    }

    StszAtom() {
        memcpy(type, TYPE_STSZ, 4);
    }
};

struct StcoAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    uint32_t entry_num;
    std::vector<uint32_t> chunk_offset_table;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tnumberOfEntries: " << entry_num << std::endl;
        out << "\tchunkOffsetTable: " << std::endl;
        uint32_t i = 0;
        for (auto a: chunk_offset_table) {
            if (i++ % 10 == 0) {
                out << "\t\t";
            }
            out << a << " ";
            if (i % 10 == 0) {
                out << std::endl;
            }
        }
    }

    StcoAtom() {
        memcpy(type, TYPE_STCO, 4);
    }
};

struct SdtpAtom: Atom {
    uint8_t version;
    uint8_t flags[3];
    std::vector<uint8_t> sample_dependency_flags_table;

    void print(std::ostream &out) const override {
        out << std::string(type, 4) << ":" << std::endl;
        out << "\tsize: " << size << std::endl;
        out << "\tversion: " << int(version) << std::endl;
        out << "\tflags: " << int(flags[0]) << int(flags[1]) << int(flags[2]) << std::endl;
        out << "\tsampleDependencyFlagsTable: " << std::endl;
        uint32_t i = 0;
        for (auto a: sample_dependency_flags_table) {
            if (i++ % 10 == 0) {
                out << "\t\t";
            }
            out << int(a) << " ";
            if (i % 10 == 0) {
                out << std::endl;
            }
        }
    }

    SdtpAtom() {
        memcpy(type, TYPE_SDTP, 4);
    }
};

class M4aParser: public Parser {
public:
    explicit M4aParser(const std::string& filePath);
    ~M4aParser();

private:
    int custom_parse() override;
    int dump_info() override;
    int dump_data() override;

    void register_parse_functions();
    uint64_t parse_atom(size_t start_pos, size_t end_pos, Atom* parent = nullptr);

    Atom* parse_ftyp(size_t size, size_t data_pos);
    Atom* parse_free(size_t size, size_t data_pos);
    Atom* parse_skip(size_t size, size_t data_pos);
    Atom* parse_wide(size_t size, size_t data_pos);
    Atom* parse_mdat(size_t size, size_t data_pos);
    Atom* parse_pnot(size_t size, size_t data_pos);
    Atom* parse_mvhd(size_t size, size_t data_pos);
    Atom* parse_ctab(size_t size, size_t data_pos);
    Atom* parse_tkhd(size_t size, size_t data_pos);
    Atom* parse_txas(size_t size, size_t data_pos);
    Atom* parse_clef(size_t size, size_t data_pos);
    Atom* parse_prof(size_t size, size_t data_pos);
    Atom* parse_enof(size_t size, size_t data_pos);
    Atom* parse_elst(size_t size, size_t data_pos);
    Atom* parse_load(size_t size, size_t data_pos);
    Atom* parse_mdhd(size_t size, size_t data_pos);
    Atom* parse_elng(size_t size, size_t data_pos);
    Atom* parse_hdlr(size_t size, size_t data_pos);
    Atom* parse_vmhd(size_t size, size_t data_pos);
    Atom* parse_smhd(size_t size, size_t data_pos);
    Atom* parse_gmin(size_t size, size_t data_pos);
    Atom* parse_dref(size_t size, size_t data_pos);
    Atom* parse_stsd(size_t size, size_t data_pos);
    Atom* parse_stts(size_t size, size_t data_pos);
    Atom* parse_ctts(size_t size, size_t data_pos);
    Atom* parse_cslg(size_t size, size_t data_pos);
    Atom* parse_stss(size_t size, size_t data_pos);
    Atom* parse_stps(size_t size, size_t data_pos);
    Atom* parse_stsc(size_t size, size_t data_pos);
    Atom* parse_stsz(size_t size, size_t data_pos);
    Atom* parse_stco(size_t size, size_t data_pos);
    Atom* parse_sdtp(size_t size, size_t data_pos);

private:
    Atom root;
    std::unordered_map<std::string, std::function<Atom*(size_t, size_t)>> leaf_parse_func;
};


// todo:
// 1. parse metadata https://developer.apple.com/documentation/quicktime-file-format/metadata_atoms_and_types
// 2. parse sound_media https://developer.apple.com/documentation/quicktime-file-format/sound_media


#endif //MEDIAFORMATPARSER_M4APARSER_H
