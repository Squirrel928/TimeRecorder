#ifndef _TIMERECORDER_LOG_H__
#define _TIMERECORDER_LOG_H__
#include "TimeRecorderBase.h"
#include "log.h"

class TimeRecorderLog : public TimeRecorderBase {
private:
    zql::Log m_log;

public:
    TimeRecorderLog(TString fileName);

    void WriteLog(LOGLEVEL level, TString content);
};

#endif // !_TIMERECORDER_LOG_H__