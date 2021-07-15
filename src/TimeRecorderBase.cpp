#include "TimeRecorderBase.h"
#include <codecvt>

TimeRecorderBase::TimeRecorderBase() {}

TimeRecorderBase::~TimeRecorderBase() {}

std::string TimeRecorderBase::UnicodeToUTF8(const std::wstring& src) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> wcv;
    return wcv.to_bytes(src);
}

std::string TimeRecorderBase::UnicodeToANSI(const std::wstring& src) {
    return WideToMulti(src, CP_ACP);
}

std::wstring TimeRecorderBase::UTF8ToUnicode(const std::string& src) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> wcv;
    return wcv.from_bytes(src);
}

std::wstring TimeRecorderBase::ANSIToUnicode(const std::string& src) {
    return MultiToWide(src, CP_ACP);
}

std::string TimeRecorderBase::WideToMulti(const std::wstring& src,
                                          UINT codePage) {
    std::string res;
    res.resize(WideCharToMultiByte(codePage, 0, src.c_str(), (int)src.size(),
                                   nullptr, 0, nullptr, nullptr));
    WideCharToMultiByte(codePage, 0, src.c_str(), (int)src.size(), &res[0],
                        (int)res.size(), nullptr, nullptr);
    return res;
}

std::wstring TimeRecorderBase::MultiToWide(const std::string& src,
                                           UINT codePage) {
    std::wstring res;
    res.resize(MultiByteToWideChar(codePage, 0, src.c_str(), (int)src.size(),
                                   nullptr, 0));
    MultiByteToWideChar(codePage, 0, src.c_str(), (int)src.size(), &res[0],
                        (int)res.size());
    return res;
}