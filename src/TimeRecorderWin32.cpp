#include "TimeRecorderWin32.h"
#include "TimeRecorderLog.h"
#include "TimeRecorderSQLite.h"
#include "table.h"
#include <Psapi.h>
#include <TlHelp32.h>
#include <Windows.h>
#include <iostream>
#include <locale>

TimeRecorderWin32::TimeRecorderWin32(TString SqlFile)
    : TimeRecorder(new TimeRecorderLog(FileName_Log),
                   new TimeRecorderSQLite(FileName_SQL, FileName_Log)) {
    m_apps = m_sql->m_apps;
}

void TimeRecorderWin32::exec() {
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
        SetConsoleCursorPosition(hOutput, bInfo.dwCursorPosition);
        Print();
        Sleep(1000);
    }
}

void TimeRecorderWin32::ModeBack(TApp& app) {
    bool run = IsProcessRunning(app.exeName);
    if (run && !app.running) {
        app.running = true;
        app.runTime.begin = NowTime();
    } else if (!run && app.running) {
        app.runTime.end = NowTime();
        app.runTime.total = app.runTime.end - app.runTime.begin;
        app.running = false;
        InsertRecord(app);
    }
}

// 执行到此函数的前提条件
// m_foreExe != m_preForeExe
// app.running == true
bool TimeRecorderWin32::ModeNone(TApp& app) {
    app.running = IsProcessRunning(app.exeName);
    if (!app.running)
        return false;
    if (app.exeName == m_foreExe)
        app.runTime.end = NowTime();
    else if (app.exeName == m_preForeExe)
        app.runTime.total = NowTime() - app.runTime.end;
    return true;
}

// m_foreExe != m_preForeExe
void TimeRecorderWin32::ModeFore(TApp& app) {
    bool run = IsProcessRunning(app.exeName);
    if (!app.running && run) {
        app.running = true;
        app.runTime.end = app.runTime.begin = NowTime();
        app.runTime.total = 0;
        return;
    }
    if (app.exeName == m_preForeExe)
        app.runTime.total = NowTime() - app.runTime.end;
    else if (app.exeName == m_foreExe)
        app.runTime.end = NowTime();
    if (!run && app.running) {
        app.running = false;
        app.runTime.end = NowTime();
        InsertRecord(app);
    }
}

void TimeRecorderWin32::Update() {
    GetAllProcess();
    m_preForeExe = m_foreExe;
    // get fore exeName
    DWORD pid;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    HANDLE hProcess =
        OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    //获取进程路径
    LPWSTR path = new WCHAR[MAX_PATH];
    bool sign = K32GetModuleFileNameExW(hProcess, NULL, path, MAX_PATH);
    if (sign) {
        m_foreExe = path;
        m_foreExe = m_foreExe.substr(m_foreExe.find_last_of('\\') + 1);
    } else
        m_log->WriteLog(
            LOGLEVEL::error,
            L"Failed to get the name of the foreground program: pid = " +
                std::to_wstring(pid));
    delete[] path;

    sign = true;
    for (auto i = m_apps.begin(); i != m_apps.end();) {
        if (sign && i->exeName == m_foreExe)
            sign = false;
        if (i->mode == TMODE::back)
            ModeBack(*i++);
        else if (m_foreExe == m_preForeExe)
            i++;
        else if (i->mode == TMODE::none)
            i = ModeNone(*i) ? i + 1 : m_apps.erase(i);
        else if (i->mode == TMODE::fore)
            ModeFore(*i++);
    }
    if (sign) {
        TApp app;
        app.exeName = m_foreExe;
        app.runTime.begin = app.runTime.end = NowTime();
        app.running = true;
        m_apps.push_back(app);
    }
}

void TimeRecorderWin32::InsertRecord(TApp& app) {
    app.records.Insert(app.runTime);
    m_sql->AddRecord(app.sqlId, app.runTime);
}

void TimeRecorderWin32::GetAllProcess() {
    m_runningApps.clear();

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        m_log->WriteLog(LOGLEVEL::error, L"Failed to obtain system snapshot.");
        return;
    }
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        m_log->WriteLog(LOGLEVEL::error,
                        L"Failed to obtain system process information.");
        CloseHandle(hProcessSnap);
        return;
    }
    do {
        m_runningApps.push_back(pe32.szExeFile);
    } while (Process32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
}

bool TimeRecorderWin32::IsProcessRunning(const TString& name) {
    return find(m_runningApps.begin(), m_runningApps.end(), name) !=
           m_runningApps.end();
}

TimeRecorderWin32::TTime TimeRecorderWin32::RunTime(const TApp& app) {
    if (!app.running)
        return 0;
    if (app.mode == TMODE::back)
        return NowTime() - app.runTime.begin;
    else {
        if (app.exeName == m_foreExe)
            return NowTime() - app.runTime.end + app.runTime.total;
        else
            return app.runTime.total;
    }
}

void TimeRecorderWin32::Print() {
    // 正在运行的程序
    zql::Table table_a(0, 3);
    table_a.AddLine({L"名称", L"本次运行时间", L"总计运行时间"});
    for (auto i = m_apps.begin(); i != m_apps.end(); i++) {
        if (!i->running)
            continue;
        if (i->mode == TMODE::none)
            table_a.AddLine({i->exeName,
                             TimeRecorder::TimeToString(RunTime(*i)),
                             TimeRecorder::TimeToString(RunTime(*i))});
        else {
            TAppRecord temp = i->records;
            TRecord rec = i->runTime;
            rec.total = RunTime(*i);
            temp.Insert(rec);
            table_a.AddLine({i->name, TimeRecorder::TimeToString(RunTime(*i)),
                             TimeRecorder::TimeToString(temp.total)});
        }
    }
    // 不在运行的程序
    zql::Table table_b(0, 2);
    table_b.AddLine({L"名称", L"总计运行时间"});
    for (auto i = m_apps.begin(); i != m_apps.end(); i++) {
        if (i->running)
            continue;
        table_b.AddLine(
            {i->exeName, TimeRecorder::TimeToString(i->records.total)});
    }

    TString out = L"正在运行的程序: \n" + table_a.GetUnicode() +
                  L"\n不在运行的程序: \n" + table_b.GetUnicode() +
                  L"\n北京时间: " +
                  TimeRecorder::DateToString(TimeRecorder::NowTime()) + L'\n';
    // 用空格填补空白
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO bInfo;
    GetConsoleScreenBufferInfo(hOutput, &bInfo);
    for (auto i : out) {
        if (i == L'\n') {
            GetConsoleScreenBufferInfo(hOutput, &bInfo);
            std::wcout << TString(bInfo.dwSize.X - bInfo.dwCursorPosition.X - 1,
                                  L' ')
                       << '\n';
        } else
            std::wcout << i;
    }
}