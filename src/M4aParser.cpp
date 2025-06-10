//
// Created by 余泓 on 2025/5/26.
//

#include "M4aParser.h"
#include "utils.h"
#include "logger/easylogging++.h"

std::ostream& operator<<(std::ostream& os, const Atom& a) {
    a.print(os);
    return os;
}

M4aParser::M4aParser(const std::string& filePath): Parser(filePath) {
    register_parse_functions();
}

M4aParser::~M4aParser() {

}

uint64_t M4aParser::parse_atom(size_t start_pos, size_t end_pos, Atom* parent) {
    LOG(DEBUG) << __FUNCTION__;
    if (parent == nullptr) {
        LOG(ERROR) << "parent is null";
        return 0;
    }

    if (end_pos > data_size_) {
        LOG(ERROR) << "not enough data 1";
        return 0;
    }

    if (end_pos - start_pos < 8) {
        LOG(ERROR) << "not enough data 2";
        return 0;
    }

    size_t pos = start_pos;
    uint32_t size = bytes_to_int4_be(data_ + pos);
    pos += 4;

    uint64_t data_size = size;

    char type[4];
    memcpy(type, data_ + pos, 4);

    pos += 4;

    uint64_t extended_size = 0;
    if (size == 1) {
        if (pos + 8 > end_pos) {
            LOG(ERROR) << "not enough data 3";
            return 0;
        }

        extended_size = bytes_to_int8_be(data_ + pos);
        data_size = extended_size;
        pos += 8;
    }

    if (start_pos + data_size > end_pos) {
        LOG(ERROR) << "not enough data 4";
        return 0;
    }

    size_t data_end_pos = start_pos + data_size;
    start_pos = pos;

    std::string type_str = std::string(type, 4);
    LOG(DEBUG) << "got atom " << type_str;

    // if is a leaf atom
    if (leaf_parse_func.count(type_str)) {
        Atom* child = leaf_parse_func[type_str](data_size, start_pos);
        if (child == nullptr) {
            LOG(ERROR) << "parse atom failed";
        } else {
            child->size = size;
            child->extended_size = extended_size;
            parent->children.push_back(child);
            LOG(INFO) << *child;
        }
    } else {
        // is a container
        Atom *atom = new Atom;
        atom->size = size;
        memcpy(atom->type, type, 4);
        atom->extended_size = extended_size;

        if (parent) {
            parent->children.push_back(atom);
        }

        // parse children
        while (start_pos < data_end_pos) {
            auto child_size = parse_atom(start_pos, data_end_pos, atom);
            if (child_size == 0) {
                LOG(ERROR) << "not enough data 6";
                break;
            }
            start_pos += child_size;
        }
    }
    return data_size;
}

int M4aParser::custom_parse() {
    while (pos_ < data_size_) {
        auto child_size = parse_atom(pos_, data_size_, &root);
        if (child_size < 0) {
            LOG(ERROR) << "not enough data 6";
            return -1;
        }
        pos_ += child_size;
    }
    return 0;
}

Atom* M4aParser::parse_ftyp(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    FtypAtom *atom = new FtypAtom();

    memcpy(atom->major_brand, data_ + data_pos, 4);
    data_pos += 4;

    memcpy(atom->minor_version, data_ + data_pos, 4);
    data_pos += 4;

    int cur_len = 16;
    while (cur_len + 4 <= size) {
        char* band = new char[4];
        memcpy(band, data_ + data_pos, 4);
        atom->compatible_brands.push_back(band);
        data_pos += 4;
        cur_len += 4;
    }

    return atom;
}

Atom* M4aParser::parse_free(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    FreeAtom *atom = new FreeAtom();
    atom->free_space = size - 8;

    return atom;
}

Atom* M4aParser::parse_skip(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    SkipAtom *atom = new SkipAtom();
    atom->free_space = size - 8;

    return atom;
}

Atom* M4aParser::parse_wide(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    WideAtom *atom = new WideAtom();

    return atom;
}

Atom* M4aParser::parse_mdat(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    MdatAtom *atom = new MdatAtom();
    atom->data = data_ + data_pos;
    return atom;
}

Atom* M4aParser::parse_pnot(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    auto *atom = new PnotAtom();

    atom->modification_date = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->version_number = bytes_to_int2_be(data_ + data_pos);
    data_pos += 2;

    atom->atom_type = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->atom_index = bytes_to_int2_be(data_ + data_pos);
    data_pos += 2;

    return atom;
}

