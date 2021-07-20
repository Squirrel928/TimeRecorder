#include "TimeRecorderSQLite.h"
#include <stdarg.h>
constexpr auto BUFFERSIZE = 256;

TimeRecorderSQLite::TimeRecorderSQLite(TString sqlName, TString logName)
    : m_sqlite(nullptr), TimeRecorderSQL(sqlName, logName) {
    if (SetSQLFile(sqlName))
        m_log->WriteLog(TimeRecorderLog::LOGLEVEL::info,
                        L"Open the database file successfully.");
    else
        m_log->WriteLog(TimeRecorderLog::LOGLEVEL::error,
                        L"Open the database file fail.");

    if (GetAppData())
        m_log->WriteLog(TimeRecorderLog::LOGLEVEL::info,
                        L"Get applications info success");
    else
        m_log->WriteLog(TimeRecorderLog::LOGLEVEL::error,
                        L"Get applications info fail");
}

TimeRecorderSQLite::~TimeRecorderSQLite() {
    if (m_sqlite != nullptr && sqlite3_close(m_sqlite) != SQLITE_OK)
        m_log->WriteLog(TimeRecorderLog::LOGLEVEL::error,
                        L"Fail to close the database!");
}

bool TimeRecorderSQLite::SetSQLFile(TString fileName) {
    sqlite3* temp;
    if (sqlite3_open(UnicodeToUTF8(fileName).c_str(), &temp) != SQLITE_OK) {
        m_log->WriteLog(TimeRecorderLog::LOGLEVEL::error,
                        L"Failed to open database: " + fileName);
        return false;
    } else {
        if (m_sqlite != nullptr)
            sqlite3_close(m_sqlite);
        m_sqlite = temp;
        m_sqlName = fileName;
        return true;
    }
}

bool TimeRecorderSQLite::Sql_exe(const wchar_t* format, ...) {
    if (m_sqlite == nullptr) {
        m_log->WriteLog(TimeRecorderLog::LOGLEVEL::error,
                        L"Attempt to use an uninitialized database!");
        return false;
    }
    wchar_t buffer[BUFFERSIZE];
    // vswprintf_s
    va_list args;
    va_start(args, format);
    vswprintf_s(buffer, format, args);
    va_end(args);

    char* errmsg = NULL;
    m_log->WriteLog(TimeRecorderLog::LOGLEVEL::info,
                    L"Sql statement executed: " + TString(buffer));
    if (sqlite3_exec(m_sqlite, UnicodeToUTF8(buffer).c_str(), nullptr, nullptr,
                     &errmsg) != SQLITE_OK) {
        m_log->WriteLog(TimeRecorderLog::LOGLEVEL::error,
                        L"Sql statement execution error: " + TString(buffer) +
                            L", " + UTF8ToUnicode(errmsg));
        delete[] errmsg;
        return false;
    }
    return true;
}

bool TimeRecorderSQLite::GetAppData() {
    m_apps.clear();
    if (m_sqlite == NULL) {
        m_log->WriteLog(
            TimeRecorderLog::LOGLEVEL::error,
            L"Failed to get application information from database!");
        return false;
    }
    wchar_t buffer[BUFFERSIZE];
    char* errmsg = nullptr;
    // get app info
    swprintf_s(buffer, 128, L"SELECT %s, %s, %s, %s, %s FROM %s", TABLE_APP_ID,
               TABLE_APP_NAME, TABLE_APP_EXENAME, TABLE_APP_TYPEID,
               TABLE_APP_MODE, TABLE_APP);
    m_log->WriteLog(TimeRecorderLog::LOGLEVEL::info,
                    TString(L"Read app information from the database: ") +
                        buffer);
    if (sqlite3_exec(m_sqlite, UnicodeToUTF8(buffer).c_str(), SqlCall_GetApp,
                     &m_apps, &errmsg) != SQLITE_OK) {
        m_log->WriteLog(
            TimeRecorderLog::LOGLEVEL::error,
            TString(L"Fail to get application info from the database: ") +
                UTF8ToUnicode(errmsg));
        delete[] errmsg;
        errmsg = nullptr;
        return false;
    }
    // get record info
    for (auto i : m_apps) {
        swprintf_s(buffer, 128, L"SELECT %s, %s, %s FROM %s WHERE %s = %llu",
                   TABLE_REC_BEGIN, TABLE_REC_END, TABLE_REC_TOTAL, TABLE_REC,
                   TABLE_REC_ID, i.sqlId);
        if (sqlite3_exec(m_sqlite, UnicodeToUTF8(buffer).c_str(),
                         SqlCall_GetRecord, (void*)&i.records,
                         &errmsg) != SQLITE_OK) {
            m_log->WriteLog(TimeRecorderLog::LOGLEVEL::error,
                            TString(L"Fail to get application record "
                                    L"info from the database: ") +
                                UTF8ToUnicode(errmsg));
            delete[] errmsg;
            errmsg = nullptr;
            return false;
        }
    }
    // get type info
    m_types.clear();
    swprintf_s(buffer, 128, L"SELECT %s, %s, %s FROM %s", TABLE_TYPE_ID,
               TABLE_TYPE_NAME, TABLE_TYPE_SUPERTYPE, TABLE_TYPE);
    m_log->WriteLog(TimeRecorderLog::LOGLEVEL::info,
                    TString(L"Read type information from the database: ") +
                        buffer);
    if (sqlite3_exec(m_sqlite, UnicodeToUTF8(buffer).c_str(), SqlCall_GetType,
                     (void*)&m_types, &errmsg) != SQLITE_OK) {
        m_log->WriteLog(TimeRecorderLog::LOGLEVEL::error,
                        TString(L"Fail to get application type "
                                L"info from the database: ") +
                            UTF8ToUnicode(errmsg));
        delete[] errmsg;
        errmsg = nullptr;
        return false;
    }
    return true;
}

