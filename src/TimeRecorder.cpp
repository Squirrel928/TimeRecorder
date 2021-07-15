#include "TimeRecorder.h"

TimeRecorder::TimeRecorder(TimeRecorderLog* _log, TimeRecorderSQL* _sql) {
    m_log = _log;
    m_sql = _sql;

    m_log->WriteLog(LOGLEVEL::info, L"start time: " + DateToString(NowTime()));
}

TimeRecorder::~TimeRecorder() {
    m_log->WriteLog(LOGLEVEL::info, L"end time: " + DateToString(NowTime()));
}

TimeRecorder::TTime TimeRecorder::NowTime() { return time(nullptr); }

TimeRecorder::TString TimeRecorder::TimeToString(TTime source) {
    wchar_t buffer[30];
    int sec = source % 60;
    source /= 60;
    int min = source % 60;
    swprintf_s(buffer, 30, L"%02d:%02d:%02d", int(source / 60), min, sec);
    return buffer;
}

TimeRecorder::TString TimeRecorder::DateToString(TTime source) {
    wchar_t buffer[30];
    tm date;
    localtime_s(&date, &source);
    swprintf_s(buffer, 30, L"%04d/%02d/%02d %02d:%02d:%02d",
               date.tm_year + 1900, date.tm_mon + 1, date.tm_mday, date.tm_hour,
               date.tm_min, date.tm_sec);
    return buffer;
}