Atom* M4aParser::parse_mvhd(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    auto *atom = new MvhdAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->creation_time = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->modification_time = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->time_scale = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->duration = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->preferred_rate = bytes_to_fixed4_be(data_ + data_pos);;
    data_pos += 4;

    atom->preferred_volume = bytes_to_fixed2_be(data_ + data_pos);
    data_pos += 2;

    for (int & i : atom->matrix_structure) {
        memcpy(&i, data_ + data_pos, 4);
        data_pos += 4;
    }

    atom->preview_time = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->preview_duration = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->poster_time = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->selection_time = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->selection_duration = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->current_time = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->next_track_id = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    return atom;
}

Atom* M4aParser::parse_ctab(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    auto *atom = new CtabAtom();

    atom->color_table_seed = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->color_table_flags = bytes_to_int2_be(data_ + data_pos);
    data_pos += 2;

    atom->color_table_size = bytes_to_int2_be(data_ + data_pos);
    data_pos += 2;

    for (int i = 0; i < atom->color_table_size; ++i) {
        Color color{};
        color.first = bytes_to_int2_be(data_ + data_pos);
        data_pos += 2;

        color.red = bytes_to_int2_be(data_ + data_pos);
        data_pos += 2;

        color.green = bytes_to_int2_be(data_ + data_pos);
        data_pos += 2;

        color.blue = bytes_to_int2_be(data_ + data_pos);
        data_pos += 2;

        atom->color_array.push_back(color);
    }

    return atom;
}

Atom* M4aParser::parse_tkhd(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    auto *atom = new TkhdAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->creation_time = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->modification_time = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->track_id = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->reserved1 = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->duration = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->reserved2 = bytes_to_int8_be(data_ + data_pos);
    data_pos += 8;

    atom->layer = bytes_to_int2_be(data_ + data_pos);
    data_pos += 2;

    atom->alternate_group = bytes_to_int2_be(data_ + data_pos);
    data_pos += 2;

    atom->volume = bytes_to_fixed2_be(data_ + data_pos);
    data_pos += 2;

    atom->reserved3 = bytes_to_int2_be(data_ + data_pos);
    data_pos += 2;

    for (int & i : atom->matrix_structure) {
        memcpy(&i, data_ + data_pos, 4);
        data_pos += 4;
    }

    atom->track_width = bytes_to_fixed4_be(data_ + data_pos);
    data_pos += 4;

    atom->track_height = bytes_to_fixed4_be(data_ + data_pos);
    data_pos += 4;

    return atom;
}

Atom* M4aParser::parse_txas(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    auto *atom = new TxasAtom();
    return atom;
}

Atom* M4aParser::parse_clef(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    auto atom = new ClefAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->width = bytes_to_fixed4_be(data_ + data_pos);
    data_pos += 4;

    atom->height = bytes_to_fixed4_be(data_ + data_pos);
    data_pos += 4;

    return atom;
}

Atom* M4aParser::parse_prof(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    auto atom = new ProfAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->width = bytes_to_fixed4_be(data_ + data_pos);
    data_pos += 4;

    atom->height = bytes_to_fixed4_be(data_ + data_pos);
    data_pos += 4;

    return atom;
}

Atom* M4aParser::parse_enof(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    auto atom = new EnofAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->width = bytes_to_fixed4_be(data_ + data_pos);
    data_pos += 4;

    atom->height = bytes_to_fixed4_be(data_ + data_pos);
    data_pos += 4;

    return atom;
}

Atom* M4aParser::parse_elst(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    auto atom = new ElstAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->entry_num = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    for (size_t i = 0; i < atom->entry_num; ++i) {
        uint32_t track_duration = bytes_to_int4_be(data_ + data_pos);
        data_pos += 4;

        uint32_t media_time = bytes_to_int4_be(data_ + data_pos);
        data_pos += 4;

        float media_rate = bytes_to_fixed4_be(data_ + data_pos);
        data_pos += 4;

        atom->edit_list_table.push_back({track_duration, media_time, media_rate});
        data_pos += 4;
    }
    return atom;
}

