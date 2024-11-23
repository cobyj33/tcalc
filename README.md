# tcalc

`tcalc` is a simple terminal calculator written in C99
meant for use from the terminal.

`tcalc` was inspired by wanting to do some simple
calculations in my terminal but finding that the standard `expr`, `bc`, and `dc` commands
were quite inadequate for typing mathematical expressions in a format that was
comfortable for me. Even using `python -c` for anything that isn't the 4 main
arithmetic operators and exponentiation was quite a pain. Furthermore, when
writing shell scripts, I found it interesting how there were facilities
to do integer arithmetic calculations, but nothing to simply do floating point
calculations or use math operations beyond the simple 4 arithmetic operations.
For my use case, basic math functions and floating point calculation
should just be available directly from the command line
with no extra options or declarations, and I should just be able to type the
expression as a single
command. A simple use-case like this shouldn't require any large scientific
computing program and the program powering it shouldn't require any extra
dependencies to be installed to work. Therefore, I saw that I had a chance to
write exactly what I had wanted, and the task seemed quite manageable for the
scope I had assigned for myself.

`tcalc`, however, is quite limited: `tcalc` is not a programming language, nor
does it try to be. There is no way to define variables, there is no way
to define functions, and there are no control structures.
This isn't a bug or feature, it is simply outside the scope of what I want the
project to be capable of. Additionally, tcalc, does not support calculations
including complex numbers.

