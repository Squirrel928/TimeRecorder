#ifndef _TIMERECORDER_SOL_SQLITE_H_
#define _TIMERECORDER_SOL_SQLITE_H_
#include "TimeRecorder.h"
#include <map>
#include <sqlite3.h>
#include <vector>

#define LOGFILE "log"
#define SQLFILE "Recorder.db"

class TimeRecorder_sql_sqlite : public TimeRecorder::SQL {
private:
    using _string = TimeRecorder::_string;
    using _listSQL = TimeRecorder::_listSQL;

    // 存储着从数据库获取的数据，第一个string是字段名，第二个为其数据
    using _vector = std::vector<_string>;
    using _map = std::map<_string, _vector>;

    using _sqlData = TimeRecorder::_sqlData;
    using _time = TimeRecorder::_time;

private:
    sqlite3* m_sqlite;
    _map m_sqlData;

    static int callback_app(void*, int, char**, char**);
    static int callback_sumTotalTime(void*, int, char**, char**);

    struct _temp_callback {
        TimeRecorder_sql_sqlite* sqlite;
        _listSQL* list;

        _temp_callback(TimeRecorder_sql_sqlite* sqlite, _listSQL* list) {
            this->sqlite = sqlite;
            this->list = list;
        }
    };

public:
    TimeRecorder_sql_sqlite();
    ~TimeRecorder_sql_sqlite();

    void getApplication(_listSQL& appData) override;
    bool sql_exe(_string, _string&) override;
};
#endif // !_TIMERECORDER_SOL_SQLITE_H_