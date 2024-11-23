#!/bin/sh
set -xeu

# Do note that these commands are listed in the main README.md file,
# so if they are changed here, they should be changed in the main README.md file
# as well

cc -Wall -Wextra -Wpedantic -g -O1 -std=c99 \
  -I./include -I./cli \
  ./src/*.c ./cli/*.c \
  -lm \
  -o tcalc

cc -Wall -Wextra -Wpedantic -g -O1 -std=c99 \
  -I./include -I./tests/include -I./lib/cutest-1.5\
  ./src/*.c ./tests/src/*.c ./lib/cutest-1.5/CuTest.c \
  -lm \
  -o tcalc_tests

clang -g -O1 -I./include/ -fsanitize=fuzzer,address -std=c99 \
  src/*.c fuzzer/fuzzer.c \
  -lm \
  -o tcalc_fuzzer