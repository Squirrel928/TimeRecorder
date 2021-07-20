#ifndef _TIMERECORDER_LOG_H__
#define _TIMERECORDER_LOG_H__
#include "TimeRecorderBase.h"

constexpr auto BUFFERSIZE_LOG = 4096;

class TimeRecorderLog : public TimeRecorderBase {
private:
    TString m_path;
    tm m_nowDate;
    std::string m_fileInfo;
    std::string m_fileError;

public:
    enum class LOGLEVEL { info, error };

    TimeRecorderLog(TString path);

    void WriteLog(LOGLEVEL level, TString content);
    void WriteLog(LOGLEVEL level, const wchar_t* format, ...);

    static TString Format(const wchar_t* format, ...);
    static std::string Format(const char* format, ...);

private:
    void Write(TString str);
    std::string FileName(LOGLEVEL level);
    const char* LevelDescribe(LOGLEVEL level);
    void Check();
};
#endif // !_TIMERECORDER_LOG_H__