#ifndef CCMDLPAR_H
#define CCMDLPAR_H

//  ccmdlpar is only ASCII aware (sort of)
//
//  More specifically, UTF-8 could be used, but non-ASCII names of
//  short/long options
//  with option arguments cannot be matched reliably by `ccmdlpar` when detected
//  on the command line. This is because, although a UTF-8 encoded short/long
//  option name **could** be recognized, as it is just a sequence of bytes,
//  the same unicode string can be made from
//  different unicode code points, and ccmdlpar does not handle and does not
//  ever plan to handle unicode equivalence or unicode normalization.
//
//  Non-options and option arguments should be able to be processed in UTF-8
//  just fine, as they are not processed, they are just returned as byte strings
//  back to the caller.
//
//  This does technically mean that all the string 'lengths' like
//  ccmdlpar_parse_info->argLen in ccmdlpar are just
//  byte lengths though, which is why I saw ccmdlpar is 'only ASCII aware'.


//  The internal state used in `ccmdlpar` to keep track of where to restart
//  command-line parsing on each successive call to `ccmdlpar`.
//
//  Users of the `ccmdlpar` function don't need to touch this state at all
//  beyond just initializing it to optind = 1, chind = 0, and
//  isOptionsEnded = 0, which can be done with the CCMDLPAR_STATE_INIT macro.
//
//  This can be easily done with the CCMDLPAR_STATE_INIT macro.
//  After a return from `ccmdlpar`, a ccmdlpar_state struct represents the
//  state to be used on the **next** call to ccmdlpar. This means that, for
//  viewing errors, you should save the last ccmdlpar_state value to query
//  on what input an error occured.
//
//  An example for this can be seen in the `ccmdlpar-template.c` file. It's
//  much simpler than it may sound.
struct ccmdlpar_state
{
  // The index of the next command-line argument to be processed.
  int optind;
  // The index of the next character within argv[optind] to process.
  // Only used and meaningful for processing chained short options
  int chind;
  // Set whenever we have hit an end of options '--' command-line argument
  int isOptionsEnded;
};

// Note that this is to show that for C, the starting index of option parsing
// is at argv[1]. If parsing an arbitrary array of strings, optind just
// starts wherever options start in your array. Everything else should be 0
// though
#define CCMDLPAR_STATE_INIT ((struct ccmdlpar_state){ \
    .optind = 1, \
    .chind = 0, \
    .isOptionsEnded = 0 \
  })

// Journeys in API design

// Data relavent to user:
// If we parse a non-option:
//   - The index within argv in which that non-option is at. This is
//   the only relevant information.
// If we parse a short option:
//   - The actual character for the short option which was found
//   - The index within argv in which that short option is at
//   - The index of the character within the command-line argument within argv in which that short option is at
//   If the short option takes an argument:
//     - The index of the option argument within argv.
// If we parse a long option:
//   - The index within argv in which the long option is at
//   If the long option takes an argument:
//     - The index of the option argument within argv.
//     - The return value specified in lopts that the long option with arguments is defined at
//     - The index within lopts where the long option is

// So we can see that in all of these cases, we care about the index within
// argv. Sometimes, this isn't just the value of 'optind' - 1 within the new
// state, as with options with arguments we would move optind by 2 for every
// invokation of `ccmdlpar`, not by 1. We also care about some return
// value which signifies meaning determined by the user to them.

// For short options, we then additionally have to handle:
//
// For long options, we then additionally have to handle:
//   The index within lopts for that specific long option.

// for any option arguments, we need to return an 'optarg' to the user somehow
// as well.

// For a long option which takes an argument, if the argument is attached
// to the long option, we can just signal that by having the index of the long
// option and the index of the option argument being equivalent. Then, the
// user can just iterate past the first '=' within the command-line argument
// to find it.
// We can also just explicitly provide an 'optarg' argument which already
// handles all of this. In most cases, the user probably doesn't care.

