#ifndef TCALC_CONSTANTS_H
#define TCALC_CONSTANTS_H

/**
 * I literally copied this from math.h of glibc. I don't get
 * why M_PI is sometimes included and sometimes it's not but whatever.
*/

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E		2.7182818284590452354	/* e */
#endif

#ifndef M_LN10
# define M_LN10		2.30258509299404568402	/* log_e 10 */
#endif

#endif