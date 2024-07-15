#include "bytes_string.hpp"

#include <string>
#include <vector>
#include <ctype.h>

const char hexme_uppercase[]{"0123456789ABCDEF"};
const char hexme_lowercase[]{"0123456789abcdef"};

/// @brief 转换成byte字符串
/// @param data_ptr 输入数据
/// @param size 数据长度
/// @param hex_uppercase 是否使用大写的16进制表示
/// @return 形如“ff aa bb”的数据
const std::string to_hex_str(const void *data_ptr, const size_t size, bool hex_uppercase)
{
  if (!data_ptr)
  {
    return "";
  }
  auto str_ptr = static_cast<const unsigned char *>(data_ptr);
  auto hexme = hex_uppercase ? hexme_uppercase : hexme_lowercase;
  std::string result{};
  size_t pos{0};
  for (; pos < size - 1; ++pos)
  {
    result.push_back(hexme[(str_ptr[pos] & 0xF0) >> 4]);
    result.push_back(hexme[str_ptr[pos] & 0x0F]);
    result.push_back(' ');
  }
  result.push_back(hexme[(str_ptr[pos] & 0xF0) >> 4]);
  result.push_back(hexme[str_ptr[pos] & 0x0F]);
  return result;
}

const std::string to_hex_str(const char data_ptr, bool hex_uppercase)
{
  auto hexme = hex_uppercase ? hexme_uppercase : hexme_lowercase;
  std::string result{};
  result.push_back(hexme[(data_ptr & 0xF0) >> 4]);
  result.push_back(hexme[data_ptr & 0x0F]);
  return result;
}

/// @brief 转换成byte字符串
/// @param data_ptr 输入数据
/// @param size 数据长度
/// @param hex_uppercase 是否使用大写的16进制表示
/// @return 形如“abc\x00\x01”的数据
const std::string to_byte_str(const void *data_ptr, const size_t size, bool hex_uppercase)
{
  if (!data_ptr)
  {
    return "";
  }
  auto str_ptr = static_cast<const unsigned char *>(data_ptr);
  auto hexme = hex_uppercase ? hexme_uppercase : hexme_lowercase;
  std::string result{};
  for (size_t pos = 0; pos < size; ++pos)
  {
    if (isascii(str_ptr[pos]))
    {
      result.push_back(str_ptr[pos]);
    }
    else
    {
      result.push_back('\\');
      result.push_back('x');
      result.push_back(hexme[(str_ptr[pos] & 0xF0) >> 4]);
      result.push_back(hexme[str_ptr[pos] & 0x0F]);
    }
  }
  return result;
}

/// @brief 转换成byte字符串，只显示可见字符（如回车、空格等不显示）
/// @param data_ptr 输入数据
/// @param size 数据长度
/// @param hex_uppercase 是否使用大写的16进制表示
/// @return 形如“abc\x00\x01”的数据
const std::string to_byte_vistr(const void *data_ptr, const size_t size, bool hex_uppercase)
{
  if (!data_ptr)
  {
    return "";
  }
  auto str_ptr = static_cast<const unsigned char *>(data_ptr);
  auto hexme = hex_uppercase ? hexme_uppercase : hexme_lowercase;
  std::string result{};
  for (size_t pos = 0; pos < size; ++pos)
  {
    if (isprint(str_ptr[pos]))
    {
      result.push_back(str_ptr[pos]);
    }
    else
    {
      result.push_back('\\');
      result.push_back('x');
      result.push_back(hexme[(str_ptr[pos] & 0xF0) >> 4]);
      result.push_back(hexme[str_ptr[pos] & 0x0F]);
    }
  }
  return result;
}

const std::string to_byte_vistr(const char chr_val, bool hex_uppercase)
{
  auto hexme = hex_uppercase ? hexme_uppercase : hexme_lowercase;
  std::string result{};
  if (isprint(chr_val))
  {
    result.push_back(chr_val);
  }
  else
  {
    result.push_back('\\');
    result.push_back('x');
    result.push_back(hexme[(chr_val & 0xF0) >> 4]);
    result.push_back(hexme[chr_val & 0x0F]);
  }
  return result;
}
