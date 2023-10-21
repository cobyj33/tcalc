cmake_minimum_required(VERSION 3.7)

project(tcalc VERSION 0.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

option(TCALC_BUILD_TESTS "Build tcalc tests" OFF)

set(TCALC_SRC_FILES
./src/tcalc_error.c
./src/tcalc_string.c
./src/tcalc_dstring.c
./src/tcalc_func.c
./src/tcalc_eval.c
./src/tcalc_tokens.c
./src/tcalc_exprtree.c
./src/tcalc_mem.c
./src/tcalc_infix_to_rpn.c
./src/tcalc_context.c
)

set(TEST_SRC_FILES
./tests/src/test_tcalc_rpn_eval.c
./tests/src/test_tcalc_tokenize.c
./tests/src/test_tcalc_dstring.c
./tests/src/test_tcalc_string.c
./tests/src/test_tcalc_eval.c
)

set(TCALC_COMPILE_OPTIONS -Wall -Wextra -Wpedantic)
if (MSVC)
  set(TCALC_COMPILE_OPTIONS /std:c++17 /W4 /WX)
endif()

add_library(tcalc ${TCALC_SRC_FILES})
target_include_directories(tcalc PUBLIC ./include)
target_link_libraries(tcalc PUBLIC m)
target_compile_options(tcalc PRIVATE ${TCALC_COMPILE_OPTIONS})
set_target_properties(tcalc PROPERTIES C_STANDARD 99)

add_library(cutest SHARED ./lib/cutest-1.5/CuTest.c)
target_include_directories(cutest PUBLIC ./lib/cutest-1.5)

add_executable(tcalc_cli ./cli/tcalc.c)
set_property(TARGET tcalc_cli PROPERTY OUTPUT_NAME tcalc)
target_link_libraries(tcalc_cli PRIVATE tcalc)

if (TCALC_BUILD_TESTS)
  add_executable(tcalc_tests ./tests/src/test_run_all_tests.c ${TEST_SRC_FILES} )
  target_include_directories(tcalc_tests PRIVATE ./tests/include)
  target_link_libraries(tcalc_tests PRIVATE cutest tcalc)

  add_executable(rpn_tokenizer ./cli/rpn_tokenizer.c)
  target_link_libraries(rpn_tokenizer PRIVATE tcalc)
  
  add_executable(infix_tokenizer ./cli/infix_tokenizer.c)
  target_link_libraries(infix_tokenizer PRIVATE tcalc)
  
  add_executable(rpn ./cli/rpn.c)
  target_link_libraries(rpn PRIVATE tcalc)
  
  add_executable(infix_to_rpn ./cli/infix_to_rpn.c)
  target_link_libraries(infix_to_rpn PRIVATE tcalc)
  
  add_executable(exprtree ./cli/exprtree.c)
  target_link_libraries(exprtree PRIVATE tcalc)
endif()
