#ifndef _TIMERECORDER_WIN32_H__
#define _TIMERECORDER_WIN32_H__
#include "TimeRecorder.h"

class TimeRecorderWin32 : public TimeRecorder {
private:
    std::vector<TString> m_runningApps;
    TString m_foreExe;
    TString m_preForeExe;

    bool ModeNone(TApp&);
    void ModeBack(TApp&);
    void ModeFore(TApp&);
    void InsertRecord(TApp&);
    bool IsProcessRunning(const TString&);
    void GetAllProcess();

    void Print();

public:
    TimeRecorderWin32(TString SqlFile);

    // override
    void exec() override;
    void Update() override;

    TTime RunTime(const TApp&);
};

#endif // !_TIMERECORDER_WIN32_H__