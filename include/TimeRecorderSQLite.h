#ifndef _TIMERECORDER_SQLITE_H__
#define _TIMERECORDER_SQLITE_H__
#include "TimeRecorderSQL.h"
#include "sqlite3.h"

class TimeRecorderSQLite : public TimeRecorderSQL {
private:
    sqlite3* m_sqlite;

    // Auxiliary function
    static int SqlCall_GetApp(void*, int, char**, char**);
    static int SqlCall_GetRecord(void*, int, char**, char**);
    static int SqlCall_GetType(void*, int, char**, char**);

public:
    TimeRecorderSQLite(TString sqlName, TString logName);
    virtual ~TimeRecorderSQLite();

    // override
    bool SetSQLFile(TString) override;
    bool Sql_exe(const wchar_t*, ...) override;
    bool GetAppData() override;
};
#endif // !_TIMERECORDER_SQLITE_H__