`tcalc`, has no dependencies except for the C99 standard library. For
building the testing executable,
[CuTest](https://sourceforge.net/projects/cutest/) is used, and it is included
in the `lib/` folder within the project's root repository.

`tcalc` is essentially stable. No big features or changes should be added
anytime soon. The scope
of the project is just to have a simple calculator terminal implementation.
Other features, such as arbitrary-precision arithmetic, control flow,
user-defined variables, and user-defined functions are slightly far outside of
the scope of what I'm willing to do for this project, and if I do implement a
project with those, it will be after much more studying and rewriting, and it
would be in a different repository.

## Compiling

There is a CMakeLists.txt to compile through, which should help when compiling
for non-UNIX platforms or platforms where a compiler isn't readily accessible
through a command-line. In a somewhat POSIX-compliant shell,
a standard cmake pipeline can be used to compile tcalc:

```sh
mkdir build
cd build
cmake ..
make
```

However, `tcalc` was made to be easy to compile, and I felt like
designing `tcalc` to have little dependencies would be spoiled if building
`tcalc` required a complicated build system like `cmake` to be installed
on the computer of a user who wants to compile `tcalc`. Therefore, assuming
you have `gcc` installed on your system, the command
below can be run in any somewhat POSIX-compiliant shell (such as `bash`, `dash`,
etc...) from the root folder of the tcalc repository to compile the whole
project. These commands are also contained in the `build.sh` file at the top of
the source tree.

For any non-GNU compilers, these commands should be simple enough to just
emulate

To build `tcalc`, run the command below from the root of the `tcalc` source
repository:

```sh
cc -Wall -Wextra -Wpedantic -g -O1 -std=c99 -I./include -I./cli ./src/*.c ./cli/*.c -lm -o tcalc
```

To build `tcalc_tests`, run the command below from the root of the `tcalc`
source repository (Optional):

```sh
cc -Wall -Wextra -Wpedantic -g -O1 -std=c99 -I./include -I./tests/include -I./lib/cutest-1.5 ./src/*.c ./tests/src/*.c ./lib/cutest-1.5/CuTest.c -lm -o tcalc_tests
```

To build `tcalc_fuzzer`, run the command below from the root of the `tcalc`
source repository (Optional):

```sh
clang -g -O1 -I./include/ -fsanitize=fuzzer,address -std=c99 src/*.c fuzzer/fuzzer.c -lm -o tcalc_fuzzer
```

## Usage:

```bash
tcalc --help
# tcalc usage: tcalc [-h] expression
#
# Optional arguments:
#     -h --help: Show this help message
#     --exprtree: Print the expression tree of the given expression
#     --tokens: Print the tokens of the given expression
#     --degrees: Set trigonometric functions to be defined with degrees
#     --radians: Set trigonometric functions to be defined with radians

tcalc "2 * 3 ^ ln(2)"
# 4.282972
tcalc "(sin(5))^2 + (cos(5))^2"
# 1.000000
tcalc "5 + sin(2 * pi)"
# 5.000000
tcalc "23 + arcsin(0.5) * (1 / 4)"
# 23.130900
tcalc "arctan( 5 % ln(10^e) + arctan( 5 * 2 ^ 5 ** 2 / (103 + sin(pow(3, cos(log(20pi))))) ))"
# 1.419767
```

## Limitations

Do note that all math in `tcalc` is done with double precision floating-point,
which means that operations can overflow if they go over the
maximum representable value of a C `double`.

`tcalc` also, to be frank, has quite terrible error reporting as of now,
usually just providing a somewhat cryptic message when an error has occured
which may or may not help at all. Better error reporting is the only thing truly
planned to be added.


## Usage Tips

When calling `tcalc`, it is reccomended to call `tcalc` as
`tcalc -- "<expression>"` rather than `tcalc "<expression>"`. This is because
if `"<expression>"`were to begin with a hyphen ('-') character, it will be
interpreted as a command-line option and the expression will not be
evaluated. The `--` command-line argument notifies `tcalc` that all following
command-line arguments are not command-line options. This should especially be
followed in command-line scripts where a user-supplied expression is passed to
`tcalc`.

The power of a simple command like `tcalc` comes to light when combined with
shell scripting to perform calculations.

While tcalc does not support variables natively, tcalc is used on the command
line, and the command line does support variables and parameter substitution
natively.  We can leverage this to give us a way to use variables **with** tcalc
rather than **in** tcalc. For example, if I were to define two parameters in the
shell:

```sh
a=3
b=21
c=5
x=4
tcalc "$a*$x^2 + $b*$x + $c"
# > 137.000000
```

This is, of course, still a parameter substitution,
which by being purely textual, may act in ways unexpected to a user.
For example, substituting two numbers next to each other would lead to
cocnatenating the digits of those numbers, not multiplying them:

```sh
a=5
b=6
tcalc "$a$b"
# > 56.000000
```

In fact, since shell parameter substitutions are textual, i.e. they don't care
about the context of the text in which they're used, many of the
same pitfalls of C macros mirror the pitfalls of using tcalc as such, and many
of the same solutions to these pitfalls also apply.

When substituting expressions, place that expression within parenthesis
to make sure operator precedences of the surrounding expression don't produce
unexpected values.

```sh
a=3
b=5
apb="$a+$b"
tcalc "$apb*5"
# Calculates as "3+5*5" = 28 when a user may expect "(3+5)*5" = 40
# > 28.00000
tcalc "$(tcalc "$a+$b") * 5"
tcalc "$(tcalc "$a+$b") * 5"
#
tcalc "$(tcalc "($a)+($b)") * 5"
```

Chain calls to tcalc to calculate expressions.

```sh
a=3
b=5
```

While tcalc (as of writing) only prints results in floating-point format, if
a user were to want an integer result, they could simply remove the returned
text including and after the decimal point to truncate the returned value.
For example, this can be done by piping the output of tcalc into the sed
command:

```sh
tcalc "1000sin(3*pi^5+2)" | sed 's/\..*//'
# > 414
```

The `round`, `floor`, `ceil`, and `trunc` commands can then be used
to approximate toward an integer value in more specific behaviors before
textually removing the trailing decimal part.

## Examining Tokens and the AST

This functionality is mostly exists for debugging purposes: Perhaps seeing the
tree that is executed will disambiguate exactly how each expression is
evaluated.

```sh
tcalc --exprtree "((3 ^ 2 + 5) / (ln(5)/ln(10))) ^ pi"
# ^
# |___/
# |___|___+
# |___|___|___^
# |___|___|___|___3
# |___|___|___|___2
# |___|___|___5
# |___|___/
# |___|___|___ln
# |___|___|___|___5
# |___|___|___ln
# |___|___|___|___10
# |___pi
# 12283.307634
tcalc --tokens "3 + sin(4e)"
# 3 + sin(4e)
# --------------------------------
# { index: 0, type: 'number', value: '3' },
# { index: 1, type: 'binary operator', value: '+' },
# { index: 2, type: 'identifier', value: 'sin' },
# { index: 3, type: 'group start', value: '(' },
# { index: 4, type: 'number', value: '4' },
# { index: 5, type: 'identifier', value: 'e' },
# { index: 6, type: 'group end', value: ')' }
# --------------------------------
# '3', '+', 'sin', '(', '4', 'e', ')'
# 2.007487
```
