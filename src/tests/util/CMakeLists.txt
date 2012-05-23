message("-- Util tests --")

find_package( Boost COMPONENTS test_exec_monitor unit_test_framework REQUIRED)

add_executable(UtilLinux_test UtilLinux_test.cpp)

add_dependencies(UtilLinux_test util)

target_link_libraries(UtilLinux_test ${Boost_TEST_EXEC_MONITOR_LIBRARY} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} util)