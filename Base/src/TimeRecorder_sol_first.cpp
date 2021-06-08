#include "TimeRecorder_sol_first.h"
#include "TimeRecorder_sql_sqlite.h"
#include <Windows.h>

// zql tools
#include <table.h>

#include <Psapi.h>
#include <TlHelp32.h>
#include <iostream>
#include <locale>

TimeRecorder_sol_first::TimeRecorder_sol_first() {
    m_frontPID = 0;
    m_nowPID = 0;
    m_sql = new TimeRecorder_sql_sqlite;

    // 从数据库中初始化
    _listSQL list;
    m_sql->getApplication(list);
    _appData temp;
    temp.inSql = true;
    temp.running = false;
    for (auto& i : list) {
        temp.sqlData = i;
        m_runPro.push_back(temp);
    }
    return;
}

void TimeRecorder_sol_first::exec() {
    std::wcout.imbue(std::locale("chs"));
    // 不使用 cls 刷新屏幕以实现防闪屏
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    // 获取光标位置
    CONSOLE_SCREEN_BUFFER_INFO bInfo;
    GetConsoleScreenBufferInfo(hOutput, &bInfo);
    //隐藏缓冲区的光标
    CONSOLE_CURSOR_INFO cci;
    cci.bVisible = 0;
    cci.dwSize = 1;
    SetConsoleCursorInfo(hOutput, &cci);
    while (true) {
        Update();
        Sort();
        SetConsoleCursorPosition(hOutput, bInfo.dwCursorPosition);
        Print();
        Sleep(1000);
    }
}

void TimeRecorder_sol_first::Update() {
    m_nowPID = ForeExePid();
    if (m_nowPID == m_frontPID) {
        return;
    } else {
        _time nowTime = GetTime();
        GetAllProcess();
        // frontPID != nowPID 的情况下，是否在 proList 中找到 nowPID
        int sign = false;
        for (auto i = m_runPro.begin(); i != m_runPro.end();) {
            if (i->inSql) {
                if (i->running) {
                    if (i->pid == m_frontPID) {
                        i->total += nowTime - i->front;
                    } else if (i->pid == m_nowPID) {
                        i->front = nowTime;
                        sign = true;
                    } else if (!IsProcessRunning(i->sqlData.exeName)) {
                        i->total += nowTime - i->front;
                        m_sql->addRecord(i->sqlData.sqlId, i->begin, nowTime,
                                         i->total);
                        i->sqlData.totalTime += i->total;
                        i->running = false;
                    }
                } else {
                    _string str;
                    GetExeName(m_nowPID, str);

                    if (i->sqlData.exeName == str) {
                        i->running = true;
                        i->pid = m_nowPID;
                        i->front = i->begin = nowTime;
                        i->total = 0;
                        sign = true;
                    }
                }
            } else {
                if (i->pid == m_frontPID) {
                    i->total += nowTime - i->front;
                } else if (i->pid == m_nowPID) {
                    i->front = nowTime;
                    sign = true;
                } else if (!IsProcessRunning(i->sqlData.exeName)) {
                    i = m_runPro.erase(i);
                    continue;
                }
            }
            i++;
        }

        if (!sign) {
            _appData t;
            if (GetExeName(m_nowPID, t.sqlData.exeName)) {
                t.inSql = false;
                t.pid = m_nowPID;
                t.front = t.begin = nowTime;
                t.total = 0;
                t.running = true;
                m_runPro.push_back(t);
            }
        }
        m_frontPID = m_nowPID;
    }
}

void TimeRecorder_sol_first::GetAllProcess() {
    m_allPro.clear();
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        TimeRecorder::log(1) << "Failed to obtain system snapshot.";
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        TimeRecorder::log(1) << "Failed to obtain system process information.";
        CloseHandle(hProcessSnap);
        return;
    }

    do {
        m_allPro.insert(_string(pe32.szExeFile));
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return;
}

bool TimeRecorder_sol_first::IsProcessRunning(const _string& name) {
    return m_allPro.find(name) != m_allPro.end();
}

bool TimeRecorder_sol_first::GetExeName(const _size pid, _string& ret) {
    HANDLE hProcess =
        OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, (DWORD)pid);
    //获取进程路径
    LPWSTR path = new WCHAR[MAX_PATH];
    bool sign = K32GetModuleFileNameExW(hProcess, NULL, path, MAX_PATH);

    if (sign) {
        _string exeName = path;
        ret = exeName.substr(exeName.find_last_of('\\') + 1);
    }
    delete[] path;
    return sign;
}

TimeRecorder::_size TimeRecorder_sol_first::ForeExePid() {
    DWORD pid;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    return pid;
}

time_t TimeRecorder_sol_first::RunTime(const _appData& target) {
    return m_nowPID == target.pid ? time(NULL) - target.front + target.total
                                  : target.total;
}

void TimeRecorder_sol_first::Print() {
    std::wcout << L"正在运行的程序: \n";
    zql::Table table_a(0, 3);
    table_a.AddLine({L"名称", L"本次运行时间", L"总计运行时间"});
    auto i = m_runPro.begin();
    for (; i != m_runPro.end(); i++) {
        if (!i->running)
            break;

        _string name;
        table_a.AddLine({i->inSql ? i->sqlData.name : i->sqlData.exeName,
                         TimeRecorder::TimeToString(RunTime(*i)),
                         i->inSql ? TimeRecorder::TimeToString(
                                        RunTime(*i) + i->sqlData.totalTime)
                                  : TimeRecorder::TimeToString(RunTime(*i))});
    }
    std::wcout << table_a.GetUnicode() << std::endl;

    std::wcout << L"不在运行的程序: \n";
    zql::Table table_b(0, 2);
    table_b.AddLine({L"名称", L"总计运行时间"});
    for (; i != m_runPro.end(); i++) {
        table_b.AddLine({i->sqlData.name,
                         TimeRecorder::TimeToString(i->sqlData.totalTime)});
    }
    std::wcout << table_b.GetUnicode() << std::endl;
}