Atom* M4aParser::parse_load(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__;
    auto atom = new LoadAtom();

    atom->preload_start_time = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->preload_duration = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->preload_flags = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->default_hints = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    return atom;
}

Atom* M4aParser::parse_mdhd(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    auto atom = new MdhdAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->creation_time = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->modification_time = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->time_scale = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->duration = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->language = bytes_to_int2_be(data_ + data_pos);
    data_pos += 2;

    atom->quality = bytes_to_int2_be(data_ + data_pos);
    data_pos += 2;

    return atom;
}

Atom* M4aParser::parse_elng(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    auto atom = new ElngAtom();

    auto end_pos = data_pos + size - 8;

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    bool got_str_end = false;
    while (data_pos < end_pos) {
        char c = data_[data_pos++];
        if (c == 0) {
            got_str_end = true;
            break;
        }
        atom->language_tag_string.push_back(c);
    }
    if (!got_str_end) {
        LOG(ERROR) << "parse language_tag_string failed";
        atom->language_tag_string = "";
        delete atom;
        return nullptr;
    }

    return atom;
}

Atom* M4aParser::parse_hdlr(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    auto atom = new HdlrAtom();

    auto end_pos = data_pos + size - 8;

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    memcpy(atom->component_type, data_ + data_pos, 4);
    data_pos += 4;

    if (memcmp(atom->component_type, "mhlr", 4) != 0 || memcmp(atom->component_type, "dhlr", 4) != 0) {
        LOG(WARNING) << "not valid component type, got " << std::string(atom->component_type, 4) << ", expected mhlr or dhlr";
    }

    memcpy(atom->component_subtype, data_ + data_pos, 4);
    data_pos += 4;

    memcpy(&atom->component_manufacturer, data_ + data_pos, 4);
    data_pos += 4;

    memcpy(&atom->component_flags, data_ + data_pos, 4);
    data_pos += 4;

    memcpy(&atom->component_flags_mask, data_ + data_pos, 4);
    data_pos += 4;

    atom->component_name = std::string((char*)(data_ + data_pos), end_pos - data_pos);
    return atom;
}


Atom* M4aParser::parse_vmhd(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    auto atom = new VmhdAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->graphics_mode = bytes_to_int2_be(data_ + data_pos);
    data_pos += 2;

    atom->opcolor_red = bytes_to_int2_be(data_ + data_pos);
    data_pos += 2;

    atom->opcolor_green = bytes_to_int2_be(data_ + data_pos);
    data_pos += 2;

    atom->opcolor_blue = bytes_to_int2_be(data_ + data_pos);
    data_pos += 2;

    return atom;
}

Atom* M4aParser::parse_smhd(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    auto atom = new SmhdAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->balance = bytes_to_int2_be(data_ + data_pos);

    return atom;
}

Atom* M4aParser::parse_gmin(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    auto atom = new GminAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->graphics_mode = bytes_to_int2_be(data_ + data_pos);
    data_pos += 2;

    atom->opcolor_red = bytes_to_int2_be(data_ + data_pos);
    data_pos += 2;

    atom->opcolor_green = bytes_to_int2_be(data_ + data_pos);
    data_pos += 2;

    atom->opcolor_blue = bytes_to_int2_be(data_ + data_pos);
    data_pos += 2;

    atom->balance = bytes_to_int2_be(data_ + data_pos);

    return atom;
}

Atom* M4aParser::parse_dref(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    auto atom = new DrefAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->entry_num = bytes_to_int4_be(data_ + data_pos);

    return atom;
}

Atom* M4aParser::parse_stsd(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    auto atom = new StsdAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->entry_num = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    for (uint32_t i = 0; i < atom->entry_num; ++i) {
        auto mediaDataAtom = new MediaDataAtom;
        size_t pos = data_pos;
        mediaDataAtom->sample_description_size = bytes_to_int4_be(data_ + pos);
        pos += 4;
        memcpy(mediaDataAtom->data_format, data_ + pos, 4);
        pos += 4;
        pos += 6; // reserved
        mediaDataAtom->data_reference_index = bytes_to_int2_be(data_ + pos);

        atom->sample_description_table.push_back(mediaDataAtom);

        data_pos += mediaDataAtom->sample_description_size;
    }

    return atom;
}

