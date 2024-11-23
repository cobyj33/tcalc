#include "ccmdlpar.h"

// no standard C library just cuz, except for assert I guess

#define CCMDLPAR_NULL ((void*)0)

// If you want to remove C standard library, just change the 1 to 0
#if 1
#include <assert.h>
#else
#undef assert
#define assert(...) ((void)0)
#endif

// Only deals with command-line arguments which will never
// The functionality of the option ender is encoded within our ccmdlpar_state
// and must also be considered to decide if a command-line argument should
// be interpreted as a non-option
#define CCMDLPAR_IS_NON_OPTION(str) ((str)[0] != '-' || ((str)[0] == '-' && (str)[1] == '\0'))

#define CCMDLPAR_IS_LONG_OPTION(str) ((str)[0] == '-' && (str)[1] == '-' && (str)[2] != '\0')

#define CCMDLPAR_IS_SHORT_OPTION(str) ((str)[0] == '-' && (str)[1] != '-' && (str)[1] != '\0')

#define CCMDLPAR_IS_OPTION_ENDER(str) ((str)[0] == '-' && (str)[1] == '-' && (str)[2] == '\0')

static int ccmdlpar_lbstr_ntstr_eq(const char* const lbstr, const int lbstrLen, const char* const ntstr)
{
  int i = 0;
  while (i < lbstrLen && ntstr[i] != '\0' && lbstr[i] == ntstr[i]) i++;
  return ntstr[i] == '\0' && i == lbstrLen;
}

static int ccmdlpar_ntstr_strlen(const char* ntstr)
{
  int i = 0;
  while (ntstr[i] != '\0') i++;
  return i;
}

int ccmdlpar_found_sopt(struct ccmdlpar_parse_info pi, char ch)
{
  assert(pi.argType != CCMDLPAR_ARG_TYPE_SHORT_OPTION ||
    (
      pi.argType == CCMDLPAR_ARG_TYPE_SHORT_OPTION &&
      pi.arg != CCMDLPAR_NULL &&
      pi.argLen == 1 &&
      (*pi.arg) != '\0'
    )
  );

  return pi.argType == CCMDLPAR_ARG_TYPE_SHORT_OPTION
    && *(pi.arg) == ch;
}

int ccmdlpar_found_lopt(struct ccmdlpar_parse_info pi, const char* ntstr)
{
  assert(ntstr != CCMDLPAR_NULL);
  assert(pi.argType != CCMDLPAR_ARG_TYPE_LONG_OPTION ||
    (
      pi.argType == CCMDLPAR_ARG_TYPE_LONG_OPTION &&
      pi.arg != CCMDLPAR_NULL &&
      pi.argLen > 0
    )
  );

  return ntstr != CCMDLPAR_NULL &&
    pi.argType == CCMDLPAR_ARG_TYPE_LONG_OPTION &&
    ccmdlpar_lbstr_ntstr_eq(pi.arg, pi.argLen, ntstr);
}

