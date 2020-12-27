## it spams the target list in CLion
include(doctest)
set(SOURCE_FILES TestMain.cpp ArgumentsTest.cpp)
add_executable(CheckedCmdTests ${SOURCE_FILES})
target_link_libraries(CheckedCmdTests doctest checked_cmd clara)
add_test(
        NAME CheckedCmdTests
        COMMAND CheckedCmdTests
)
