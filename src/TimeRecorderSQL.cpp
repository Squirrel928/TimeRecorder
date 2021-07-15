#include "TimeRecorderSQL.h"

TimeRecorderSQL::TimeRecorderSQL(TString sqlName, TString logName)
    : m_log(new TimeRecorderLog(logName)) {}

bool TimeRecorderSQL::AddApplication(TString name, TString exeName,
                                     TSize typeID, TMODE mode) {
    return Sql_exe(
        L"INSERT INTO %s (%s, %s, %s, %s) VALUES ('%s', '%s', %s, %d);",
        TABLE_APP, TABLE_APP_NAME, TABLE_APP_EXENAME, TABLE_APP_TYPEID,
        TABLE_APP_MODE, name.c_str(), exeName.c_str(),
        typeID == 0 ? L"NULL" : std::to_wstring(typeID).c_str(), (int)mode);
}

bool TimeRecorderSQL::ModifyApplication(TSize id, TString name, TString exeName,
                                        TSize typeID, TMODE mode) {
    return Sql_exe(
        L"UPDATE %s SET %s = '%s', %s = '%s', %s = %s, %s = %d WHERE "
        L"%s = %llu",
        TABLE_APP, TABLE_APP_NAME, name.c_str(), TABLE_APP_EXENAME,
        exeName.c_str(), TABLE_APP_TYPEID,
        typeID == 0 ? L"NULL" : std::to_wstring(typeID).c_str(), TABLE_APP_MODE,
        (int)mode, TABLE_APP_ID, id);
}

bool TimeRecorderSQL::DelApplication(TSize sqlId) {
    return Sql_exe(L"DELETE FROM %s WHERE %s = %llu;", TABLE_APP, TABLE_APP_ID,
                   sqlId);
}

bool TimeRecorderSQL::AddRecord(TSize sqlId, TRecord record) {
    return Sql_exe(
        L"INSERT INTO %s (%s, %s, %s, %s) VALUES (%llu, %llu, %llu, %llu);",
        TABLE_REC, TABLE_REC_ID, TABLE_REC_BEGIN, TABLE_REC_END,
        TABLE_REC_TOTAL, sqlId, record.begin, record.end, record.total);
}

bool TimeRecorderSQL::DelRecord(TSize sqlId, TRecord record) {
    return Sql_exe(
        L"DELETE FROM %s WHERE %s = %llu AND %s = %llu AND %s = %llu "
        L"AND %s = %llu;",
        TABLE_REC, TABLE_REC_ID, sqlId, TABLE_REC_BEGIN, record.begin,
        TABLE_REC_END, record.end, TABLE_REC_TOTAL, record.total);
}

bool TimeRecorderSQL::AddType(TString typeName, TSize parentID) {
    return Sql_exe(L"INSERT INTO %s (%s, %s) VALUES ('%s', %s);", TABLE_TYPE,
                   TABLE_TYPE_NAME, TABLE_TYPE_SUPERTYPE, typeName.c_str(),
                   parentID == 0 ? L"NULL" : std::to_wstring(parentID).c_str());
}

bool TimeRecorderSQL::ModifyType(TSize typeId, TString typeName,
                                 TSize parentID) {
    return Sql_exe(L"UPDATE %s SET %s = '%s', %s = %s WHERE %s = %llu",
                   TABLE_TYPE, TABLE_TYPE_NAME, typeName.c_str(),
                   TABLE_TYPE_SUPERTYPE,
                   parentID == 0 ? L"NULL" : std::to_wstring(parentID).c_str(),
                   TABLE_TYPE_ID, typeId);
}

bool TimeRecorderSQL::DelType(TSize typeId) {
    return Sql_exe(L"DELETE FROM %s WHERE %s = %llu", TABLE_TYPE, TABLE_TYPE_ID,
                   typeId);
}