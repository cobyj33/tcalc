#ifndef TCALC_TNUM_H
#define TCALC_TNUM_H

typedef struct tnum_impl TNum;

TNum tnum_add(TNum a, TNum b);
TNum tnum_subtract(TNum a, TNum b);
TNum tnum_multiply(TNum a, TNum b);
TNum tnum_divide(TNum a, TNum b);

#endif