// The first parameter type TListApp
int TimeRecorderSQLite::SqlCall_GetApp(void* lp, int nColumn, char** colValues,
                                       char** colNames) {
    TApp temp;
    for (auto i = 0; i < nColumn; i++) {
        if (colNames[i] == UnicodeToUTF8(TABLE_APP_ID))
            temp.sqlId = std::strtoull(colValues[i], nullptr, 10);
        else if (colNames[i] == UnicodeToUTF8(TABLE_APP_NAME))
            temp.name = UTF8ToUnicode(colValues[i]);
        else if (colNames[i] == UnicodeToUTF8(TABLE_APP_EXENAME))
            temp.exeName = UTF8ToUnicode(colValues[i]);
        else if (colNames[i] == UnicodeToUTF8(TABLE_APP_TYPEID))
            temp.type = std::strtoull(colValues[i], nullptr, 10);
        else if (colNames[i] == UnicodeToUTF8(TABLE_APP_MODE))
            switch (std::atoi(colValues[i])) {
            case 1:
                temp.mode = TMODE::fore;
                break;
            case 2:
                temp.mode = TMODE::back;
                break;
            default:
                temp.mode = TMODE::fore;
                break;
            }
    }
    ((TListApp*)lp)->push_back(temp);
    return 0;
}

// The first parameter type TAppRecord
int TimeRecorderSQLite::SqlCall_GetRecord(void* lp, int nColumn,
                                          char** colValues, char** colNames) {
    TRecord rec;
    for (auto i = 0; i < nColumn; i++) {
        if (colNames[i] == UnicodeToUTF8(TABLE_REC_BEGIN))
            rec.begin = std::strtoull(colValues[i], nullptr, 10);
        else if (colNames[i] == UnicodeToUTF8(TABLE_REC_END))
            rec.end = std::strtoull(colValues[i], nullptr, 10);
        else if (colNames[i] == UnicodeToUTF8(TABLE_REC_TOTAL))
            rec.total = std::strtoull(colValues[i], nullptr, 10);
    }
    ((TAppRecord*)lp)->rec.push_back(rec);
    return 0;
}

// The first parameter type TMapType
int TimeRecorderSQLite::SqlCall_GetType(void* lp, int nColumn, char** colValues,
                                        char** colNames) {
    TType temp;
    for (auto i = 0; i < nColumn; i++) {
        if (colNames[i] == UnicodeToUTF8(TABLE_TYPE_ID))
            temp.id = std::strtoull(colValues[i], nullptr, 10);
        else if (colNames[i] == UnicodeToUTF8(TABLE_TYPE_NAME))
            temp.name = UTF8ToUnicode(colValues[i]);
        else if (colNames[i] == UnicodeToUTF8(TABLE_TYPE_SUPERTYPE))
            temp.supertype = colValues[i] == nullptr
                                 ? 0
                                 : std::strtoull(colValues[i], nullptr, 10);
    }
    ((TMapType*)lp)->insert({temp.id, temp});
    return 0;
}