Atom* M4aParser::parse_stts(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    auto atom = new SttsAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->entry_num = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    for (uint32_t i = 0; i < atom->entry_num; ++i) {
        uint32_t sample_count = bytes_to_int4_be(data_ + data_pos);
        data_pos += 4;

        uint32_t sample_duration = bytes_to_int4_be(data_ + data_pos);
        data_pos += 4;

        atom->time_to_sample_table.push_back({sample_count, sample_duration});
    }

    return atom;
}

Atom* M4aParser::parse_ctts(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    auto atom = new CttsAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->entry_count = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    for (uint32_t i = 0; i < atom->entry_count; ++i) {
        uint32_t sample_count = bytes_to_int4_be(data_ + data_pos);
        data_pos += 4;

        uint32_t composition_offset = bytes_to_int4_be(data_ + data_pos);
        data_pos += 4;

        atom->composition_offset_table.push_back({sample_count, composition_offset});
    }

    return atom;
}

Atom* M4aParser::parse_cslg(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    auto atom = new CslgAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->composition_offset_to_display_offset_shift = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->least_display_offset = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->greatest_display_offset = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->display_start_time = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->display_end_time = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    return atom;
}

Atom* M4aParser::parse_stss(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    auto atom = new StssAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->entry_num = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    for (uint32_t i = 0; i < atom->entry_num; ++i) {
        atom->sample_numbers.push_back(bytes_to_int4_be(data_ + data_pos));
        data_pos += 4;
    }

    return atom;
}

Atom* M4aParser::parse_stps(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    auto atom = new StpsAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->entry_num = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    for (uint32_t i = 0; i < atom->entry_num; ++i) {
        atom->sample_numbers.push_back(bytes_to_int4_be(data_ + data_pos));
        data_pos += 4;
    }

    return atom;
}

Atom* M4aParser::parse_stsc(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    auto atom = new StscAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->entry_num = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    for (uint32_t i = 0; i < atom->entry_num; ++i) {
        StscEntry entry{};

        entry.first_chunk = bytes_to_int4_be(data_ + data_pos);
        data_pos += 4;

        entry.sample_per_chunk = bytes_to_int4_be(data_ + data_pos);
        data_pos += 4;

        entry.sample_per_chunk = bytes_to_int4_be(data_ + data_pos);
        data_pos += 4;

        atom->sample_to_chunk_table.push_back(entry);
    }

    return atom;
}

Atom* M4aParser::parse_stsz(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    auto atom = new StszAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->sample_size = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    atom->entry_num = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    for (uint32_t i = 0; i < atom->entry_num; ++i) {
        atom->sample_size_table.push_back(bytes_to_int4_be(data_ + data_pos));
        data_pos += 4;
    }

    return atom;
}

Atom* M4aParser::parse_stco(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    auto atom = new StcoAtom();

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    atom->entry_num = bytes_to_int4_be(data_ + data_pos);
    data_pos += 4;

    for (uint32_t i = 0; i < atom->entry_num; ++i) {
        atom->chunk_offset_table.push_back(bytes_to_int4_be(data_ + data_pos));
        data_pos += 4;
    }

    return atom;
}

Atom* M4aParser::parse_sdtp(size_t size, size_t data_pos) {
    LOG(DEBUG) << __FUNCTION__ ;
    auto atom = new SdtpAtom();

    auto end_pos = data_pos + size - 8;

    atom->version = int_value(data_[data_pos]);
    data_pos++;

    memcpy(atom->flags, data_ + data_pos, 3);
    data_pos += 3;

    while (data_pos < end_pos) {
        atom->sample_dependency_flags_table.push_back(int_value(data_[data_pos]));
        data_pos += 1;
    }

    return atom;
}