enum ccmdlpar_status ccmdlpar(
  int argc,
  const char * const * argv,
  struct ccmdlpar_state * state,
  const char* soptsWArg,
  const char* const * loptsWArg,
  struct ccmdlpar_parse_info* outParseInfo
)
{
  assert(state != CCMDLPAR_NULL);
  assert(argv != CCMDLPAR_NULL);
  assert(outParseInfo != CCMDLPAR_NULL);
  assert(argv[argc] == CCMDLPAR_NULL);
  assert(state->optind >= 0);
  assert(state->chind >= 0);

  // Before returning in **any** case, even trivially erroneous cases,
  //  we explicitly seet all fields of this
  // struct. This is to keep debugability easy, as all assignments to
  // outParseInfo are grouped in one place.
  if (outParseInfo != CCMDLPAR_NULL)
  {
    *outParseInfo = (struct ccmdlpar_parse_info){
      .arg = CCMDLPAR_NULL,
      .argLen = 0,
      .argType = CCMDLPAR_ARG_TYPE_NON_OPTION,
      .optind = -1,
      .chind = -1,
      .optarg = CCMDLPAR_NULL,
      .optArgInd = -1,
    };
  }


  // every case here should be asserted above
  if (state == CCMDLPAR_NULL ||
    argv == CCMDLPAR_NULL ||
    outParseInfo == CCMDLPAR_NULL ||
    argv[argc] != CCMDLPAR_NULL ||
    state->optind < 0 ||
    state->chind < 0)
  {
    return CCMDLPAR_STATUS_ERR_FATAL;
  }

  if (state->optind >= argc)
    return CCMDLPAR_STATUS_FINISHED;

  assert(state->optind < argc);
  // state->chind is at 0, or it is not at 0 and we are in a short option and
  // we are not at the end of the short option
  assert(state->chind == 0 || (state->chind > 0 && CCMDLPAR_IS_SHORT_OPTION(argv[state->optind]) && argv[state->optind][state->chind] != '\0'));

  if (!state->isOptionsEnded && CCMDLPAR_IS_OPTION_ENDER(argv[state->optind]))
  {
    state->isOptionsEnded = 1;
    state->optind++;
    state->chind = 0;
    if (state->optind >= argc)
      return CCMDLPAR_STATUS_FINISHED;
  }

  if (state->isOptionsEnded || CCMDLPAR_IS_NON_OPTION(argv[state->optind]))
  {
    *outParseInfo = (struct ccmdlpar_parse_info){
      .arg = argv[state->optind],
      .argLen = ccmdlpar_ntstr_strlen(argv[state->optind]),
      .argType = CCMDLPAR_ARG_TYPE_NON_OPTION,
      .optind = state->optind,
      .chind = 0,
      .optarg = CCMDLPAR_NULL,
      .optArgInd = -1
    };

    state->optind++;
    state->chind = 0;
    return CCMDLPAR_STATUS_OK;
  }

  if (CCMDLPAR_IS_SHORT_OPTION(argv[state->optind]) && state->chind == 0)
  {
    // after incrementing chind, state->chind must still be in bounds since
    // CCMDLPAR_IS_SHORT_OPTION guarantees argv[state->optind][1] != '\0'
    state->chind++;
  }

  // short options
  if (state->chind > 0 && argv[state->optind][state->chind] != '\0')
  {
    assert(CCMDLPAR_IS_SHORT_OPTION(argv[state->optind]));

    int hasArg = 0;
    for (int i = 0; soptsWArg != CCMDLPAR_NULL && soptsWArg[i] != '\0'; i++)
    {
      if (soptsWArg[i] == argv[state->optind][state->chind])
      {
        hasArg = 1;
        break;
      }
    }

    *outParseInfo = (struct ccmdlpar_parse_info){
      .arg = argv[state->optind] + state->chind,
      .argLen = 1,
      .argType = CCMDLPAR_ARG_TYPE_SHORT_OPTION,
      .optind = state->optind,
      .chind = state->chind,
      .optarg = CCMDLPAR_NULL, // to be possibly set later
      .optArgInd = -1 // to be possibly set later
    };

    if (hasArg)
    {
      if (argv[state->optind][state->chind + 1] != '\0')
      {
        state->chind++;
        return CCMDLPAR_STATUS_CHAINED_SHORT_OPT_NOT_LAST;
      }
      else if (state->optind + 1  == argc)
      {
        state->optind++;
        state->chind = 0;
        return CCMDLPAR_STATUS_MISSING_OPT_ARG;
      }

      outParseInfo->optarg = argv[state->optind + 1];
      outParseInfo->optArgInd = state->optind + 1;

      state->optind += 2;
      state->chind = 0;
      return CCMDLPAR_STATUS_OK;
    }
    else
    {
      state->chind++;
      if (argv[state->optind][state->chind] == '\0')
      {
        state->optind++;
        state->chind = 0;
      }

      return CCMDLPAR_STATUS_OK;
    }

    assert(0 && "unreachable");
    return CCMDLPAR_STATUS_ERR;
  }

  if (CCMDLPAR_IS_LONG_OPTION(argv[state->optind]))
  {
    const char* loptName = argv[state->optind] + 2;
    int loptNameLen = 0;
    while (loptName[loptNameLen] != '\0' && loptName[loptNameLen] != '=')
      loptNameLen++;
    assert(loptNameLen > 0);
    // loptNameLen > 0 should be guaranteed by CCMDLPAR_IS_LONG_OPTION
    // now, loptName[loptNameLen] == '\0' || loptName[loptNameLen] == '='

    int hasArg = 0;
    for (int i = 0; loptsWArg != CCMDLPAR_NULL && loptsWArg[i] != CCMDLPAR_NULL; i++)
    {
      if (ccmdlpar_lbstr_ntstr_eq(loptName, loptNameLen, loptsWArg[i]))
      {
        hasArg = 1;
        break;
      }
    }

    *outParseInfo = (struct ccmdlpar_parse_info){
      .arg = loptName,
      .argLen = loptNameLen,
      .argType = CCMDLPAR_ARG_TYPE_LONG_OPTION,
      .optind = state->optind,
      .chind = 0,
      .optarg = CCMDLPAR_NULL, // to be possibly set later
      .optArgInd = -1 // to be possibly set later
    };

    if (hasArg)
    {
      if (loptName[loptNameLen] == '=')
      {
        outParseInfo->optarg = loptName + loptNameLen + 1;
        outParseInfo->optArgInd = state->optind;

        state->optind++;
        state->chind = 0;
        return CCMDLPAR_STATUS_OK;
      }
      else
      {
        if (state->optind + 1 >= argc)
          return CCMDLPAR_STATUS_MISSING_OPT_ARG;

        outParseInfo->optarg = argv[state->optind + 1];
        outParseInfo->optArgInd = state->optind + 1;

        state->optind += 2;
        state->chind = 0;
        return CCMDLPAR_STATUS_OK;
      }
    }
    else
    {
      state->optind++;
      state->chind = 0;
      return CCMDLPAR_STATUS_OK;
    }

    assert(0 && "unreachable");
    return CCMDLPAR_STATUS_ERR;
  }

  assert(0 && "unreachable");

  // ensure forward progress even in weird error states
  state->optind++;
  state->chind = 0;
  return CCMDLPAR_STATUS_ERR;
}

