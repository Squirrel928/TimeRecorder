#include "TimeRecorderLog.h"
#include "stdio.h"
#include <fstream>

TimeRecorderLog::TimeRecorderLog(TString path) : m_path(path) { Check(); }

void TimeRecorderLog::WriteLog(LOGLEVEL level, TString content) {
    Check();

    auto fun = [level, this, content](std::string& fn) {
        std::ofstream ofile(fn.c_str());
        // [16:45:52][0]
        ofile << Format("[%02d:%02d:%02d][%d]", m_nowDate.tm_hour,
                        m_nowDate.tm_min, m_nowDate.tm_sec,
                        LevelDescribe(level))
              << UnicodeToANSI(content) << '\n';
        ofile.close();
    };

    // error 也显示在 info 里，故不写 break
    switch (level) {
    case LOGLEVEL::error:
        fun(m_fileError);
    case LOGLEVEL::info:
        fun(m_fileInfo);
        break;
    }
}

void TimeRecorderLog::WriteLog(LOGLEVEL level, const wchar_t* format, ...) {
    wchar_t buffer[BUFFERSIZE_LOG];
    // vswprintf_s
    va_list args;
    va_start(args, format);
    vswprintf_s(buffer, format, args);
    va_end(args);

    WriteLog(level, (TString)buffer);
}

TimeRecorderLog::TString TimeRecorderLog::Format(const wchar_t* format, ...) {
    wchar_t buffer[BUFFERSIZE_LOG];
    // vswprintf_s
    va_list args;
    va_start(args, format);
    vswprintf_s(buffer, format, args);
    va_end(args);

    return buffer;
}

std::string TimeRecorderLog::Format(const char* format, ...) {
    char buffer[BUFFERSIZE_LOG];
    // vswprintf_s
    va_list args;
    va_start(args, format);
    vsprintf_s(buffer, format, args);
    va_end(args);

    return buffer;
}

const char* TimeRecorderLog::LevelDescribe(LOGLEVEL level) {
    switch (level) {
    case LOGLEVEL::info:
        return "info";
    case LOGLEVEL::error:
        return "error";
    default:
        return "NULL";
    }
}

void TimeRecorderLog::Check() {
    time_t t = time(nullptr);
    tm date;
    localtime_s(&date, &t);

    if (m_nowDate.tm_year != date.tm_year || m_nowDate.tm_mon != date.tm_mon ||
        m_nowDate.tm_mday != date.tm_mday) {
        m_nowDate = date;

        // 修改 fileName 并使文件从一个新行开始
        auto fun = [this](std::string& fn, LOGLEVEL level) {
            fn = Format("%s/%s_%04d_%02d_%02d.txt",
                        UnicodeToANSI(m_path).c_str(), LevelDescribe(level),
                        m_nowDate.tm_year + 1900, m_nowDate.tm_mon + 1,
                        m_nowDate.tm_mday);

            // 判断文件最后一个字符是否是 \n 不是则添加\n
            // 如果是空文件则不用添加换行
            std::fstream file(fn.c_str(), std::ios::app);
            file.seekg(-1, std::ios::end);
            char c = -1;
            file >> c;
            // c == -1表示是文件为空
            if (c != -1 && c != '\n')
                file << '\n';
            file.close();
        };

        fun(m_fileInfo, LOGLEVEL::info);
        fun(m_fileError, LOGLEVEL::error);
    }
}