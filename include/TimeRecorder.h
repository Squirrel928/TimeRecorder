#ifndef _TIMERECORDER_H__
#define _TIMERECORDER_H__
#include "TimeRecorderBase.h"
#include "TimeRecorderLog.h"
#include "TimeRecorderSQL.h"

constexpr auto FileName_SQL =
    L"C:/Home/GitProject/TimeRecorderNewSol/TimeRecorder/Recorder.db";
constexpr auto FileName_Log =
    L"C:/Home/GitProject/TimeRecorderNewSol/TimeRecorder/testlog";

class TimeRecorder : public TimeRecorderBase {
protected:
    TimeRecorderLog* m_log;
    TimeRecorderSQL* m_sql;
    TListApp m_apps;

public:
    TimeRecorder(TimeRecorderLog*, TimeRecorderSQL*);
    virtual ~TimeRecorder();

    virtual void exec() = 0;
    virtual void Update() = 0;
    TTime NowTime();
    TString TimeToString(TTime);
    TString DateToString(TTime);
};

#endif // !_TIMERECORDER_H__