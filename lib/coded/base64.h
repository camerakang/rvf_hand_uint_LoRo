#ifndef __BASE64_H__
#define __BASE64_H__

#include <stdio.h>

#if __cplusplus
extern "C"
{
#endif

/// @brief 读取编码后的数据长度
/// @param inlen 输出数据的长度
/// @return 编码后的长度
int base64_encode_len( const int inlen);

/// @brief base64编码
/// @param indata 输入数据源
/// @param inlen 输入数据长度
/// @param outdata 输出数据地址，为字符
/// @param outlen 输出数据长度地址，为实际字符长度
/// @return 是否成功，-1为异常
int base64_encode(const void *indata, int inlen, char *outdata, int *outlen);

/// @brief 计算数据解码后的长度
/// @param indata 输入的数据，为字符串
/// @param inlen 字符串长度
/// @return 计算后的长度
int base64_decode_len(const char *indata, const int inlen);

/// @brief base64解码
/// @param indata 输入数据源，为字符
/// @param inlen 输入数据长度，为实际字符长度
/// @param outdata 输出数据地址
/// @param outlen 输出数据长度地址
/// @return 是否成功，-1为异常
int base64_decode(const char *indata, int inlen, void *outdata, int *outlen);

#if __cplusplus
}
#endif

#endif /* __BASE64_H__ */