void M4aParser::register_parse_functions() {
    leaf_parse_func[TYPE_MDAT] = [this](size_t size, size_t data_pos) {
        return this->parse_mdat(size, data_pos);
    };

    leaf_parse_func[TYPE_FTYP] = [this](size_t size, size_t data_pos) {
        return this->parse_ftyp(size, data_pos);
    };

    leaf_parse_func[TYPE_FREE] = [this](size_t size, size_t data_pos) {
        return this->parse_free(size, data_pos);
    };

    leaf_parse_func[TYPE_SKIP] = [this](size_t size, size_t data_pos) {
        return this->parse_skip(size, data_pos);
    };

    leaf_parse_func[TYPE_WIDE] = [this](size_t size, size_t data_pos) {
        return this->parse_wide(size, data_pos);
    };

    leaf_parse_func[TYPE_PNOT] = [this](size_t size, size_t data_pos) {
        return this->parse_pnot(size, data_pos);
    };

    leaf_parse_func[TYPE_MVHD] = [this](size_t size, size_t data_pos) {
        return this->parse_mvhd(size, data_pos);
    };

    leaf_parse_func[TYPE_CTAB] = [this](size_t size, size_t data_pos) {
        return this->parse_ctab(size, data_pos);
    };

    leaf_parse_func[TYPE_TKHD] = [this](size_t size, size_t data_pos) {
        return this->parse_tkhd(size, data_pos);
    };

    leaf_parse_func[TYPE_TXAS] = [this](size_t size, size_t data_pos) {
        return this->parse_txas(size, data_pos);
    };

    leaf_parse_func[TYPE_CLEF] = [this](size_t size, size_t data_pos) {
        return this->parse_clef(size, data_pos);
    };

    leaf_parse_func[TYPE_PROF] = [this](size_t size, size_t data_pos) {
        return this->parse_prof(size, data_pos);
    };

    leaf_parse_func[TYPE_ENOF] = [this](size_t size, size_t data_pos) {
        return this->parse_enof(size, data_pos);
    };

    leaf_parse_func[TYPE_ELST] = [this](size_t size, size_t data_pos) {
        return this->parse_elst(size, data_pos);
    };

    leaf_parse_func[TYPE_LOAD] = [this](size_t size, size_t data_pos) {
        return this->parse_load(size, data_pos);
    };

    leaf_parse_func[TYPE_MDHD] = [this](size_t size, size_t data_pos) {
        return this->parse_mdhd(size, data_pos);
    };

    leaf_parse_func[TYPE_ELNG] = [this](size_t size, size_t data_pos) {
        return this->parse_elng(size, data_pos);
    };

    leaf_parse_func[TYPE_HDLR] = [this](size_t size, size_t data_pos) {
        return this->parse_hdlr(size, data_pos);
    };

    leaf_parse_func[TYPE_VMHD] = [this](size_t size, size_t data_pos) {
        return this->parse_vmhd(size, data_pos);
    };

    leaf_parse_func[TYPE_SMHD] = [this](size_t size, size_t data_pos) {
        return this->parse_smhd(size, data_pos);
    };

    leaf_parse_func[TYPE_GMIN] = [this](size_t size, size_t data_pos) {
        return this->parse_gmin(size, data_pos);
    };

    leaf_parse_func[TYPE_DREF] = [this](size_t size, size_t data_pos) {
        return this->parse_dref(size, data_pos);
    };

    leaf_parse_func[TYPE_STSD] = [this](size_t size, size_t data_pos) {
        return this->parse_stsd(size, data_pos);
    };

    leaf_parse_func[TYPE_STTS] = [this](size_t size, size_t data_pos) {
        return this->parse_stts(size, data_pos);
    };

    leaf_parse_func[TYPE_CTTS] = [this](size_t size, size_t data_pos) {
        return this->parse_ctts(size, data_pos);
    };

    leaf_parse_func[TYPE_CSLG] = [this](size_t size, size_t data_pos) {
        return this->parse_cslg(size, data_pos);
    };

    leaf_parse_func[TYPE_STSS] = [this](size_t size, size_t data_pos) {
        return this->parse_stss(size, data_pos);
    };

    leaf_parse_func[TYPE_STPS] = [this](size_t size, size_t data_pos) {
        return this->parse_stps(size, data_pos);
    };

    leaf_parse_func[TYPE_STSC] = [this](size_t size, size_t data_pos) {
        return this->parse_stsc(size, data_pos);
    };

    leaf_parse_func[TYPE_STSZ] = [this](size_t size, size_t data_pos) {
        return this->parse_stsz(size, data_pos);
    };

    leaf_parse_func[TYPE_STCO] = [this](size_t size, size_t data_pos) {
        return this->parse_stco(size, data_pos);
    };

    leaf_parse_func[TYPE_SDTP] = [this](size_t size, size_t data_pos) {
        return this->parse_sdtp(size, data_pos);
    };
}

int M4aParser::dump_info() {
    return 0;
}

int M4aParser::dump_data() {
    return 0;
}
