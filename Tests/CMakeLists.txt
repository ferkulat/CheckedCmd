
set(SOURCE_FILES TestMain.cpp ArgumentsTest.cpp)
add_executable(CheckedCmdTests ${SOURCE_FILES})
add_test(
        NAME CheckedCmdTests
        COMMAND CheckedCmdTests
)
