#include "TimeRecorder.h"
#include <algorithm>

using namespace TimeRecorder;

// table name of application
#define TABEL_APP L"application"
#define TABEL_APP_ID L"app_id"
#define TABEL_APP_NAME L"app_name"
#define TABEL_APP_EXENAME L"app_exeName"
#define TABEL_APP_MODE L"app_mode"
// table name of record
#define TABLE_REC L"record"
#define TABLE_REC_ID L"app_id"
#define TABLE_REC_BEGIN L"rec_begin"
#define TABLE_REC_END L"rec_end"
#define TABLE_REC_TOTAL L"rec_total"

// time recorder
zql::Log TimeRecorder::log("log", {"info", "error"});

_string TimeRecorder::TimeToString(time_t source, bool hasYearMonthDay) {
    wchar_t* buffer = new wchar_t[30];
    _string ret;

    if (hasYearMonthDay) {
        tm date;
        localtime_s(&date, &source);

        swprintf_s(buffer, 30, L"%04d/%02d/%02d %02d:%02d:%02d",
                   date.tm_year + 1900, date.tm_mon + 1, date.tm_mday,
                   date.tm_hour, date.tm_min, date.tm_sec);
    } else {
        int sec = source % 60;
        source /= 60;
        int min = source % 60;

        swprintf_s(buffer, 30, L"%02d:%02d:%02d", int(source / 60), min, sec);
    }
    ret = buffer;
    delete[] buffer;
    return ret;
}

_time TimeRecorder::StringToTime(_string source, bool hasYearMonthDay) {
    tm date;

    if (hasYearMonthDay) {
        swscanf_s(source.c_str(), L"%04d/%02d/%02d %02d:%02d:%02d",
                  &date.tm_year, &date.tm_mon, &date.tm_mday, &date.tm_hour,
                  &date.tm_min, &date.tm_sec);
        date.tm_year -= 1900;
        date.tm_mon--;

        return mktime(&date);
    } else {
        swscanf_s(source.c_str(), L"%02d:%02d:%02d", &date.tm_hour,
                  &date.tm_min, &date.tm_sec);
        _time tmp = date.tm_min;
        tmp += (_time)date.tm_hour * (_time)24;
        tmp = tmp * 60 + date.tm_sec;
        return tmp;
    }
}

// sql
void SQL::addApplication(_string name, _string exeName, int mode) {
    wchar_t* buffer = new wchar_t[128];
    swprintf_s(buffer, 128, L"INSERT INTO %s (%s, %s, %s) VALUES (%s, %s, %d);",
               TABEL_APP, TABEL_APP_NAME, TABEL_APP_EXENAME, TABEL_APP_MODE,
               name.c_str(), exeName.c_str(), (int)mode);
    log(0) << "insert an application in sql: " << buffer;
    _string error;
    if (!sql_exe(buffer, error))
        log(1) << "Fail to insert application to the database: " << buffer;
    delete[] buffer;
}

void SQL::addRecord(_size id, _time begin, _time end, _time total) {
    wchar_t* buffer = new wchar_t[128];
    swprintf_s(
        buffer, 128,
        L"INSERT INTO %s (%s, %s, %s, %s) VALUES (%lld, %lld, %lld, %lld);",
        TABLE_REC, TABLE_REC_ID, TABLE_REC_BEGIN, TABLE_REC_END,
        TABLE_REC_TOTAL, id, begin, end, total);
    log(0) << "insert a record in sql: " << buffer;
    _string error;
    if (!sql_exe(buffer, error))
        log(1) << "Fail to insert recorde to the database: " << error;
    delete[] buffer;
}

// time recorder base
_string TimeRecorderBase::TotalTime() {
    _time sum = 0;
    for (auto i : m_runPro)
        if (i.running)
            sum += RunTime(i);
    return TimeRecorder::TimeToString(sum);
}

void TimeRecorderBase::AddApplication(_string name, _string exeName) {
    m_sql->addApplication(name, exeName, 1);
}

void TimeRecorderBase::AddRecord(_size id, _time begin, _time end,
                                 _time total) {
    m_sql->addRecord(id, begin, end, total);
}

bool TimeRecorderBase::cmp(const _appData& a, const _appData& b) {
    // 第一优先级 running
    if (a.running != b.running)
        return a.running == true;
    // 第二优先级 in sql
    if (a.inSql != b.inSql)
        return a.inSql == true;
    // 第三优先级 totalTime

    // 1. a b 都 running == true
    if (a.running) {
        if (a.total > b.total)
            return true;
        else if (a.total < b.total)
            return false;
    }
    // else
    if (a.inSql)
        return a.sqlData.name >= b.sqlData.name;
    else
        return a.sqlData.exeName >= b.sqlData.exeName;
}

void TimeRecorderBase::Sort() { sort(m_runPro.begin(), m_runPro.end(), cmp); }
