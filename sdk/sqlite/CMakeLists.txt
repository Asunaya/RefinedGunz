add_target(NAME sqlite TYPE STATIC SOURCES "src/sqlite3.c")
target_include_directories(sqlite INTERFACE include)