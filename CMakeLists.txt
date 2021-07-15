cmake_minimum_required (VERSION 3.8)

add_definitions(-DUNICODE -D_UNICODE)

# 添加包含文件
include_directories("include" "lib/include")

project (TimeRecorder)

set(PROJECT_SOURCES
    "lib/sqlite3.c"
    "src/TimeRecorder.cpp"
    "src/TimeRecorderBase.cpp"
    "src/TimeRecorderLog.cpp"
    "src/TimeRecorderSQL.cpp"
    "src/TimeRecorderSQLite.cpp"
    "src/TimeRecorderWin32.cpp"
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} "src/main.cpp")

add_executable(TestSQLite ${PROJECT_SOURCES} "Test/TestSQLite.cpp")
