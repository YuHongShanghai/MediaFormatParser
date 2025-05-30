//
// Created by 余泓 on 2025/5/26.
//

#include "M4aParser.h"
#include "utils.h"
#include "logger/easylogging++.h"

std::ostream& print_atom(std::ostream& out, const Atom& a, int indent_level) {
    std::string indent(indent_level * 4, ' ');  // 每层缩进 4 个空格
    std::string type(a.type, 4);

    out << indent << type << ":" << std::endl;
    out << indent << "    size: " << a.size << std::endl;
    out << indent << "    type: " << type << std::endl;
    if (a.extended_size) {
        out << indent << "    extendedSize: " << a.extended_size << std::endl;
    }

    for (const auto& child : a.children) {
        print_atom(out, *child, indent_level + 1);  // 递归打印子 box
    }
    return out;
}

std::ostream& operator << (std::ostream &out, const Atom &a) {
    return print_atom(out, a, 0);
}

std::ostream& operator << (std::ostream &out, const FtypAtom &a) {
    out << TYPE_FTYP << ":" << std::endl;
    out << "\tsize: " << a.size << std::endl;
    out << "\ttype: " << std::string(a.type, 4) << std::endl;
    out << "\tmajorBrand: " << std::string(a.major_brand, 4) << std::endl;
    out << "\tminorVersion: " << static_cast<int>(a.minor_version[0]) << " "
            << static_cast<int>(a.minor_version[1]) << " "
            << static_cast<int>(a.minor_version[2]) << " "
            << static_cast<int>(a.minor_version[3]) << std::endl;
    out << "\tcompatibleBrands: ";
    for (char* c: a.compatible_brands) {
        out << std::string(c, 4) << " ";
    }
    out << std::endl;
    return out;
}

std::ostream& operator << (std::ostream &out, const FreeAtom &a) {
    out << TYPE_FREE << ":" << std::endl;
    out << "\tsize: " << a.size << std::endl;
    out << "\ttype: " << std::string(a.type, 4) << std::endl;
    out << "\tfreeSpace:" << (a.size - 8) << std::endl;
    return out;
}

std::ostream& operator << (std::ostream &out, const SkipAtom &a) {
    out << TYPE_SKIP << ":" << std::endl;
    out << "\tsize: " << a.size << std::endl;
    out << "\ttype: " << std::string(a.type, 4) << std::endl;
    out << "\tfreeSpace:" << (a.size - 8) << std::endl;
    return out;
}

std::ostream& operator << (std::ostream &out, const MdatAtom &a) {
    out << TYPE_MDAT << ":" << std::endl;
    out << "\tsize: " << a.size << std::endl;
    out << "\ttype: " << std::string(a.type, 4) << std::endl;
    out << "\textendedSize: " << a.extended_size << std::endl;
    size_t data_size = a.extended_size == 0 ? a.size - 8: a.extended_size - 16;
    out << "\tdata:" << "(" << data_size << " bytes)" << std::endl;
    return out;
}

M4aParser::M4aParser(const std::string& filePath): Parser(filePath) {

}

M4aParser::~M4aParser() {

}

int M4aParser::custom_parse() {
    while (pos_ < data_size_) {
        size_t pos = pos_;
        uint32_t size = bytes_to_int4_be(data_ + pos);

        pos += 4;
        if (pos >= data_size_) {
            LOG(ERROR) << "not enough data 1";
            return -1;
        }

        char type[4];
        memcpy(type, data_ + pos, 4);

        LOG(INFO) << "got atom type " << std::string(type, 4);
        pos += 4;

        uint64_t extended_size = 0;
        if (size == 1) {
            if (pos >= data_size_) {
                LOG(ERROR) << "not enough data 2";
                return -2;
            }
            extended_size = bytes_to_int8_be(data_ + pos);
            pos += 8;
        }

        size_t offset = size == 1 ? extended_size : size;
        if (pos_ + offset > data_size_) {
            LOG(ERROR) << "not enough data 3";
            return -3;
        }

        if (memcmp(type, TYPE_FTYP, 4) == 0) {
            parse_ftyp(size, type, pos);
        } else if (memcmp(type, TYPE_FREE, 4) == 0) {
            parse_free(size, type, pos);
        } else if (memcmp(type, TYPE_SKIP, 4) == 0) {
            parse_skip(size, type, pos);
        } else if (memcmp(type, TYPE_MDAT, 4) == 0) {
            parse_mdat(size, type, extended_size, pos);
        } else if (memcmp(type, TYPE_MOOV, 4) == 0) {
            parse_moov(size, type, pos);
        }

        pos_ += offset;
    }

    return 0;
}

int M4aParser::parse_ftyp(uint32_t size, char* type, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    FtypAtom *atom = new FtypAtom();
    atom->size = size;
    memcpy(atom->type, type, 4);

    memcpy(atom->major_brand, data_ + data_pos, 4);
    data_pos += 4;

    memcpy(atom->minor_version, data_ + data_pos, 4);
    data_pos += 4;

    int cur_len = 16;
    while (cur_len + 4 < size) {
        char* band = new char[4];
        memcpy(band, data_ + data_pos, 4);
        atom->compatible_brands.push_back(band);
        data_pos += 4;
        cur_len += 4;
    }

    LOG(INFO) << *atom;
    atoms.push_back(atom);
    return 0;
}

int M4aParser::parse_free(uint32_t size, char *type, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    FreeAtom *atom = new FreeAtom();
    atom->size = size;
    memcpy(atom->type, type, 4);
    LOG(INFO) << *atom;
    atoms.push_back(atom);
    return 0;
}

int M4aParser::parse_skip(uint32_t size, char *type, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    SkipAtom *atom = new SkipAtom();
    atom->size = size;
    memcpy(atom->type, type, 4);
    LOG(INFO) << *atom;
    atoms.push_back(atom);
    return 0;
}

int M4aParser::parse_mdat(uint32_t size, char* type, uint64_t extended_size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    MdatAtom *atom = new MdatAtom();
    atom->size = size;
    memcpy(atom->type, type, 4);
    atom->extended_size = extended_size;
    atom->data = data_ + data_pos;
    LOG(INFO) << *atom;
    atoms.push_back(atom);
    return 0;
}

int M4aParser::parse_moov(uint32_t size, char* type, size_t data_pos) {
    LOG(INFO) << __FUNCTION__;
    Atom *atom = new Atom();
    atom->size = size;
    memcpy(atom->type, type, 4);

    char *child_types[8] = {TYPE_PRFL, TYPE_MVHD, TYPE_CLIP, TYPE_TRAK, TYPE_UDTA, TYPE_CTAB,
                            TYPE_CMOV, TYPE_RMRA};

    while (data_pos - pos_ < size) {
        uint32_t child_size = bytes_to_int4_be(data_ + data_pos);
        data_pos += 4;

        char child_type[4];
        memcpy(child_type, data_ + data_pos, 4);
        data_pos += 4;

        LOG(INFO) << "got atom type " << std::string(child_type, 4);
        //for (int i = 0; i < 8; ++i) {
            if (memcmp(child_type, TYPE_MVHD, 4) == 0) {
                // todo
            }
        //}
        atoms.push_back(atom);
        data_pos += child_size - 8;
    }

    return 0;
}


int M4aParser::dump_info() {
    return 0;
}

int M4aParser::dump_data() {
    return 0;
}
