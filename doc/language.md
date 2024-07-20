# tcalc language definition

Note that anything in this document is subject to change at anytime, although
things will of course change based on decisions

## Values

Values in tcalc can either be numbers or booleans.
As of now, numbers are only represented as double-precision floating point
numbers. However, as the language progresses, I will definitely
switch to a more scientific-computing-based number system, probably
with the [GNU GMP](https://gmplib.org/) and
[GNU MPFR](https://www.mpfr.org/) libraries.

Booleans are not treated as either 0s or 1s, and are not

## Numbers



### Number Structure

### Number Classification

## Arithmetic Operators

### Unary Operators

- \+ [Unary Plus](#subtraction)
- \+ [Unary Minus](#subtraction)

#### Unary Plus

#### Unary Minus

### Binary Operators

- \+ [Addition](#addition)
- \- [Subtraction](#subtraction)
- \* [Multiplication](#multiplication)
- / [Division](#division)
- ^, ** [Exponentiation](#exponentiation)

#### Addition


#### Subtraction


#### Multiplication


#### Division


#### Exponentiation


## Relational Operators

- =
- ==
- !=
- <
- \>
- <=
- \>=

## Logical Operators

- !
- =
- ==
- !=
- &&
- ||

> NOTE: Still trying to figure out if = and == should be different
>
>
>

## Builtin Functions



### Number Casting Functions

### Mathematical Functions


## Environment


### Variables

There are two main operators for assignment in tcalc: ":=" and "<="
:= serves as a binding reference between the given function or variable and
its dependencies
<= serves as a simple evaluation between the given function or variable and
its dependencies.

Variables can be set with the := operator.

Example:

```tcalc
x := 5+3
x + 5
> 13 // x + 3;
```

Functions can be set with the func([ arg1 [, argn]* ]) :=

```tcalc
f(x) := x + 3;
g(x, y) := f(x) + 2y;
h(x, y) <= f(x) + 3;
g(2, 3) + f(3)
```

Variables are binded toward their arguments. Whenever their arguments
definition changes, the variable additionally changes

## ABNF Specification

```abnf

expression :=


```
