#ifndef TCALC_EXIT_H
#define TCALC_EXIT_H

/**
 * Quickly terminate the program with a printf-formatted message toward stderr.
 * 
 * @param err a printf-formatted string which will be printed to stderr 
 * @param ... printf-style varargs
*/
void tcalc_die(const char* err, ...);

#endif