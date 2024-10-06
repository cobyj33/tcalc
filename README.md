# tcalc

Terminal Floating-Precision Calculator written in C99

## Usage:

```bash
tcalc "2 * 3 ^ ln(2)"
tcalc "(sin(5))^2 + (cos(5))^2"
tcalc "5 + sin(2 * pi)"
tcalc "23 + arcsin(0.5) * (1 / 4)"
```

## Accepted Operators

- a + b
- a - b
- a * b
- a / b
- a % b
- -a
- +a

## Accepted Functions

### Trigonometric Functions

- sin(a), cos(a), tan(a)
- sec(a), csc(a), cot(a)
- asin(a), arcsin(a), acos(a), arccos(a), atan(a), arctan(a)
- asec(a), arcsec(a), acsc(a), arccsc(a), acot(a), arccot(a)

### Hyperbolic Functions

- sinh(a), cosh(a), tanh(a)
- asinh(a), arcsinh(a), acosh(a), arccosh(a), atanh(a), arctanh(a)

### Exponential Functions

- log(a)
- ln(a)
- exp(a)
- sqrt(a)
- cbrt(a)
- pow(a, b)

### Other Functions

- ceil(a)
- floor(a)
- round(a)
- abs(a)


## Accepted Grouping Symbols

- (a)
- \[b]

## Accepted Variables:

- pi
- e
