cmake_minimum_required(VERSION 3.7)

project(tcalc VERSION 0.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

option(TCALC_BUILD_TESTS "Build tcalc tests" OFF)

message("+--------------------------------------------+")
message("|-TCALC Configuration:------------------------")
message("| TCALC_BUILD_TESTS: " ${TCALC_BUILD_TESTS})
message("+--------------------------------------------+")

set(TCALC_LIB_SRC_FILES
${CMAKE_SOURCE_DIR}/src/tcalc_context.c
${CMAKE_SOURCE_DIR}/src/tcalc_error.c
${CMAKE_SOURCE_DIR}/src/tcalc_eval.c
${CMAKE_SOURCE_DIR}/src/tcalc_exit.c
${CMAKE_SOURCE_DIR}/src/tcalc_exprtree.c
${CMAKE_SOURCE_DIR}/src/tcalc_func.c
${CMAKE_SOURCE_DIR}/src/tcalc_infix_to_rpn.c
${CMAKE_SOURCE_DIR}/src/tcalc_mem.c
${CMAKE_SOURCE_DIR}/src/tcalc_parser.c
${CMAKE_SOURCE_DIR}/src/tcalc_string.c
${CMAKE_SOURCE_DIR}/src/tcalc_tokens.c
)

set(TCALC_CLI_SRC_FILES
${CMAKE_SOURCE_DIR}/cli/tcalc_cli_common.c
${CMAKE_SOURCE_DIR}/cli/tcalc_cli_eval_rpn.c
${CMAKE_SOURCE_DIR}/cli/tcalc_cli_eval.c
${CMAKE_SOURCE_DIR}/cli/tcalc_cli_infix_tokenizer.c
${CMAKE_SOURCE_DIR}/cli/tcalc_cli_print_exprtree.c
${CMAKE_SOURCE_DIR}/cli/tcalc_cli_rpn_tokenizer.c
${CMAKE_SOURCE_DIR}/cli/tcalc_cli.c
${CMAKE_SOURCE_DIR}/cli/tcalc_repl.c
)

set(TCALC_TEST_SRC_FILES
${CMAKE_SOURCE_DIR}/tests/src/test_tcalc_rpn_eval.c
${CMAKE_SOURCE_DIR}/tests/src/test_tcalc_tokenize.c
${CMAKE_SOURCE_DIR}/tests/src/test_tcalc_string.c
${CMAKE_SOURCE_DIR}/tests/src/test_tcalc_eval.c
)

set(TCALC_COMPILE_OPTIONS -Wall -Wextra -Wpedantic)
if (MSVC)
  set(TCALC_COMPILE_OPTIONS /W4 /WX)
endif()

add_library(tcalc ${TCALC_LIB_SRC_FILES})
target_include_directories(tcalc PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(tcalc PUBLIC m)
target_compile_options(tcalc PRIVATE ${TCALC_COMPILE_OPTIONS})
set_target_properties(tcalc PROPERTIES C_STANDARD 99)

add_library(cutest SHARED ${CMAKE_SOURCE_DIR}/lib/cutest-1.5/CuTest.c)
target_include_directories(cutest PUBLIC ${CMAKE_SOURCE_DIR}/lib/cutest-1.5)
set_target_properties(cutest PROPERTIES C_STANDARD 99)

add_executable(tcalc_cli ${TCALC_CLI_SRC_FILES})
set_property(TARGET tcalc_cli PROPERTY OUTPUT_NAME tcalc)
target_link_libraries(tcalc_cli PRIVATE tcalc)
set_target_properties(tcalc_cli PROPERTIES C_STANDARD 99)

if (TCALC_BUILD_TESTS)
  add_executable(tcalc_tests ${CMAKE_SOURCE_DIR}/tests/src/test_run_all_tests.c ${TCALC_TEST_SRC_FILES} )
  target_include_directories(tcalc_tests PRIVATE ${CMAKE_SOURCE_DIR}/tests/include)
  target_link_libraries(tcalc_tests PRIVATE cutest tcalc)
endif()
