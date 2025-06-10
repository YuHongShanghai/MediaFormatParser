//
// Created by 余泓 on 2025/5/23.
//

#ifndef MEDIAFORMATPARSER_UTILS_H
#define MEDIAFORMATPARSER_UTILS_H

#include <string>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <arm_neon.h>

bool get_bit(char c, int n);

uint64_t bytes_to_int8_be(const unsigned char* data);

double bytes_to_double_be(const unsigned char* data);

float bytes_to_fixed2_be(const uint8_t* data);

float bytes_to_fixed4_be(const uint8_t* data);

float32_t bytes_to_float4_be(const unsigned char* data);

float16_t bytes_to_float2_be(const unsigned char* data);

uint32_t bytes_to_int4_be(const unsigned char* data);

uint32_t bytes_to_int3_be(const unsigned char* data);

uint16_t bytes_to_int2_be(const unsigned char* data);

uint32_t bytes_to_int4_le(const unsigned char* data);

uint32_t bytes_to_int3_le(const unsigned char* data);

uint16_t bytes_to_int2_le(const unsigned char* data);

uint8_t int_value(unsigned char c);

std::string get_filename_without_extension(const std::string& path);

int write_data(std::string file_path, unsigned char *data, size_t size);

std::string get_output_dir();

void write_u64(uint64_t & x, int length, int value);

#endif //MEDIAFORMATPARSER_UTILS_H
