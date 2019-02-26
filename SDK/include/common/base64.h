/****************************************************************************
Copyright (c) 2017-2019 ShenZhen Red Bird Network Technology Co.,Ltd

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/
#ifndef HN_BASE64_H
#define HN_BASE64_H
#include <string>

namespace HN
{

    size_t         base64Encode(const char *src, size_t len, char *dst);
    size_t         base64Encode(const char *src, size_t len, std::string& dst);
    size_t         base64Encode(const std::string& src, std::string& dst);
    std::string    base64Encode(const char *src, size_t len);
    std::string    base64Encode(const std::string& src);

    size_t         base64Decode(const char *src, size_t len, char *dst);
    size_t         base64Decode(const char *src, size_t len, std::string& dst);
    size_t         base64Decode(const std::string& src, std::string& dst);
    std::string    base64Decode(const char *src, size_t len);
    std::string    base64Decode(const std::string& src);
}

#endif  /* HN_BASE64_H */
