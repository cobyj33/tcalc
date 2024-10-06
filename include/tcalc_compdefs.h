#ifndef TCALC_COMPDEFS_H
#define TCALC_COMPDEFS_H

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
  #define TCALC_FORMAT_ATTRIB(format_type, format_param_i, vararg_start_i) \
    __attribute__((format (format_type, format_param_i, vararg_start_i)))
#else
  #define TCALC_FORMAT_ATTRIB(format_type, format_param_i, vararg_start_i)
#endif

#endif