// Any error returned by ccmdlpar should be recoverable. This can just be
// done by skipping forward past erroneous command-line arguments and
// continuing parsing again. A caller of ccmdlpar should then decide:
// - if **any** erroneous behavior should result in them quitting their
//   application as fast as possible.
// - If, no matter if any erroneous behavior was found, the entire command-line
//   should still be processed. This may be desirable if the user application
//   would want to catch and log all errors parsing command-line arguments, then
//   exit the application after logging all errors. This would mean that
//   a user application would have to make sure that no undesirable side
//   effects happen when parsing command-line arguments where the application
//   won't run.
// This would mean that the erroneous case

// Now, if we implement any error causing the ccmdlpar parser to synchronize
// itself, how will the erroneous data be preserved so that it could be
// recovered by the user and logged or recovered in some way.

// We could have each error type correspond to some type of payload in a
// sum type. This would be advantageous if command-line parsing was actually
// difficult. However, it is not.
// We could just have a location for where each error occured. Perhaps we could
// use our ccmdlpar_parse_info struct as an in-band form of communication of where
// errors occured.


enum ccmdlpar_arg_type
{
  CCMDLPAR_ARG_TYPE_NON_OPTION,
  CCMDLPAR_ARG_TYPE_SHORT_OPTION,
  CCMDLPAR_ARG_TYPE_LONG_OPTION
};

const char* ccmdlpar_arg_type_str(enum ccmdlpar_arg_type argType);
const char* ccmdlpar_arg_type_prefix(enum ccmdlpar_arg_type argType);

// Error information is returned through ccmdlpar_parse_info as well.
struct ccmdlpar_parse_info
{
  //  On successful returns from `ccmdlpar`, holds the name of the argument
  //  parsed from the command-line.
  //
  //  Always valid on returns from ccmdlpar where ccmdlpar does returns
  //  CCMDLPAR_STATUS_OK
  //
  //  For an option, this represents the name of the option with its prefixed
  //  '-' or '--' removed. `arg` May not be
  //  NUL-terminated, and may not be equivalent to argv[optind], where optind
  //  is the same `optind` on the given `ccmdlpar_parse_info` struct.
  //  For example:
  //  - in "/README.md", arg[0..(argLen - 1)] = "/README.md"
  //  - in "--libPath=/usr/lib", arg[0..(argLen - 1)] = "libPath"
  //  - in "-a", arg[0..(argLen - 1)] = "a"
  const char* arg;

  // Always valid on returns from ccmdlpar
  //
  // The length of the string pointed to by 'arg'
  //
  // NOTE THAT 'arg[argLen]' does not necessarily equal to the NUL character!
  // For example, with '--libPath=/usr/lib',
  // `argLen` is equal to len("libPath") = 7, and `arg` points to the 'l' in
  // 'libPath'. This means 'arg' should not be used like a
  // NUL-terminated string.
  //
  // equals to 1 if argType == CCMDLPAR_ARG_TYPE_SHORT_OPTION
  int argLen;

  // Always valid on successful returns from ccmdlpar
  //
  // The type of command-line argument parsed.
  enum ccmdlpar_arg_type argType;

  // Always valid on successful returns from ccmdlpar
  //
  // The index within argv in which the argument 'arg' was parsed
  int optind;

  // The character from which the returned short option is at.
  //
  // Only meaningful when parsing short options
  // (argType == CCMDLPAR_ARG_TYPE_SHORT_OPTION), and is always in-bounds in
  // such a case.
  int chind;

  //  Only relevant when an option which takes an argument is parsed, which
  //  is determined by users of the `ccmdlpar` function
  //  through the `soptsWArgs` and `loptsWArgs` function parameters.
  //
  //  Set to NULL in cases where no optarg is necessary.
  //
  //  There is no 'optargLen' variable on ccmdlpar_parse_info as optarg is
  //  always NUL-terminated when an option argument is parsed.
  const char* optarg;