const char* ccmdlpar_arg_type_str(enum ccmdlpar_arg_type argType)
{
  switch (argType)
  {
    case CCMDLPAR_ARG_TYPE_NON_OPTION: return "non-option";
    case CCMDLPAR_ARG_TYPE_SHORT_OPTION: return "short option";
    case CCMDLPAR_ARG_TYPE_LONG_OPTION: return "long option";
  }

  assert(0 && "unreachable");
  return "unknown";
}

const char* ccmdlpar_arg_type_prefix(enum ccmdlpar_arg_type argType)
{
  switch (argType)
  {
    case CCMDLPAR_ARG_TYPE_NON_OPTION: return "";
    case CCMDLPAR_ARG_TYPE_SHORT_OPTION: return "-";
    case CCMDLPAR_ARG_TYPE_LONG_OPTION: return "--";
  }

  assert(0 && "unreachable");
  return "";
}

const char* ccmdlpar_status_str(enum ccmdlpar_status status)
{
  switch (status)
  {
    case CCMDLPAR_STATUS_OK: return "ok";
    case CCMDLPAR_STATUS_ERR: return "unknown error";
    case CCMDLPAR_STATUS_ERR_FATAL: return "fatal error";
    case CCMDLPAR_STATUS_FINISHED: return "finished";
    case CCMDLPAR_STATUS_MISSING_OPT_ARG: return "missing opt arg";
    case CCMDLPAR_STATUS_CHAINED_SHORT_OPT_NOT_LAST: return "chained short opt not last";
  }

  assert(0 && "unreachable");
  return "unknown";
}
