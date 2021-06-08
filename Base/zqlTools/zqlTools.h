#ifndef _ZQL_TOOLS_H__
#define _ZQL_TOOLS_H__
#include <codecvt>
#include <locale>
#include <string>

#ifdef WIN32
#include <windows.h>
#else
#include <iconv.h>
#endif

namespace zql {
/***********************************************************************
模块名          : 编码转换
文件名          : zqlTools.h
相关文件        :
文件实现功能    : UTF8 UTF16 ANSI 编码互相转换
作者            : zql
版本            : 1.0
----------------------------------------------------------------------
备注            : 参考资料：https://www.cnblogs.com/lidabo/p/10482864.html
                           https://blog.csdn.net/flushhip/article/details/82836867
UTF8 GB2312 实际都是多字节字符集
UTF16 为宽字节字符集，很多时候我们把 Unicode 与 UTF16等价
UTF8 UTF16 实际都是 Unicode 编码的实现
区别在于，分别使用多字节和宽字节两种方式实现的
多字节：英文用一个字节表示，中文用两个或三个字节表示
宽字节：所有字符都用两个字节表示
----------------------------------------------------------------------
修改记录     :
日 期        版本   修改人                 修改内容
2021/4/17    1.0    zql                     创建
2021/5/30    2.0    zql          修改宽字节与多字节之间的转换方法
2021/6/01    2.1    zql    增加Linux支持，linux下只有 UTF8 和 UTF16 之间的转换
2021/6/08    2.2    zql                   细节修改
*********************************************************************/
#ifdef WIN32
// use CP_ACP for ANSI or CP_UTF8
inline std::string WideToMulti(const std::wstring& src, UINT codePage) {
    std::string res;
    res.resize(WideCharToMultiByte(codePage, 0, src.c_str(), (int)src.size(),
                                   nullptr, 0, nullptr, nullptr));
    WideCharToMultiByte(codePage, 0, src.c_str(), (int)src.size(), &res[0],
                        (int)res.size(), nullptr, nullptr);
    return res;
}

// use CP_ACP for ANSI or CP_UTF8
inline std::wstring MultiToWide(const std::string& src, UINT codePage) {
    std::wstring res;
    res.resize(MultiByteToWideChar(codePage, 0, src.c_str(), (int)src.size(),
                                   nullptr, 0));
    MultiByteToWideChar(codePage, 0, src.c_str(), (int)src.size(), &res[0],
                        (int)res.size());
    return res;
}
#endif // !WIN32

inline std::string UnicodeToUTF8(const std::wstring& src) {
#ifdef WIN32
    return WideToMulti(src, CP_UTF8);
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> wcv;
    return wcv.to_bytes(src);
#endif
}

inline std::wstring UTF8ToUnicode(const std::string& src) {
#ifdef WIN32
    return MultiToWide(src, CP_UTF8);
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> wcv;
    return wcv.from_bytes(src);
#endif
}

inline std::string UnicodeToANSI(const std::wstring& src) {
#ifdef WIN32
    return WideToMulti(src, CP_ACP);
#else
    return UnicodeToUTF8(src);
#endif
}

inline std::wstring ANSIToUnicode(const std::string& src) {
#ifdef WIN32
    return MultiToWide(src, CP_ACP);
#else
    return UTF8ToUnicode(src);
#endif
}

inline std::string UTF8ToANSI(const std::string& src) {
#ifdef WIN32
    return UnicodeToANSI(MultiToWide(src, CP_UTF8));
#else
    return src;
#endif
}

inline std::string ANSIToUTF8(const std::string& src) {
#ifdef WIN32
    return WideToMulti(ANSIToUnicode(src), CP_UTF8);
#else
    return src;
#endif
}

/***********************************************************************
模块名          : 编码判断
文件名          : zqlTools.h
相关文件        :
文件实现功能    : UTF8 UTF16 ANSI 编码判断
作者            : zql
版本            : 1.0
----------------------------------------------------------------------
备注            :
参考资料：https://blog.csdn.net/weixin_38595946/article/details/103128955
----------------------------------------------------------------------
修改记录     :
日 期        版本   修改人                 修改内容
2021/5/30    1.0    zql                     创建
*********************************************************************/
inline int preNum(const char byte) {
    unsigned char mask = 0x80;
    int num = 0;
    for (int i = 0; i < 8; i++) {
        if ((byte & mask) == mask) {
            mask = mask >> 1;
            num++;
        } else {
            break;
        }
    }
    return num;
}

inline bool isUtf8(const char* data, size_t len) {
    size_t num = 0;
    size_t i = 0;
    while (i < len) {
        if ((data[i] & 0x80) == 0x00) {
            // 0XXX_XXXX
            i++;
            continue;
        } else if ((num = preNum(data[i])) > 2) {
            // 110X_XXXX 10XX_XXXX
            // 1110_XXXX 10XX_XXXX 10XX_XXXX
            // 1111_0XXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
            // 1111_10XX 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
            // 1111_110X 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
            // preNUm()
            // 返回首个字节8个bits中首�?0bit前面1bit的个数，该数量也是该字符所使用的字节数
            i++;
            for (size_t j = 0; j < num - 1; j++) {
                //判断后面num - 1 个字节是不是都是10开
                if ((data[i] & 0xc0) != 0x80) {
                    return false;
                }
                i++;
            }
        } else {
            //其他情况说明不是utf-8
            return false;
        }
    }
    return true;
}

//需要说明的是，isGBK()是通过双字节是否落在gbk的编码范围内实现的，
//而utf-8编码格式的每个字节都是落在gbk的编码范围内
//所以只有先调用isUtf8()先判断不是utf-8编码，再调用isGBK()才有意义
inline bool isGBK(const char* data, size_t len) {
    size_t i = 0;
    while (i < len) {
        if (data[i] <= 0x7f) {
            //编码小于等于127,只有一个字节的编码，兼容ASCII
            i++;
            continue;
        } else {
            //大于127的使用双字节编码
            if (data[i] >= 0x81 && data[i] <= 0xfe && data[i + 1] >= 0x40 &&
                data[i + 1] <= 0xfe && data[i + 1] != 0xf7) {
                i += 2;
                continue;
            } else {
                return false;
            }
        }
    }
    return true;
}

enum CODING { GBK, UTF8, UNKOWN };

inline CODING GetCoding(const std::string str) {
    CODING coding;
    if (isUtf8(str.c_str(), str.size()) == true) {
        coding = UTF8;
    } else if (isGBK(str.c_str(), str.size()) == true) {
        coding = GBK;
    } else {
        coding = UNKOWN;
    }
    return coding;
}
} // namespace zql
#endif // !_ZQL_TOOLS_H__