  //  Only relevant when an option which takes an argument is parsed, which
  //  is determined by users of the `ccmdlpar` function
  //  through the `soptsWArgs` and `loptsWArgs` function parameters.
  //
  //  Set to -1 in cases where no optarg is parsed.
  //
  //  DO NOT USE THIS AS REPLACEMENT TO `optarg` TO INDEX INTO `argv`.
  //  For example, if we have a command-line argument '--libPath=/usr/lib',
  //  `optarg` would point to '/usr/lib' and `argv[optArgInd]` would point to
  //  --libPath=/usr/lib. However, if we have a command-line argument sequence
  //  '--libPath /usr/lib', `optarg` and `argv[optArgInd]` would both point to
  //  '/usr/lib', as the option argument is in it's own command-line argument
  //  and is not attached to its long option.
  int optArgInd;
};

// Given the design of our API, there are only a few errors that we have
// that can actually happen at the command-line parsing level.
enum ccmdlpar_status
{
  CCMDLPAR_STATUS_OK = 0,


  CCMDLPAR_STATUS_ERR,

  // This should never actually be returned unless the caller passes invalid
  // data to `ccmdlpar`, such as NULL pointers for the ccmdlpar_state* and
  // ccmdlpar_parse_info* arguments.
  CCMDLPAR_STATUS_ERR_FATAL,


  // Note that, for almost all use-cases, the caller should proceed with parsing
  // until CCMDLPAR_STATUS_FINISHED is returned, not until CCMDLPAR_STATUS_OK is
  // not returned.
  CCMDLPAR_STATUS_FINISHED,

  // If an option argument was expected from a short/long option and was not
  // found, this status code will be emitted.
  CCMDLPAR_STATUS_MISSING_OPT_ARG,

  // If a short option which expects an option argument was not the last
  // argument in a chain of short options,
  // CCMDLPAR_STATUS_CHAINED_SHORT_OPT_NOT_LAST status code will be emitted.
  //
  // Since that is a mouthful of an explanation, if, for example, '-u'
  // expected an option argument, '-efuc' would be an invalid, since '-u' must
  // be last in order to have an option argument. However, '-efcu' would be
  // valid (assuming some command-line argument follows '-efcu' like in
  // '-efcu arg', otherwise it would be a
  // CCMDLPAR_STATUS_MISSING_OPT_ARG error).
  CCMDLPAR_STATUS_CHAINED_SHORT_OPT_NOT_LAST,
};

const char* ccmdlpar_status_str(enum ccmdlpar_status status);



// Simple wrapper to check if the parse info contains a short option which
// is equal to a given character 'ch'
int ccmdlpar_found_sopt(struct ccmdlpar_parse_info pi, char ch);

// Simple wrapper to check if the parse info contains a long option which
// is identified by a given NUL-terminated string ntstr
int ccmdlpar_found_lopt(struct ccmdlpar_parse_info pi, const char* ntstr);

// If within the ccmdlpar source tree, see ccmdlpar-template.c
// for example usage.


enum ccmdlpar_status ccmdlpar(
  // should be greater than 0
  int argc,
  // must be non-NULL and NULL terminated, where argv[argc] == NULL, just as
  // standardized by argv passed by the program.
  const char * const * argv,
  // must be non-NULL. The pointed to state should be initialized with
  // state->optind = 1 for parsing arguments given by argv in the main()
  // entry point function. This can be done with the CCMDLPAR_STATE_INIT macro.
  // through `struct ccmdlpar_state cmdlState = CCMDLPAR_STATE_INIT`.
  // Don't modify this state between calls.
  struct ccmdlpar_state * state,
  // A NUL-terminated string containing all short options which are expected to
  // take arguments
  const char* soptsWArg,
  // A NUL-terminated array of NUL-terminated strings containing all long
  // options which are expected to take arguments
  const char* const * loptsWArg,
  // Returns data for each parsed output.
  struct ccmdlpar_parse_info* outParseInfo
);


#endif
