cmake_minimum_required(VERSION 3.7)

project(tcalc VERSION 0.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(TCALC_SRC_FILES
./src/tcalc_error.c
./src/tcalc_darray.c
./src/tcalc_string.c
./src/tcalc_dstring.c
./src/tcalc_func.c
./src/tcalc_eval.c
./src/tcalc_eval_rpn.c
./src/tcalc_infix_to_rpn.c
./src/tcalc_tokens.c
./src/tcalc_tokenize_infix.c
./src/tcalc_mem.c
./src/tcalc_stradv.c
./src/tcalc_tokenize_rpn.c
)

set(TEST_SRC_FILES
./tests/src/test_tcalc_darray.c
./tests/src/test_tcalc_rpn_eval.c
./tests/src/test_tcalc_tokenize.c
./tests/src/test_tcalc_dstring.c
./tests/src/test_tcalc_string.c
)

add_library(tcalc ${TCALC_SRC_FILES})
target_include_directories(tcalc PUBLIC ./include)
target_link_libraries(tcalc PUBLIC m)

add_library(cutest SHARED ./lib/cutest-1.5/CuTest.c)
target_include_directories(cutest PUBLIC ./lib/cutest-1.5)

add_executable(tcalc_tests ./tests/src/test_run_all_tests.c ${TEST_SRC_FILES} )
target_include_directories(tcalc_tests PRIVATE ./tests/include)
target_link_libraries(tcalc_tests PRIVATE cutest tcalc)

add_executable(tokenizer ./cli/tokenizer.c)
target_link_libraries(tokenizer PRIVATE tcalc)

add_executable(rpn ./cli/rpn.c)
target_link_libraries(rpn PRIVATE tcalc)

add_executable(infix_to_rpn ./cli/infix_to_rpn.c)
target_link_libraries(infix_to_rpn PRIVATE tcalc)

add_executable(tcalc_cli ./cli/tcalc.c)
set_property(TARGET tcalc_cli PROPERTY OUTPUT_NAME tcalc)
target_link_libraries(tcalc_cli PRIVATE tcalc)