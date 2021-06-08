#ifndef _TIMERECORDER_SOL_FIRST_H__
#define _TIMERECORDER_SOL_FIRST_H__
#include "TimeRecorder.h"
#include <set>

class TimeRecorder_sol_first : public TimeRecorder::TimeRecorderBase {
private:
    using _string = TimeRecorder::_string;
    using _size = TimeRecorder::_size;
    using _time = TimeRecorder::_time;
    using _appData = TimeRecorder::_appData;
    using _listSQL = TimeRecorder::_listSQL;

private:
    void GetAllProcess();
    inline bool IsProcessRunning(const _string& name);
    inline bool GetExeName(const _size pid, _string& ret);
    inline _size ForeExePid();

    void Print();

protected:
    std::set<_string> m_allPro;
    _size m_frontPID, m_nowPID;

    void Update() override;
    _time RunTime(const _appData& target) override;

public:
    TimeRecorder_sol_first();
    virtual void exec() override;
};
#endif // !_TIMERECORDER_SOL_FIRST_H__