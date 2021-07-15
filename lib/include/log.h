#ifndef _LOG_H__
#define _LOG_H__
#ifdef WIN32
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#endif //! WIN32

#include "zqlTools.h"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>

static inline tm _nowTime() {
#ifndef WIN32
    time_t t = time(nullptr);
    tm* date = localtime(&t);
    return *date;
#else
    time_t t = time(nullptr);
    tm date;
    localtime_s(&date, &t);
    return date;
#endif
}

static inline void _mkdir_for_log(const char* str) {
#ifndef WIN32
    mkdir(str, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#else
    _mkdir(str);
#endif
}

static inline FILE* _file_open(const std::string& str) {
#ifdef WIN32
    FILE* fp;
    fopen_s(&fp, str.c_str(), "a+");
    return fp;
#else
    return fopen(str.c_str(), "a+");
#endif
}

namespace zql {
/***********************************************************************
模块名          : 日志类
文件名          : log.h
相关文件        :
文件实现功能    : 日志记录
作者            : zql
版本            : 1.0
----------------------------------------------------------------------
备注            :
LogStream：先调用 Head()，再通过其返回的引用使用 << 运算符进行输入
           [2021-05-28 13:54:10][0] This is a log.
Log：使用 LogStream 进行多文件的日志记录
----------------------------------------------------------------------
修改记录     :
日 期        版本   修改人                 修改内容
2021/5/31    1.0    zql                   创建内容
*********************************************************************/
class LogStream {
private:
    std::string m_fileName; // 包含路径的完整文件名
    int m_level;
    LogStream* m_subordinate; // level 低于 this->m_level 的所有 LogStream

public:
    LogStream(const char* fileName, int level = 0,
              LogStream* subordinate = nullptr)
        : m_level(level), m_subordinate(subordinate), m_fileName(fileName) {}

    // level == -1 表示使用 this->m_level
    LogStream& Head(int level = -1) {
        //判断文件最后一个字符是否是\n 不是则设置sign = true 以添加\n
        bool sign = false;
        std::ifstream file(m_fileName);
        file.seekg(-1, std::ios::end);
        char c = -1;
        file >> c;
        // c == -1表示是文件为空
        if (c != -1 && c != '\n')
            sign = true;
        file.close();

        // open
        FILE* fp = _file_open(m_fileName);
        // '\n'
        if (sign)
            fputc('\n', fp);
        // [16:45:52][0]
        tm date = _nowTime();
        fprintf(fp, "[%02d:%02d:%02d][%d] ", date.tm_hour, date.tm_min,
                date.tm_sec, level == -1 ? m_level : level);
        // close
        fclose(fp);

        // m_subordinate
        if (m_subordinate != nullptr)
            m_subordinate->Head(m_level);
        return *this;
    }

    LogStream& operator<<(const std::string& str) {
        FILE* fp = _file_open(m_fileName);

        if (zql::GetCoding(str) == GBK)
            fputs(zql::ANSIToUTF8(str).c_str(), fp);
        else
            fputs(str.c_str(), fp);

        fclose(fp);
        WriteSon(str);
        return *this;
    }

    LogStream& operator<<(const char c) {
        this->operator<<(std::string(1, c));
        return *this;
    }

    LogStream& operator<<(const char* str) {
        this->operator<<(std::string(str));
        return *this;
    }

    LogStream& operator<<(int val) {
        FILE* fp = _file_open(m_fileName);
        fprintf(fp, "%d", val);
        fclose(fp);
        WriteSon(val);
        return *this;
    }

    LogStream& operator<<(double val) {
        FILE* fp = _file_open(m_fileName);
        fprintf(fp, "%lf", val);
        fclose(fp);
        WriteSon(val);
        return *this;
    }

    LogStream& operator<<(const std::wstring& str) {
        FILE* fp = _file_open(m_fileName);
        fputs(zql::UnicodeToUTF8(str).c_str(), fp);
        fclose(fp);
        WriteSon(str);
        return *this;
    }

    LogStream& operator<<(const wchar_t c) {
        this->operator<<(std::wstring(1, c));
        return *this;
    }

    LogStream& operator<<(const wchar_t* str) {
        this->operator<<(std::wstring(str));
        return *this;
    }

    // every operator << needs this
    // run for m_subordinate
    template <class T> inline void WriteSon(T a) {
        if (m_subordinate != nullptr)
            *m_subordinate << a;
    }
};

class Log {
public:
    Log(std::string path, std::vector<std::string> files, int maxCount = 30)
        : m_path(path), m_files(files), m_maxCount(maxCount) {
        _mkdir_for_log(m_path.c_str());
        m_beginDay = -1;
        CheckDate();
        ClearFiles();
    }

    LogStream& operator()(int level) { return m_logs[level]->Head(); }

private:
    std::string m_path;               // 日志文件夹
    size_t m_maxCount;                // 每个类型的日志的最大数量
    std::vector<std::string> m_files; // 日志文件名前缀，及其
    std::vector<LogStream*> m_logs;   // 日志文件 LogStream 对象
    int m_beginDay; // 记录日志对象创建的日期，如果过去一天了，建立新的文件记录日志

    // 检查 m_beginDay
    void CheckDate() {
        tm date = _nowTime();
        if (m_beginDay != date.tm_mday) {
            if (m_beginDay == -1)
                for (int i = 0; i < m_files.size(); i++)
                    m_logs.push_back(new LogStream(
                        GenerateName(i).c_str(), i,
                        i == 0 ? nullptr : m_logs[(size_t)i - 1]));
            else {
                for (int i = 0; i < m_files.size(); i++) {
                    delete m_logs[i];
                    m_logs[i] =
                        new LogStream(GenerateName(i).c_str(), i,
                                      i == 0 ? nullptr : m_logs[(size_t)i - 1]);
                }
            }
            m_beginDay = date.tm_mday;
        }
    }

    std::string GenerateName(int level) {
        tm date = _nowTime();
        return m_path + '/' + m_files[level] + '_' +
               std::to_string(date.tm_year) + '_' +
               std::to_string(date.tm_mon) + '_' +
               std::to_string(date.tm_mday) + ".txt";
    }

private:
    // 辅助函数及结构体
    struct FileInfo {
        bool isLog;
        std::string prefix;
        unsigned short year;
        unsigned short month;
        unsigned short day;
        std::string fileName;

        FileInfo() : isLog(false), year(0), month(0), day(0) {}
    };

    // 根据文件创建时间进行清理
    void ClearFiles() {
        std::vector<FileInfo> allFiles;

#ifdef WIN32
        // 查找所有文件，并加入 allFiles 中
        struct _finddata_t c_file;
        intptr_t hFile;
        // Find first .txt file in log
        if ((hFile = _findfirst((m_path + "/*.txt").c_str(), &c_file)) != -1L) {
            do {
                FileInfo tmp = FileNameParse(c_file.name);
                if (tmp.isLog)
                    allFiles.push_back(tmp);
            } while (_findnext(hFile, &c_file) == 0);
            _findclose(hFile);
        }
#else
        DIR* dir = opendir(m_path.c_str());
        struct dirent* ptr;
        char base[1000];

        if (dir == NULL)
            return;

        while ((ptr = readdir(dir)) != NULL) {
            if (strcmp(ptr->d_name, ".") == 0 ||
                strcmp(ptr->d_name, "..") == 0) /// current dir OR parrent dir
                continue;
            else if (ptr->d_type == 8) {
                FileInfo tmp = FileNameParse(ptr->d_name);
                if (tmp.isLog)
                    allFiles.push_back(tmp);
            } else
                continue;
        }
        closedir(dir);
#endif

        sort(allFiles.begin(), allFiles.end(), fileCMP);
        // 删除小的文件夹，使文件夹总数为 m_maxCount
        for (int i = 0; i < (int)allFiles.size() - (int)m_maxCount; i++)
            remove((m_path + '/' + allFiles[i].fileName).c_str());
    }

    // 用于 ClearFiles() 的比较函数，升序排列，小的将会被删除
    static bool fileCMP(const FileInfo& a, const FileInfo& b) {
        if (a.year != b.year)
            return a.year < b.year;
        if (a.month != b.month)
            return a.month < b.month;
        if (a.day != b.day)
            return a.day < b.day;
        return a.fileName < b.fileName;
    }

    FileInfo FileNameParse(std::string str) {
        FileInfo ret;
        ret.isLog = false;
        ret.fileName = str;

        // 查找由 '_' 分割开的四个字符串
        std::string part[5];
        size_t begin, end = -1;
        for (int i = 0; i < 3; i++) {
            begin = end + 1;
            end = str.find('_', begin);
            if (end == std::string::npos)
                return ret;
            part[i] = str.substr(begin, end - begin);
        }
        begin = end + 1;
        part[3] = str.substr(begin, 2);
        part[4] = str.substr(begin + 2);
        if (part[4] != ".txt")
            return ret;
        if (part[1].size() != 4 || part[2].size() != 2 || part[3].size() != 2)
            return ret;

        // 对找到的四部分分别赋值
        ret.prefix = part[0];
        ret.year = atoi(part[1].c_str());
        ret.month = atoi(part[2].c_str());
        ret.day = atoi(part[3].c_str());
        // check
        if (ret.year < 1900)
            return ret;
        if (ret.month > 12)
            return ret;
        if (ret.day > 31)
            return ret;
        if (find(m_files.begin(), m_files.end(), ret.prefix) == m_files.end())
            return ret;
        ret.isLog = true;
        return ret;
    }
};

} // namespace zql
#endif // !_LOG_H__
