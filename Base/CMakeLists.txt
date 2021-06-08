# TimeRecorder 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

add_definitions(-DUNICODE -D_UNICODE)

project (TimeRecorderBase)

# 添加包含文件
include_directories(include)
include_directories(SQLite)
include_directories(zqlTools)

add_library(${PROJECT_NAME} "SQLite/sqlite3.c" "src/TimeRecorder.cpp" "src/TimeRecorder_sol_first.cpp" "src/TimeRecorder_sql_sqlite.cpp")