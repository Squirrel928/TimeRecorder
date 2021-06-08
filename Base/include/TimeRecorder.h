#ifndef _TIMERECORDER_BASE_H__
#define _TIMERECORDER_BASE_H__
// zql tools
#include <zqlTools.h>

#include <log.h>

#include <codecvt>
#include <ctime>
#include <string>
#include <vector>

namespace TimeRecorder {

extern zql::Log log;

using _string = std::wstring;
using _size = unsigned long long;
using _Log = zql::Log;
using _time = time_t;

struct _sqlData {
    _size sqlId;
    _string name;
    _string exeName;
    _time totalTime;
};

struct _appData {
    _size pid;
    _time begin, front, total;
    _sqlData sqlData;
    bool inSql;
    // if insql is running?
    bool running;
};

using _listSQL = std::vector<_sqlData>;
using _listApp = std::vector<_appData>;

_string TimeToString(time_t, bool hasYearMonthDay = false);
_time StringToTime(_string, bool hasYearMonthDay = false);

// time recorder sql
class SQL {
protected:
    // 第一个参数为要执行的 sql 语句，第二个参数为执行错误时的返回值
    virtual bool sql_exe(_string, _string&) = 0;

public:
    void addApplication(_string name, _string exeName, int mode);
    void addRecord(_size id, _time begin, _time end, _time total);

    // pure virtual function
public:
    virtual void getApplication(_listSQL& m_appData) = 0;
};

// time recorder
class TimeRecorderBase {
protected:
    _listApp m_runPro;
    SQL* m_sql;

    _time GetTime() { return time(NULL); }
    void AddApplication(_string name, _string exeName);
    void AddRecord(_size id, _time begin, _time end, _time total);
    _string TotalTime(); // 返回所有应用的运行时间
    void Sort();         // 排序以按照顺序显示所有信息

    static bool cmp(const _appData& a, const _appData& b);

    // pure virtual function
protected:
    // 此函数负责监控程序运行，并把要显示的应用更新到 m_runPro
    virtual void Update() = 0;
    // 根据_appData 获取程序运行时间
    virtual inline _time RunTime(const _appData& target) = 0;

public:
    TimeRecorderBase() {
        TimeRecorder::log(0)
            << "Running time: " << TimeRecorder::TimeToString(time(NULL), true);
    }

    ~TimeRecorderBase() {
        TimeRecorder::log(0)
            << "Endding time: " << TimeRecorder::TimeToString(time(NULL), true);
    }

    // 此函数用于运行，并显示程序运行状况
    virtual void exec() = 0;
};
} // namespace TimeRecorder

#endif // !_TIMERECORDER_BASE_H__
