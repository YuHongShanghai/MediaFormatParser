#include "utils.h"

bool get_bit(char c, int n) {
    return (c >> (8 - n)) & 1;
}

uint64_t bytes_to_int8_be(const unsigned char* data) {
    return  (static_cast<uint64_t>(static_cast<uint8_t>(data[0])) << 56) |
            (static_cast<uint64_t>(static_cast<uint8_t>(data[1])) << 48) |
            (static_cast<uint64_t>(static_cast<uint8_t>(data[2])) << 40) |
            (static_cast<uint64_t>(static_cast<uint8_t>(data[3])) << 32) |
            (static_cast<uint64_t>(static_cast<uint8_t>(data[4])) << 24) |
            (static_cast<uint64_t>(static_cast<uint8_t>(data[5])) << 16) |
            (static_cast<uint64_t>(static_cast<uint8_t>(data[6])) << 8) |
            (static_cast<uint64_t>(static_cast<uint8_t>(data[7])));
}

double bytes_to_double_be(const unsigned char* data) {
    uint64_t temp = bytes_to_int8_be(data);
    double result;
    std::memcpy(&result, &temp, sizeof(result));
    return result;
}

float bytes_to_fixed4_be(const uint8_t* data) {
    int16_t int_part = (data[0] << 8) | data[1];
    uint16_t frac_part = (data[2] << 8) | data[3];
    return static_cast<float>(int_part) + static_cast<float>(frac_part) / 65536.0;
}

float bytes_to_fixed2_be(const uint8_t* data) {
    int16_t raw = (data[0] << 8) | data[1];
    return static_cast<float>(raw) / 256.0f;
}

float32_t bytes_to_float4_be(const unsigned char* data) {
    auto temp = bytes_to_int4_be(data);
    float32_t result;
    std::memcpy(&result, &temp, sizeof(result));
    return result;
}

float16_t bytes_to_float2_be(const unsigned char* data) {
    auto temp = bytes_to_int2_be(data);
    float16_t result;
    std::memcpy(&result, &temp, sizeof(result));
    return result;
}

uint32_t bytes_to_int4_be(const unsigned char* data) {
    return (static_cast<uint8_t>(data[0]) << 24) |
           (static_cast<uint8_t>(data[1]) << 16) |
           (static_cast<uint8_t>(data[2]) << 8)  |
           (static_cast<uint8_t>(data[3]));
}

uint32_t bytes_to_int3_be(const unsigned char* data) {
    return (static_cast<uint8_t>(data[0]) << 16) |
           (static_cast<uint8_t>(data[1]) << 8) |
           (static_cast<uint8_t>(data[2]));
}

uint16_t bytes_to_int2_be(const unsigned char* data) {
    return (static_cast<uint8_t>(data[0]) << 8)  |
           (static_cast<uint8_t>(data[1]));
}

uint32_t bytes_to_int4_le(const unsigned char* data) {
    return (static_cast<uint8_t>(data[3]) << 24) |
           (static_cast<uint8_t>(data[2]) << 16) |
           (static_cast<uint8_t>(data[1]) << 8)  |
           (static_cast<uint8_t>(data[0]));
}

uint32_t bytes_to_int3_le(const unsigned char* data) {
    return (static_cast<uint8_t>(data[2]) << 16) |
           (static_cast<uint8_t>(data[1]) << 8)  |
           (static_cast<uint8_t>(data[0]));
}

uint16_t bytes_to_int2_le(const unsigned char* data) {
    return (static_cast<uint8_t>(data[1]) << 8) |
           (static_cast<uint8_t>(data[0]));
}

uint8_t int_value(unsigned char c) {
    return c;
}

std::string get_filename_without_extension(const std::string& path) {
    std::filesystem::path p(path);
    return p.stem().string();
}

int write_data(std::string file_path, unsigned char *data, size_t size) {
    std::ofstream out(file_path, std::ios::binary);
    if (!out) {
        return -1;
    }

    out.write(reinterpret_cast<const char *>(data), size);
    out.close();
    return 0;
}

std::string get_output_dir() {
    return "../output/";
}

void write_u64(uint64_t & x, int length, int value)
{
    uint64_t mask = 0xFFFFFFFFFFFFFFFF >> (64 - length);
    x = (x << length) | ((uint64_t)value & mask);
}

