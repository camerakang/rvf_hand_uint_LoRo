/// @brief 字节顺序处理

#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include <type_traits>
#include <initializer_list>
#include <algorithm>

#ifndef LITTLE_ENDIAN
#ifndef BIG_ENDIAN

#define LITTLE_ENDIAN 0x00000041UL
#define BIG_ENDIAN 0x41000000UL
#define UNKNOWN_ENDIAN 0xFFFFFFFFUL
#define ENDIAN_ORDER_TEST ('A')

#if LITTLE_ENDIAN == ENDIAN_ORDER_TEST
#define MACHINE_ENDIAN LITTLE_ENDIAN
#elif BIG_ENDIAN == ENDIAN_ORDER_TEST
#define MACHINE_ENDIAN BIG_ENDIAN
#else
#define MACHINE_ENDIAN UNKNOWN_ENDIAN
#endif

#else
#define MACHINE_ENDIAN BIG_ENDIAN
#endif // BIG_ENDIAN

#else
#define MACHINE_ENDIAN LITTLE_ENDIAN
#endif // LITTLE_ENDIAN

/// @brief 判断为大端还是小端面设备
/// @return 0为大端，1为小端，其它未知
const char machine_endian();

template <typename _InputType, typename = std::enable_if<std::is_arithmetic<_InputType>::value>>
const _InputType endian_reverse(const _InputType *val)
{
  _InputType result{0};
  auto len = sizeof(_InputType);
  unsigned char *rptr{reinterpret_cast<unsigned char *>(&result)};
  unsigned char *vptr{reinterpret_cast<unsigned char *>((const_cast<_InputType *>(val)))};
  for (decltype(len) i = 0; i < len; i++)
  {
    rptr[i] = vptr[len - i - 1];
  }
  return result;
}

#define endian_big_to_little(val) endian_reverse(val) // 将大端数据成小端
#define endian_little_to_big(val) endian_reverse(val) // 将小端数据成大端

#if MACHINE_ENDIAN == LITTLE_ENDIAN
#define endian_local_to_little(val) (val)             // 将数据转换成小端，数据为本地定义的数据
#define endian_local_to_big(val) endian_reverse(&val) // 将数据转换成大端，数据为本地定义的数据
#define endian_big_to_local(val) endian_reverse(&val) // 将大端转换成当前的数据
#define endian_little_to_local(val) (val)             // 将小端转换成当前的数据
#elif MACHINE_ENDIAN == BIG_ENDIAN
#define endian_local_to_little(val) endian_reverse(&val) // 将数据转换成小端，数据为本地定义的数据
#define endian_local_to_big(val) (val)                   // 将数据转换成大端，数据为本地定义的数据
#define endian_big_to_local(val) (val)                   // 将大端转换成当前的数据
#define endian_little_to_local(val) endian_reverse(&val) // 将小端转换成当前的数据
#endif

#define USHORT_MSB(val) ((unsigned char)(((val)&0xFF00) >> 8))          // 取16位的高8位
#define USHORT_LSB(val) ((unsigned char)((val)&0x00FF))                 // 取16位的低8位
#define FETCH_BYTE(val, n) ((unsigned char)(((val) >> (n * 8)) & 0xFF)) // 按位取数

/// @brief 将输入数据合并成数字
/// @tparam _ResultType 返回的数据类型
/// @tparam _InputType 输入的数据类型
/// @param datlist 数据列表
/// @return 合并后的数据
template <typename _InputType, typename _ResultType, typename = std::enable_if<std::is_arithmetic<_ResultType>::value>>
const _ResultType number_merge(std::initializer_list<_InputType> datlist)
{
  _ResultType num{0};
  std::copy(datlist.begin(), datlist.end(), reinterpret_cast<_InputType *>(&num));
  return num;
}

/// @brief 将输入数据合并成数字
/// @tparam _ResultType 返回的数据类型
/// @tparam _InputType 输入的数据类型
/// @param dptr 数据列表
/// @param size 数据长度
/// @return 合并后的数据
template <typename _InputType, typename _ResultType, typename = std::enable_if<std::is_arithmetic<_ResultType>::value>>
const _ResultType number_merge(const _InputType *dptr, unsigned char size)
{
  _ResultType num{0};
  std::copy(dptr, dptr + size, reinterpret_cast<_InputType *>(&num));
  return num;
}

/// @brief 将输入数据合并成数字（合并前先反转数据顺序）
/// @tparam _ResultType 返回的数据类型
/// @tparam _InputType 输入的数据类型
/// @param datlist 数据列表
/// @return 合并后的数据
template <typename _InputType, typename _ResultType, typename = std::enable_if<std::is_arithmetic<_ResultType>::value>>
const _ResultType number_merge_reverse(std::initializer_list<_InputType> datlist)
{
  std::reverse(datlist.begin(), datlist.end());
  return number_merge(datlist);
}

/// @brief 将输入数据合并成数字（合并前先反转数据顺序）
/// @tparam _ResultType 返回的数据类型
/// @tparam _InputType 输入的数据类型
/// @param dptr 数据列表
/// @param size 数据长度
/// @return 合并后的数据
template <typename _InputType, typename _ResultType, typename = std::enable_if<std::is_arithmetic<_ResultType>::value>>
const _ResultType number_merge_reverse(const _InputType *dptr, unsigned char size)
{
  std::reverse(dptr, dptr + size);
  return number_merge(dptr);
}

/// @brief 将数据填充到指定的内存中
/// @tparam _InputType 输入数据的类型
/// @param num 输入数据
/// @param dest 内存地址
/// @return dest
template <typename _InputType, typename = std::enable_if<std::is_arithmetic<_InputType>::value>>
const void *number_fill(_InputType num, void *dest)
{
  *static_cast<_InputType *>(dest) = num;
  return dest;
}

/// @brief 将数据填充到指定的内存中（复制前先反转数据顺序）
/// @tparam _InputType 输入数据的类型
/// @param num 输入数据
/// @param dest 内存地址
/// @return dest
template <typename _InputType, typename = std::enable_if<std::is_arithmetic<_InputType>::value>>
const void *number_fill_reverse(_InputType num, void *dest)
{
  return number_fill(endian_reverse<_InputType>(num));
}

#endif //__ENDIAN_H__
