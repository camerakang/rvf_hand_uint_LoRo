#ifndef __BTYES_STRING_HPP__
#define __BTYES_STRING_HPP__

#include <string>
#include <vector>
#include <ctype.h>

/// @brief 转换成byte字符串
/// @param data_ptr 输入数据
/// @param size 数据长度
/// @param hex_uppercase 是否使用大写的16进制表示
/// @return 形如“ff aa bb”的数据
const std::string to_hex_str(const void *data_ptr, const size_t size, bool hex_uppercase = false);

const std::string to_hex_str(const char data_ptr, bool hex_uppercase = false);

/// @brief 转换成byte字符串
/// @param data_ptr 输入数据
/// @param size 数据长度
/// @param hex_uppercase 是否使用大写的16进制表示
/// @return 形如“abc\x00\x01”的数据
const std::string to_byte_str(const void *data_ptr, const size_t size, bool hex_uppercase = false);

/// @brief 转换成byte字符串，只显示可见字符（如回车、空格等不显示）
/// @param data_ptr 输入数据
/// @param size 数据长度
/// @param hex_uppercase 是否使用大写的16进制表示
/// @return 形如“abc\x00\x01”的数据
const std::string to_byte_vistr(const void *data_ptr, const size_t size, bool hex_uppercase = false);

const std::string to_byte_vistr(const char chr_val, bool hex_uppercase);

#endif // __BTYES_STRING_HPP__
