cmake_minimum_required(VERSION 3.12)

project(c-csv
        VERSION 0.0.1
        DESCRIPTION "A simple C CSV reader"
        LANGUAGES C)

set(C_CSV_HOME "${CMAKE_CURRENT_LIST_DIR}")
set(C_CSV_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}")

set(C_CSV_SRC "${C_CSV_HOME}/src")
set(C_CSV_INCLUDE "${C_CSV_HOME}/include")
set(C_CSV_TEST "${C_CSV_HOME}/test")

file(GLOB C_CSV_SOURCES "${C_CSV_SRC}/*.c")
file(GLOB C_CSV_TEST_SOURCES "${C_CSV_TEST}/*.c")
file(GLOB C_CSV_HEADERS "${C_CSV_INCLUDE}/*.h")

add_library(c-csv STATIC "${C_CSV_SOURCES}")
#target_compile_options(c-csv PUBLIC -Werror)
target_include_directories(c-csv PRIVATE ${C_CSV_SRC}
			         			 PUBLIC  ${C_CSV_INCLUDE})
set_target_properties(c-csv PROPERTIES
					  OUTPUT_NAME "c-csv"
		              PUBLIC_HEADER "${C_CSV_HEADERS}")

add_executable(c-csv-test ${C_CSV_TEST_SOURCES} ${C_CSV_SOURCES})
#target_compile_options(c-csv-test PUBLIC -Werror)
target_link_libraries(c-csv-test c-csv)
