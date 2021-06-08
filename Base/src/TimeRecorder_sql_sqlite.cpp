#include "TimeRecorder_sql_sqlite.h"

TimeRecorder_sql_sqlite::TimeRecorder_sql_sqlite() {
    if (sqlite3_open(SQLFILE, &m_sqlite) != SQLITE_OK) {
        TimeRecorder::log(1) << "Fail to initialization the database!";
        m_sqlite = NULL;
    }
}

TimeRecorder_sql_sqlite::~TimeRecorder_sql_sqlite() {
    if (m_sqlite != NULL && sqlite3_close(m_sqlite) == SQLITE_BUSY)
        TimeRecorder::log(1) << "Fail to close the database!";
}

bool TimeRecorder_sql_sqlite::sql_exe(_string sql, _string& error) {
    if (m_sqlite == NULL) {
        error = L"Attempt to use an uninitialized database!";
        return false;
    }
    char* errmsg = NULL;
    if (sqlite3_exec(m_sqlite, zql::UnicodeToUTF8(sql).c_str(), NULL, NULL,
                     &errmsg) != SQLITE_OK) {
        error = zql::UTF8ToUnicode(errmsg);
        delete[] errmsg;
        return false;
    }
    return true;
}

void TimeRecorder_sql_sqlite::getApplication(_listSQL& appData) {
    if (m_sqlite == NULL)
        return;
    appData.clear();
    std::string sql = "SELECT app_id, app_name, app_exeName, app_type_id, "
                      "app_mode FROM application";
    TimeRecorder::log(0) << "Read app information from the database: " << sql;
    char* errmsg = NULL;
    _temp_callback temp_call(this, &appData);
    if (sqlite3_exec(m_sqlite, sql.c_str(), callback_app, (void*)&temp_call,
                     &errmsg) != SQLITE_OK) {
        TimeRecorder::log(1)
            << "Fail to get application info from the database: " << errmsg;
        delete[] errmsg;
    }
}

int TimeRecorder_sql_sqlite::callback_sumTotalTime(void* lp, int nColumn,
                                                   char** colValues,
                                                   char** colNames) {
    _time* sum = (_time*)lp;
    if (strcmp(colNames[0], "SUM(rec_total)") == 0 && colValues[0] != NULL)
        *sum = std::atoi(colValues[0]);
    else {
        *sum = 0;
    }
    return 0;
}

// 此函数每次返回查询的一行
// 第一个参数是通过 sqlite3_exec 传递过来的
// 第二个参数 nColumn 列数，及后两个参数的字符串的个数
int TimeRecorder_sql_sqlite::callback_app(void* lp, int nColumn,
                                          char** colValues, char** colNames) {
    _temp_callback* temp_call = (_temp_callback*)lp;
    TimeRecorder_sql_sqlite* sqlite = temp_call->sqlite;

    if (sqlite->m_sqlData.size() == 0) {
        for (auto i = 0; i < nColumn; i++)
            sqlite->m_sqlData.insert({zql::UTF8ToUnicode(colNames[i]), {}});
    }
    _sqlData temp;
    for (auto i = 0; i < nColumn; i++) {
        if (strcmp(colNames[i], "app_id") == 0)
            temp.sqlId = std::atoi(colValues[i]);
        else if (strcmp(colNames[i], "app_name") == 0)
            temp.name = zql::UTF8ToUnicode(colValues[i]);
        else if (strcmp(colNames[i], "app_exeName") == 0)
            temp.exeName = zql::UTF8ToUnicode(colValues[i]);
    }
    std::string sql = "SELECT SUM(rec_total) FROM record WHERE app_id == " +
                      std::to_string(temp.sqlId);
    char* errmsg = NULL;
    if (sqlite3_exec(sqlite->m_sqlite, sql.c_str(), callback_sumTotalTime,
                     (void*)(&temp.totalTime), &errmsg) != SQLITE_OK) {
        TimeRecorder::log(1)
            << "Fail to get application total time from the database: "
            << errmsg;
        delete[] errmsg;
        return 0;
    }

    temp_call->list->push_back(temp);
    return 0;
}