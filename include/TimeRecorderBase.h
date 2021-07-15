#ifndef _TIMERECORDER_BASE_H__
#define _TIMERECORDER_BASE_H__
#include <ctime>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>

class TimeRecorderBase {
public:
    TimeRecorderBase();
    virtual ~TimeRecorderBase();

    static std::string UnicodeToUTF8(const std::wstring&);
    static std::string UnicodeToANSI(const std::wstring&);
    static std::wstring UTF8ToUnicode(const std::string&);
    static std::wstring ANSIToUnicode(const std::string&);

public:
    // Every custom data type starts with T
    using TString = std::wstring;
    using TSize = unsigned long long;
    using TTime = time_t;

    enum class TMODE { none, fore, back };
    enum class LOGLEVEL { info, error };

    struct TType {
        TSize id;
        TString name;
        TSize supertype;

        TType() : id(0), supertype(0) {}
    };

    struct TRecord {
        TTime begin;
        TTime end;
        TTime total;

        TRecord() : begin(0), end(0), total(0) {}
        TRecord(TTime begin_, TTime end_, TTime total_)
            : begin(begin_), end(end_), total(total_) {}
    };

    struct TAppRecord {
        std::vector<TRecord> rec;
        TTime total;

        TAppRecord() : total(0) {}
        void Insert(TRecord _record) {
            total += _record.total;
            rec.push_back(_record);
        }
    };

    struct TApp {
        bool running;
        TRecord runTime;
        // sql data
        TSize sqlId;
        TMODE mode;
        TString name;
        TString exeName;
        TSize type;
        TAppRecord records;

        TApp() : running(false), sqlId(0), mode(TMODE::none), type(0) {}
    };

    using TMapType = std::unordered_map<TSize, TType>;
    using TListApp = std::vector<TApp>;

private:
    // use CP_ACP for ANSI or CP_UTF8
    static std::string WideToMulti(const std::wstring&, UINT);
    static std::wstring MultiToWide(const std::string&, UINT);
};
#endif // !_TIMERECORDER_BASE_H__
