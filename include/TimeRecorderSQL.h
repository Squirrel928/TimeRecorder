#ifndef _TIMERECORDER_SQL_H__
#define _TIMERECORDER_SQL_H__
#include "TimeRecorderBase.h"
#include "TimeRecorderLog.h"

// table name of application
constexpr auto TABLE_APP = L"application";
constexpr auto TABLE_APP_ID = L"app_id";
constexpr auto TABLE_APP_NAME = L"app_name";
constexpr auto TABLE_APP_EXENAME = L"app_exeName";
constexpr auto TABLE_APP_TYPEID = L"app_type_id";
constexpr auto TABLE_APP_MODE = L"app_mode";
// table name of record
constexpr auto TABLE_REC = L"record";
constexpr auto TABLE_REC_ID = L"app_id";
constexpr auto TABLE_REC_BEGIN = L"rec_begin";
constexpr auto TABLE_REC_END = L"rec_end";
constexpr auto TABLE_REC_TOTAL = L"rec_total";
// table name of type
constexpr auto TABLE_TYPE = L"type";
constexpr auto TABLE_TYPE_ID = L"type_id";
constexpr auto TABLE_TYPE_NAME = L"type_name";
constexpr auto TABLE_TYPE_SUPERTYPE = L"type_superType";

class TimeRecorderSQL : public TimeRecorderBase {
protected:
    TimeRecorderLog* m_log;
    TString m_sqlName;
    TString m_logName;

public:
    TListApp m_apps;
    TMapType m_types;

    TimeRecorderSQL(TString sqlName, TString logName);

    // application
    bool AddApplication(TString name, TString exeName, TSize typeID,
                        TMODE mode);
    bool ModifyApplication(TSize id, TString name, TString exeName,
                           TSize typeID, TMODE mode);
    bool DelApplication(TSize id);
    // record
    bool AddRecord(TSize id, TRecord record);
    bool DelRecord(TSize id, TRecord record);
    // type
    bool AddType(TString name, TSize parentID);
    bool ModifyType(TSize id, TString name, TSize parentID);
    bool DelType(TSize id);

    // Virtual function
    virtual bool SetSQLFile(TString fileName) = 0;
    // SQL statement without return value
    virtual bool Sql_exe(const wchar_t* format, ...) = 0;
    // return value type std::vector<TSqlData>
    virtual bool GetAppData() = 0;
};
#endif // !_TIMERECORDER_SQL_H__