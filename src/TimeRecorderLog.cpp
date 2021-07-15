#include "TimeRecorderLog.h"
#include "stdio.h"

TimeRecorderLog::TimeRecorderLog(TString fileName)
    : m_log(TimeRecorderBase::UnicodeToANSI(fileName).c_str(),
            {"info", "error"}) {}

void TimeRecorderLog::WriteLog(LOGLEVEL level, TString content) {
    switch (level) {
    case LOGLEVEL::info:
        m_log(0) << content;
        break;

    case LOGLEVEL::error:
        m_log(1) << content;
    default:
        break;
    }
}