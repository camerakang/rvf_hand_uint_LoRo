#include "endian.hpp"

/// @brief 判断为大端还是小端面设备
/// @return 0为大端，1为小端，其它未知
const char machine_endian()
{
    int a = 1; // 0x0000 0001
    // 如果是大端 低位字节放到高地址，高位字节放到低地址
    // 00 00 00 01
    // 如果是小端 高位字节放到高地址，低位字节放到低地址
    // 01 00 00 00
    char *p = reinterpret_cast<char *>(&a); // 字符指针只读1个字节，读8位
                                            // 如果是大端存储，则p读取的值是0
                                            // 如果是小端存储，则p读取的值是1
    return *p;
}
