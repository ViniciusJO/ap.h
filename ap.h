/*
  ap.h - Command-line argument parser using X-macros

  QUICK START:

  1. Define your flags and positional arguments using parameterized macros:

    // FLAG(NAME, SHORT_FORM, TYPE, REQUIRED, DEFAULT, DESCRIPTION, ...VARIANTS)
    #define FLAGS_LIST(FLAG)                                              \
      FLAG(verbose, v, bool, false, false, "Enable verbose output")      \
      FLAG(config, c, char*, false, "config.ini", "Config file path")    \
      FLAG(threads, t, int, false, 4, "Number of threads")              \
      FLAG(mode, m, char*, false, "fast", "Run mode", fast, safe, debug)

    // POS_ARG(NAME, REQUIRED, DEFAULT, DESCRIPTION, ...VARIANTS)
    #define POS_ARGS_LIST(POS_ARG)                                       \
      POS_ARG(input, true, "", "Input file")                             \
      POS_ARG(output, false, "out.txt", "Output file")

  2. Include the header and parse:

    #define AP_IMPLEMENTATIONS
    #include "ap.h"

    int main(int argc, char **argv) {
      Args args = ap_parse_args(argc, argv);

      printf("Verbose: %d\n", args.verbose);
      printf("Config: %s\n", args.config);
      printf("Threads: %d\n", args.threads);
      printf("Mode: %s\n", args.mode);
      printf("Input: %s\n", args.pos_args.input);
      printf("Output: %s\n", args.pos_args.output);
    }

  USAGE OUTPUT EXAMPLE:

    Usage: prog --config <config> --threads <threads> <input> [output]

    Arguments:
      help                           Shows this help message
      <input>                        Input file
      [output]                       Output file  (default: out.txt)

    Flags:
      -h, --help                     Shows this help message
      -v, --verbose    <bool>        Enable verbose output (default: false)
      -c, --config     <string>      Config file path (default: config.ini)
      -t, --threads    <int>         Number of threads (default: 4)
      -m, --mode       <string>      Run mode (default: fast) (variants: fast, safe, debug)

  COMMAND-LINE EXAMPLES:

    # Positional arguments only (use defaults for flags)
    $ prog input.txt

    # With flags
    $ prog --verbose --threads 8 input.txt output.txt

    # Short form flags
    $ prog -v -t 8 input.txt

    # Mixed short and long form
    $ prog -v --threads 8 -c custom.ini input.txt

    # Assign values with = syntax
    $ prog --threads=8 --config=custom.ini input.txt
    $ prog -t=8 -c=custom.ini input.txt

    # With -- to stop flag parsing (everything after is positional)
    $ prog input.txt -- --not-a-flag.txt

    # Built-in help subcommand
    $ prog help
    $ prog --help
    $ prog -h

    # Shell completions
    $ eval "$(prog completions zsh)"

  VARIANTS:

    Both flags and positional arguments support an optional variadic
    list of allowed values (variants). These are validated at parse
    time; invalid values cause an error with the allowed set printed.

    // Restrict flag values
    FLAG(mode, m, char*, false, "fast", "Run mode", fast, safe, debug)

    // Restrict positional argument values
    POS_ARG(action, true, "", "Action", start, stop, restart)

    // Restrict integer flag to specific values
    FLAG(DDD, , int, false, 1, "DDD code", 318, 350, 011)

    Usage message shows:  (variants: start, stop, restart)

    On invalid value:
      invalid_variant: unknown not in { start, stop, restart } allowed for action

  BUILT-IN SUBCOMMANDS:

    ap.h automatically handles two subcommands:

    help         - Prints usage message to stdout and exits
    completions  - Generates shell completion scripts

    These are detected before argument parsing, so they do not
    conflict with user-defined positional arguments.

  SHELL COMPLETIONS:

    Zsh completion generation is built in. The generated script
    includes flag names, short forms, descriptions, and variants
    for both flags and positional arguments.

    $ prog completions zsh     # Output zsh completion script
    $ prog completions bash    # (planned)
    $ prog completions fish    # (planned)

    Install for zsh:
      eval "$(prog completions zsh)"

    Or persist in .zshrc:
      eval "$(prog completions zsh)"

  FEATURES:

    - Automatic struct generation with all parsed arguments
    - Type support: bool, int, float, char, char* (strings), none (flags)
    - Required and optional arguments with defaults
    - Short (-f) and long (--flag) forms
    - Value assignment with = syntax: --flag=value and -f=value
    - Empty short form: leave it blank to disable (FLAG(name, , int, ...))
    - Automatic usage message generation with aligned columns
    - Variant validation for restricted value sets
    - Built-in --help / -h / help handling
    - Built-in completions subcommand with zsh support
    - Support for numeric bases: decimal, hex (0x), octal (0), binary (0b)
    - Option terminator (--) to stop flag parsing
    - Default flags: --help and -h are always available
    - Positional argument access via args.pos_args.NAME
    - Execution name in args.pos_args.exec_name

  OPTIONAL FEATURES:

    #define AP_ALLOW_EXTRAS    // Enable collection of extra positional arguments
    #define AP_NO_CLEAN        // Keep IS_NUM, IS_HEX, IS_BIN, IS_OCT macros
    #define AP_ATTRIBUTES      // Add custom attributes to generated functions

  BEHAVIOR OVERRIDES:

    These macros can be defined BEFORE including ap.h to change how
    the parser reports errors, exits, and outputs messages. This is
    useful for embedded systems, testing, or custom error handling
    where calling exit() or writing to stderr is not desired.

    #define AP_EXIT(code)      exit(code)     // Called on parse errors and help/completions
    #define AP_FPRINTF         fprintf        // Used for error and variant messages
    #define AP_ERR_FD          stderr         // File descriptor for error output
    #define AP_OUT_FD          stdout         // File descriptor for help and completions

    al.h (required dependency) also provides its own overrides:

    #define AL_ASSERT          assert         // Assertion for memory allocation failures
    #define AL_REALLOC         realloc        // Memory allocator for dynamic arrays
    #define AL_INIT_CAP        256            // Initial capacity for dynamic arrays
    #define AL_FPRINTF         fprintf        // Used for al_print and array error messages
    #define AL_EXIT(code)      exit(code)     // Called on array bounds errors

    Example: recover from parse errors instead of crashing:

      #include <setjmp.h>
      static jmp_buf ap_err_jmp;

      #define AP_EXIT(code) longjmp(ap_err_jmp, (code))
      #define AP_IMPLEMENTATIONS
      #include "ap.h"

      int main(int argc, char **argv) {
        int err = setjmp(ap_err_jmp);
        if (err) {
          printf("Parse failed with code %d, using defaults\n", err);
          return 1;
        }
        Args args = ap_parse_args(argc, argv);
        // ...
      }

    Example: silence all error output:

      #define AP_FPRINTF(fd, ...) ((void)0)
      #define AP_IMPLEMENTATIONS
      #include "ap.h"

    Example: redirect output to a custom log:

      #define AP_ERR_FD my_log_file
      #define AP_OUT_FD my_log_file
      #define AP_IMPLEMENTATIONS
      #include "ap.h"

  WITH AP_ALLOW_EXTRAS:

    Args args = ap_parse_args(argc, argv);

    // Access extra arguments beyond the defined positional args
    for (size_t i = 0; i < args.pos_args.extras.count; i++) {
      printf("Extra %zu: %s\n", i, args.pos_args.extras.items[i]);
    }

  MACRO FORMAT:

    FLAGS_LIST and POS_ARGS_LIST are parameterized macros that receive
    the inner macro name as argument. This allows ap.h to redefine the
    inner macro multiple times for struct generation, parsing, and usage.

    // Flags: FLAG(NAME, SHORT_FORM, TYPE, REQUIRED, DEFAULT, DESCRIPTION, ...VARIANTS)
    #define FLAGS_LIST(FLAG)                                              \
      FLAG(verbose, v, bool, false, false, "Enable verbose output")      \
      FLAG(config, c, char*, false, "config.ini", "Config file path")

    // Leave short form empty to disable it
    FLAG(long_only, , int, false, 0, "No short form for this flag")

    // none type creates boolean flags (no argument consumed)
    FLAG(debug, d, none, false, , "Enable debug mode")

    // Positional args: POS_ARG(NAME, REQUIRED, DEFAULT, DESCRIPTION, ...VARIANTS)
    #define POS_ARGS_LIST(POS_ARG)                                       \
      POS_ARG(input, true, "", "Input file")                             \
      POS_ARG(output, false, "out.txt", "Output file")

    // Empty default: leave it blank
    POS_ARG(target, false, , "Target name")

  NOTES:

    - Requires al.h (array list implementation)
    - On missing required arguments, program exits with error and usage
    - On invalid argument type, program exits with error and usage
    - On invalid variant value, program exits with the allowed set
    - Usage message printed to stderr on parse errors, stdout on help
    - exec_name always set to argv[0]
    - Default values for flags use AP_DEFAULT_OR_ZERO to handle empty defaults
    - Default values for positional args applied only when not provided by user

  TODO:

    - FIX completions for bash
    - FIX completions for fish
    - Recognize flag concatenation ?? , so that -f -l -a could be -fla
      (behind a macro to enable or disable it, disabled by default)
*/

#ifndef __AP_H__
#define __AP_H__
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "al.h"

#ifndef AP_ATTRIBUTES
#define AP_ATTRIBUTES
#endif // FLAGS_ATTRIBUTES

#ifndef AP_EXIT
extern void exit(int status);
#define AP_EXIT exit
#endif//AP_EXIT

#ifndef AP_FPRINTF
extern int fprintf(FILE *__restrict stream, const char *__restrict format, ...) __nonnull ((1));
#define AP_FPRINTF fprintf
#endif//AP_FPRINTF


#ifndef AP_OUT_FD
#define AP_OUT_FD stdout
#endif//AP_OUT_FD

#ifndef AP_ERR_FD
#define AP_ERR_FD stderr
#endif//AP_ERR_FD

#ifndef FLAGS_LIST
#define FLAGS_LIST(FLAG) FLAG(default_flag, , none, false, , "desc")
#endif // FLAGS_LIST(FLAG)

#define FLAGS_DEFAULT_LIST(FLAG) \
  FLAG(help, h, none, false, , "Shows this help message")


#ifndef POS_ARGS_LIST
#define POS_ARGS_LIST(POS_ARG) POS_ARG(default_arg, false, , "desc")
#endif // FLAGS_LIST(FLAG)

#define POS_ARGS_DEFAULT_LIST \
  POS_ARG(help, false, , "Shows this help message")

typedef bool none;

typedef struct {
  char **items;
  size_t count;
  size_t capacity;
} AP_extras;

typedef struct {
  char *exec_name;
#ifdef POS_ARGS_LIST
#define POS_ARG(NAME, REQUIRED, DEFAULT, DESC, ...) char *NAME;
  POS_ARGS_LIST(POS_ARG)
#undef POS_ARG
#endif//POS_ARGS_LIST(POS_ARG)
#ifdef AP_ALLOW_EXTRAS
  AP_extras extras;
#endif//AP_ALLOW_EXTRAS
} PosArgs;

typedef struct {
#ifdef FLAGS_LIST
#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT, DESC, ...) TYPE NAME;
  FLAGS_LIST(FLAG)
#undef FLAG
#endif//FLAGS_LIST(FLAG)
  PosArgs pos_args;
} Args;

// static const char ArgsSequence[] = {
// #define POS_ARG(NAME, TYPE, REQUIRED, DEFAULT, ...) #NAME,
//   POS_ARGS_LIST(POS_ARG)
// #undef POS_ARG
// };

enum ArgsSequence {
#define POS_ARG(NAME, REQUIRED, DEFAULT, DESC, ...) NAME,
  POS_ARGS_LIST(POS_ARG)
#undef POS_ARG
  ARGS_COUNT
};

/**
 * Print the usage message to the given file descriptor.
 *
 * Outputs the program name, required flags, positional arguments,
 * descriptions, defaults, and variants. Flags from FLAGS_DEFAULT_LIST
 * (--help, -h) are always included.
 *
 * @param fd  File descriptor to write to (e.g. stdout, stderr, or AP_OUT_FD / AP_ERR_FD)
 */
AP_ATTRIBUTES void ap_usage(FILE *fd);

/**
 * Parse command-line arguments defined in FLAGS_LIST and POS_ARGS_LIST.
 *
 * Handles built-in subcommands (help, completions) before parsing.
 * Validates required flags and positional arguments, checks variant
 * constraints, and applies default values for optional arguments.
 * On error, prints a message to AP_ERR_FD and calls AP_EXIT(1).
 *
 * @param argc  Argument count from main
 * @param argv  Argument vector from main
 * @return Args struct with all parsed flags and positional arguments
 */
AP_ATTRIBUTES Args ap_parse_args(int argc, char **argv);

/**
 * Generate a zsh completion script for the current program.
 *
 * Outputs a compdef-based script with flag names, short forms,
 * descriptions, and variant values for both flags and positional
 * arguments. Install with: eval "$(prog completions zsh)"
 *
 * @param fd  File descriptor to write the script to (typically AP_OUT_FD)
 */
AP_ATTRIBUTES void ap_completions_zsh(FILE *fd);

/**
 * Generate a bash completion script for the current program.
 *
 * Outputs a complete/compgen-based script with flag names, short
 * forms, and positional argument completions.
 * Install with: eval "$(prog completions bash)"
 *
 * @param fd  File descriptor to write the script to (typically AP_OUT_FD)
 */
AP_ATTRIBUTES void ap_completions_bash(FILE *fd);

/**
 * Generate a fish completion script for the current program.
 *
 * Outputs a series of complete commands with flag names, short
 * forms, and descriptions.
 * Install with: prog completions fish | source
 *
 * @param fd  File descriptor to write the script to (typically AP_OUT_FD)
 */
AP_ATTRIBUTES void ap_completions_fish(FILE *fd);

#ifdef __cplusplus
}
#endif //__cplusplus
#endif //__AP_H__

#define AP_IMPLEMENTATIONS
#ifdef AP_IMPLEMENTATIONS
#ifndef __AP_IMP__
#define __AP_IMP__
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

static char *exec_name = (char*)"ap.h";

typedef struct {
  char **items;
  size_t count;
  size_t capacity;
} Variants;

// Trim leading and trailing whitespace
AP_ATTRIBUTES char *ap_trim_whitespace(const char *str) {
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) return strdup("");

    const char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    size_t len = end - str + 1;
    char *result = (char *)malloc(len + 1);
    strncpy(result, str, len);
    result[len] = '\0';

    return result;
}


// Split function
AP_ATTRIBUTES Variants ap_split_string(const char *input) {
    char *copy = strdup(input);
    char *token = strtok(copy, ",");
    Variants result = {0};

    while (token != NULL) {
      al_append(&result, ap_trim_whitespace(token));
      token = strtok(NULL, ",");
    }

    free(copy);
    return result;
}



// usage: ./bin 
AP_ATTRIBUTES void ap_usage(FILE *fd) {
  
  size_t max_pa_name = 0, max_flag_name = 0, max_sf = 0, max_typename = 0;

  fprintf(fd, "Usage: %s ", exec_name);

#define POS_ARG(NAME, REQUIRED, DEFAULT, DESC, ...) if(strlen(#NAME) > max_pa_name) max_pa_name = strlen(#NAME);
  POS_ARGS_LIST(POS_ARG)
#undef POS_ARG

#define HAS_SF(SF) (strcmp(SF, "-") && strlen(SF) > 0)
#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT, DESC, ...) if(REQUIRED) fprintf(fd, "--%s <%s> ",                         \
  #NAME, #NAME                                                                                             \
);
  FLAGS_LIST(FLAG)
#undef FLAG
#undef HAS_SF

#define POS_ARG(NAME, REQUIRED, DEFAULT, DESC, ...) fprintf(fd, "%c%s%c ", REQUIRED ? '<' : '[', #NAME, REQUIRED ? '>' : ']');
  POS_ARGS_LIST(POS_ARG)
#undef POS_ARG

  fprintf(fd, "\n\nArguments:\n");


#define POS_ARG(NAME, REQUIRED, DEFAULT, DESC, ...) fprintf(fd, "\t%c%s%c%-*s  %s%s%s\n",                         \
  REQUIRED ? '<' : '[', #NAME , REQUIRED ? '>' : ']', (int)(max_pa_name - strlen(#NAME)+1), "",                             \
  DESC, strlen(#DEFAULT) != 0 ? "  (default: " #DEFAULT ")" : "",                           \
  (0 != strlen(#__VA_ARGS__)) ? "  (variants: " #__VA_ARGS__ ")": ""                                                        \
);
  POS_ARGS_DEFAULT_LIST
  POS_ARGS_LIST(POS_ARG)
#undef POS_ARG

  fprintf(fd, "\nFlags:\n");


#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT, DESC, ...) if(strlen(#NAME) > max_flag_name) max_flag_name = strlen(#NAME); if(strlen(#SF) > max_sf) max_sf = strlen(#SF);
  FLAGS_LIST(FLAG)
#undef FLAG

  size_t typename_len = 0;
#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT, DESC, ...) \
  typename_len = strlen((!strcmp(#TYPE, "char*") || !strcmp(#TYPE, "char *")) ? "<string>" : !strcmp(#TYPE, "none") ? "" : "<" #TYPE ">"); \
  if(typename_len > max_typename) max_typename = typename_len;
  FLAGS_LIST(FLAG)
#undef FLAG

#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT, DESC, ...) fprintf(fd, "\t%c%-*s%c --%-*s  %-*s  %s %s%s%c %s\n",          \
  !strcmp(#SF, "-") || strlen(#SF) == 0 ? ' ' : '-', (int)max_sf, #SF, !strcmp(#SF, "-") || strlen(#SF) == 0 ? ' ' : ',', \
  (int)max_flag_name, #NAME,                                                                                              \
  (int)max_typename, \
  (!strcmp(#TYPE, "char*") || !strcmp(#TYPE, "char *")) ? "<string>" : !strcmp(#TYPE, "none") ? "" : "<" #TYPE ">",                                               \
  DESC,                                                                                                                   \
  strlen(#DEFAULT) == 0 ? "" : "(default: ", #DEFAULT, strlen(#DEFAULT) == 0 ? ' ' : ')',                                 \
  (0 != strlen(#__VA_ARGS__)) ? "(variants: " #__VA_ARGS__ ")": ""                                                        \
);
  FLAGS_DEFAULT_LIST(FLAG)
  FLAGS_LIST(FLAG)
#undef FLAG

  fprintf(fd, "\n");
}

AP_ATTRIBUTES char *ap_coma_to_space_separeted_list(const char * const list) {
  char *copy = strdup(list);
  size_t len = strlen(copy);
  for(size_t i = 0; i < len; i++) if(',' == copy[i]) copy[i] = ' ';
  return copy;
}

AP_ATTRIBUTES char *ap_positional_completion_variants(const char * const list) {
  Variants vs = ap_split_string(list);

  char *buf;
  size_t buflen;
  FILE *out = open_memstream(&buf, &buflen);

  al_foreach(i, &vs) fprintf(out, "\t\t\t'%s' \\\n", *i);
  fclose(out);
  return buf;

  return buf;
}

AP_ATTRIBUTES void ap_completions_zsh(FILE *fd) {
  fprintf(fd, "#compdef %s\n\n_%s() {\n\tlocal context state\n\n", exec_name, exec_name);
  // fprintf(fd, "\tprint -r -- \"STATE=$state CURRENT=$CURRENT WORDS=$words\"\n\n");
  fprintf(fd, "\t_arguments -s \\\n");


  #define HAS_VARIANTS(VA) (VA && 0 != strlen(VA))
  #define HAS_SF(SF) (strlen(#SF) > 0 && strcmp(#SF, "-"))
  #define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT, DESC, ...) fprintf(fd, "\t\t%s%s" #NAME ":%s%s%s' \\\n",         \
     (HAS_SF(SF)) ? "'(-" #SF " --" #NAME ")'{-" #SF ",--" #NAME "}'" : "'--" #NAME,              \
     (strlen(DESC) > 0) ? "[" DESC "]:"  : "", \
     HAS_VARIANTS(#__VA_ARGS__) ? "(" : "", \
     HAS_VARIANTS(#__VA_ARGS__) ? ap_coma_to_space_separeted_list(#__VA_ARGS__) : "", \
     HAS_VARIANTS(#__VA_ARGS__) ? ")" : "" \
  );
    FLAGS_LIST(FLAG)
  #undef FLAG
  #undef HAS_SF

  size_t count = 1;
  
#define POS_ARG(NAME, REQUIRED, DEFAULT, DESC, ...) fprintf(fd, "\t\t'%ld:" #NAME ":->" #NAME "' \\\n", count++);
  POS_ARGS_LIST(POS_ARG)
#undef POS_ARG
  fprintf(fd, "\t\t'*::_args:->_args' && return\n");


  fprintf(fd, "\n\tcase $state in\n");

#define POS_ARG(NAME, REQUIRED, DEFAULT, DESC, ...) fprintf(fd, "\t\t" #NAME ")\n\t\t\t%s %s\n\t\t\treturn\n\t\t\t;;\n\n", \
  HAS_VARIANTS(#__VA_ARGS__) \
    ? ((0 != strlen(DESC)) ? "_values '" DESC "'" : "_values '" #NAME "'") \
    : ((0 != strlen(DESC)) ? "_message '" DESC "'" : ""), \
    ap_coma_to_space_separeted_list(#__VA_ARGS__) \
);
  POS_ARGS_LIST(POS_ARG)
#undef POS_ARG
  fprintf(fd, "\t\t_args)\n\t\t\treturn\n\t\t\t;;\n");

  fprintf(fd, "\tesac\n\treturn\n}\n\ncompdef _%s %s\n", exec_name, exec_name);
}

AP_ATTRIBUTES void ap_completions_bash(FILE *fd) {
  fprintf(fd, "# bash completions for %s\n", exec_name);
  fprintf(fd, "_%s() {\n", exec_name);
  fprintf(fd, "\tlocal cur prev words cword\n");
  fprintf(fd, "\t_init_completion || return\n\n");

  // Flag value completions based on previous word
  fprintf(fd, "\tcase \"$prev\" in\n");
#define HAS_VARIANTS(VA) (VA && 0 != strlen(VA))
#define HAS_SF(SF) (strlen(#SF) > 0 && strcmp(#SF, "-"))
#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT, DESC, ...) \
  if (HAS_VARIANTS(#__VA_ARGS__)) { \
    fprintf(fd, "\t\t--" #NAME); \
    if (HAS_SF(SF)) fprintf(fd, "|-" #SF); \
    fprintf(fd, ")\n"); \
    fprintf(fd, "\t\t\tCOMPREPLY=($(compgen -W '"); \
    { \
      Variants _vs = ap_split_string(#__VA_ARGS__); \
      al_foreach(_v, &_vs) fprintf(fd, "%s ", *_v); \
      al_free(&_vs); \
    } \
    fprintf(fd, "' -- \"$cur\"))\n"); \
    fprintf(fd, "\t\t\treturn\n"); \
    fprintf(fd, "\t\t\t;;\n"); \
  }
  FLAGS_LIST(FLAG)
#undef FLAG
#undef HAS_SF
  fprintf(fd, "\tesac\n\n");

  // Positional argument completions
  // Count how many positional args have been given so far
  fprintf(fd, "\t# Count positional arguments (skip flags and their values)\n");
  fprintf(fd, "\tlocal pos_count=0 skip_next=0\n");
  fprintf(fd, "\tfor ((i=1; i < cword; i++)); do\n");
  fprintf(fd, "\t\tif [[ $skip_next -eq 1 ]]; then\n");
  fprintf(fd, "\t\t\tskip_next=0\n");
  fprintf(fd, "\t\t\tcontinue\n");
  fprintf(fd, "\t\tfi\n");
  fprintf(fd, "\t\tcase \"${words[i]}\" in\n");

#define HAS_SF(SF) (strlen(#SF) > 0 && strcmp(#SF, "-"))
#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT, DESC, ...) \
  if (strcmp(#TYPE, "none")) { \
    fprintf(fd, "\t\t\t--" #NAME); \
    if (HAS_SF(SF)) fprintf(fd, "|-" #SF); \
    fprintf(fd, ") skip_next=1 ;;\n"); \
  } else { \
    fprintf(fd, "\t\t\t--" #NAME); \
    if (HAS_SF(SF)) fprintf(fd, "|-" #SF); \
    fprintf(fd, ") ;;\n"); \
  }
  FLAGS_LIST(FLAG)
#undef FLAG
#undef HAS_SF

  fprintf(fd, "\t\t\t-*) ;;\n");
  fprintf(fd, "\t\t\t*) ((pos_count++)) ;;\n");
  fprintf(fd, "\t\tesac\n");
  fprintf(fd, "\tdone\n\n");

  // Positional completion by index
  fprintf(fd, "\tcase \"$pos_count\" in\n");
  {
    int idx = 0;
#define POS_ARG(NAME, REQUIRED, DEFAULT, DESC, ...) \
    fprintf(fd, "\t\t%d) # " #NAME ": " DESC "\n", idx++); \
    if (HAS_VARIANTS(#__VA_ARGS__)) { \
      fprintf(fd, "\t\t\tCOMPREPLY=($(compgen -W '"); \
      { \
        Variants _vs = ap_split_string(#__VA_ARGS__); \
        al_foreach(_v, &_vs) fprintf(fd, "%s ", *_v); \
        al_free(&_vs); \
      } \
      fprintf(fd, "' -- \"$cur\"))\n"); \
    } else { \
      fprintf(fd, "\t\t\tCOMPREPLY=()\n"); \
    } \
    fprintf(fd, "\t\t\t;;\n");
    POS_ARGS_LIST(POS_ARG)
#undef POS_ARG
  }
  fprintf(fd, "\tesac\n\n");

  // Flag completions when current word starts with -
  fprintf(fd, "\tif [[ \"$cur\" == -* ]]; then\n");
  fprintf(fd, "\t\tCOMPREPLY=($(compgen -W '");

#define HAS_SF(SF) (strlen(#SF) > 0 && strcmp(#SF, "-"))
#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT, DESC, ...) \
  fprintf(fd, "--" #NAME " "); \
  if (HAS_SF(SF)) fprintf(fd, "-" #SF " ");
  FLAGS_LIST(FLAG)
#undef FLAG
#undef HAS_SF

  fprintf(fd, "--help -h");
  fprintf(fd, "' -- \"$cur\"))\n");
  fprintf(fd, "\tfi\n");

  fprintf(fd, "}\n\n");
  fprintf(fd, "complete -F _%s %s\n", exec_name, exec_name);
}

AP_ATTRIBUTES void ap_completions_fish(FILE *fd) {
  fprintf(fd, "# fish completions for %s\n\n", exec_name);

  // Disable file completions by default
  fprintf(fd, "complete -c %s -f\n\n", exec_name);

  // Help flag
  fprintf(fd, "complete -c %s -s h -l help -d 'Shows this help message'\n", exec_name);

  // Flags
#define HAS_SF(SF) (strlen(#SF) > 0 && strcmp(#SF, "-"))
#define HAS_VARIANTS(VA) (VA && 0 != strlen(VA))
#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT, DESC, ...) \
  fprintf(fd, "complete -c %s", exec_name); \
  if (HAS_SF(SF)) fprintf(fd, " -s " #SF); \
  fprintf(fd, " -l " #NAME); \
  if (strcmp(#TYPE, "none")) fprintf(fd, " -r"); \
  if (strlen(DESC) > 0) fprintf(fd, " -d '%s'", DESC); \
  fprintf(fd, "\n"); \
  if (HAS_VARIANTS(#__VA_ARGS__) && strcmp(#TYPE, "none")) { \
    Variants _vs = ap_split_string(#__VA_ARGS__); \
    al_foreach(_v, &_vs) { \
      fprintf(fd, "complete -c %s -n '__fish_seen_argument -l " #NAME, exec_name); \
      if (HAS_SF(SF)) fprintf(fd, " -s " #SF); \
      fprintf(fd, "' -a '%s'\n", *_v); \
    } \
    al_free(&_vs); \
  }
  FLAGS_LIST(FLAG)
#undef FLAG
#undef HAS_SF

  fprintf(fd, "\n");

  // Subcommands: help and completions
  fprintf(fd, "# Subcommands\n");
  fprintf(fd, "complete -c %s -n '__fish_use_subcommand' -a help -d 'Shows this help message'\n", exec_name);
  fprintf(fd, "complete -c %s -n '__fish_use_subcommand' -a completions -d 'Generate shell completions'\n", exec_name);
  fprintf(fd, "complete -c %s -n '__fish_seen_subcommand_from completions' -a 'zsh bash fish'\n", exec_name);

  fprintf(fd, "\n");

  // Positional arguments with variants
#define POS_ARG(NAME, REQUIRED, DEFAULT, DESC, ...) \
  if (HAS_VARIANTS(#__VA_ARGS__)) { \
    fprintf(fd, "# " #NAME ": " DESC "\n"); \
    { \
      Variants _vs = ap_split_string(#__VA_ARGS__); \
      al_foreach(_v, &_vs) { \
        fprintf(fd, "complete -c %s -a '%s' -d '" #NAME "'\n", exec_name, *_v); \
      } \
      al_free(&_vs); \
    } \
  }
  POS_ARGS_LIST(POS_ARG)
#undef POS_ARG
#undef HAS_VARIANTS
}

struct Parsed_Args {
#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT, DESC, ...) bool NAME;
  FLAGS_LIST(FLAG)
#undef FLAG
};

#define IS_NUM(C) ((C) >= '0' && (C) <= '9')
#define IS_HEX(C)                                                                                                         \
  (((C) >= '0' && (C) <= '9') || ((C) >= 'a' && (C) <= 'f') ||                                                            \
   ((C) >= 'A' && (C) <= 'F'))
#define IS_BIN(C) ((C) == '0' || (C) == '1')
#define IS_OCT(C) ((C) >= '0' && (C) <= '7')
AP_ATTRIBUTES int get_numeric_string_base(char *str) {
  int start_pos = (str[0] == '-' || str[0] == '+') ? 1 : 0;
  if (str[start_pos] == '0') {
    switch (str[1]) {
    case 'b': { // BINARY
      start_pos += 2;
      for (size_t i = start_pos; i < strlen(str); i++)
        if (!IS_BIN(str[start_pos + i]))
          return -1;
      return 2;
    }
    case 'x':
    case 'h': { // HEX
      start_pos += 2;
      for (size_t i = start_pos; i < strlen(str); i++)
        if (!IS_HEX(str[start_pos + i]))
          return -1;
      return 16;
    }
    default: { // OCTAL
      for (size_t i = start_pos; i < strlen(str); i++)
        if (!IS_OCT(str[start_pos + i]))
          return -1;
      return 8;
    }
    }
  } else { // DECIMAL
    for (size_t i = start_pos; i < strlen(str); i++)
      if (!IS_NUM(str[start_pos + i]))
        return -1;
    return 10;
  }
}

typedef struct {
  char type;
  union { bool b; int i; float f; char c; char * s; } value;
} ArgValue;

AP_ATTRIBUTES ArgValue __parse_arg(char *name, char *type, int argc, char **argv, int i, bool required) {
  ArgValue ret = {0};
  if (!strcmp(type, "none")) {
    ret.value.b = true;
    ret.type = 'b';
    return ret;
  }
  if (argc < i + 2) {
    if (required) {
      fprintf(AP_ERR_FD, "ERROR: Expected argument for --%s\n\n", name);
      ap_usage(AP_ERR_FD);
      AP_EXIT(1);
    }
  } else if (!strcmp(type, "bool")) {

    if (!strcmp(argv[i + 1], "true")) {
      ret.value.b = true;
      ret.type = 'b';
    } else if (!strcmp(argv[i + 1], "false")) {
      ret.value.b = false;
      ret.type = 'b';
    } else {
      fprintf(AP_ERR_FD, "ERROR: Invalid argument %s, expected: %s\n\n", argv[i + 1], "bool");
      ap_usage(AP_ERR_FD);
      AP_EXIT(1);
    }
  } else if (!strcmp(type, "int")) {

    int base = get_numeric_string_base(argv[i + 1]);
    if (base <= 0) {
      fprintf(AP_ERR_FD, "ERROR: Invalid argument %s, expected: %s\n\n", argv[i + 1], "int");
      ap_usage(AP_ERR_FD);
      AP_EXIT(1);
    }
    char *start = argv[i + 1];
    ret.value.i = (int)strtol(start, NULL, base);
    ret.type = 'i';
    return ret;
  } else if (!strcmp(type, "float")) {

    char *endptr = NULL;
    ret.value.f = strtof(argv[i + 1], &endptr);
    ret.type = 'f';
    if (ret.value.f == 0.0f && endptr == argv[i + 1]) {
      fprintf(AP_ERR_FD, "ERROR: Invalid argument %s, expected: %s\n\n", argv[i + 1], "float");
      ap_usage(AP_ERR_FD);
      AP_EXIT(1);
    };
    return ret;

  } else if (!strcmp(type, "char")) {

    ret.value.c = argv[i + 1][0];
    ret.type = 'c';
    return ret;

  } else if (!strcmp(type, "char*") || !strcmp(type, "char *")) {

    ret.value.s = argv[i + 1];
    ret.type = 's';
    return ret;

  } else { assert(false && "UNRECHEABLE: no flag type"); }

  return ret;
}

AP_ATTRIBUTES void ap_check_variants(char* arg_name, char *value, char *variants) {
  Variants vs = ap_split_string(variants);
  al_foreach(i, &vs) if(0 == strcmp(value, *i)) {
    // AL_FPRINTF(AP_ERR_FD, "\t => %s,\n", *i);
    al_free(&vs);
    return;
  }
  al_free(&vs);
  AL_FPRINTF(AP_ERR_FD, "invalid_variant: %s not in { %s } allowed for %s\n", value, variants, arg_name);
  AP_EXIT(1);
}

typedef Variants ArgsAL;

AP_ATTRIBUTES Args ap_parse_args(int argc, char **argv) {
  
  exec_name = argv[0];

  ArgsAL args = {0};

  for(int i = 0; i < argc; i++) {
    char * arg = strdup(argv[i]);
    const size_t arg_len = strlen(arg);
    size_t equal_idx = 0;
    for(; equal_idx < arg_len; equal_idx++)
      if('=' == arg[equal_idx]) break;
    al_append(&args, arg);
    if(equal_idx < arg_len) {
      arg[equal_idx] = 0;      
      al_append(&args, arg+equal_idx+1);
    }
  }

  if(args.count >= 2) {
    if(0 == strcmp(args.items[1], "help")) {
      ap_usage(AP_OUT_FD);
      AP_EXIT(0);
    } else if(0 == strcmp(args.items[1], "completions")) {
      if(args.count < 3) {
        // TODO: inform correct usage of completions
        ap_usage(AP_OUT_FD);
        AP_EXIT(1);
      }
      else if(0 == strcmp(args.items[2], "zsh")) ap_completions_zsh(AP_OUT_FD);
      else if(0 == strcmp(args.items[2], "bash")) ap_completions_bash(AP_OUT_FD);
      else if(0 == strcmp(args.items[2], "fish")) ap_completions_fish(AP_OUT_FD);
      else {
        // TODO: inform correct usage of completions
        ap_usage(AP_OUT_FD);
        AP_EXIT(1);
      }
      AP_EXIT(0);
    }
  }

  int c = 0;
  // PosArgs args = {0};
  Args ret = {0};
  struct Parsed_Args parsed = {0};

  // defaults
#define AP_GET_SECOND(A, B, ...) B
#define AP_DEFAULT_OR_ZERO_IMPL(...) AP_GET_SECOND(_, ##__VA_ARGS__, 0)
#define AP_DEFAULT_OR_ZERO(...) AP_DEFAULT_OR_ZERO_IMPL(__VA_ARGS__)

#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT, DESC, ...) if(0 != strlen(#DEFAULT)) ret.NAME = (TYPE)AP_DEFAULT_OR_ZERO(DEFAULT);
  FLAGS_LIST(FLAG)
#undef FLAG

#undef AP_GET_SECOND
#undef AP_DEFAULT_OR_ZERO_IMPL
#undef AP_DEFAULT_OR_ZERO

#define VERIFY_VARIANTS(ARG, VARIANTS) 
#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT, DESC, ...)                                                                \
  else if (!options_terminated && (!strcmp(args.items[i], "--" #NAME) || !strcmp(args.items[i], "-" #SF))) {                          \
    if(0 != strcmp(#TYPE, "none") && 0 != strlen(#__VA_ARGS__)) ap_check_variants(#NAME, args.items[i+1], #__VA_ARGS__);        \
    ArgValue av = __parse_arg((char*)#NAME, (char*)#TYPE, args.count, args.items, i, REQUIRED);                                       \
    switch (av.type) {                                                                                                    \
      case 'b': ret.NAME = *(TYPE*)&av.value.b; break;                                                                    \
      case 'i': ret.NAME = *(TYPE*)&av.value.i; break;                                                                    \
      case 'f': ret.NAME = *(TYPE*)&av.value.f; break;                                                                    \
      case 'c': ret.NAME = *(TYPE*)&av.value.c; break;                                                                    \
      case 's': ret.NAME = *(TYPE*)&av.value.s; break;                                                                    \
    };                                                                                                                    \
    if(0 != strcmp(#TYPE, "none")) ++i;                                                                                   \
    parsed.NAME = true;                                                                                                   \
  }

  ret.pos_args.exec_name = args.items[0];

  bool options_terminated = false;

  for (size_t i = 1; i < args.count; i++) {

    if(!options_terminated && !strcmp("--", args.items[i])) options_terminated = true;
    else if(!options_terminated && (0 == strcmp(args.items[i], "-h") || (0 == strcmp(args.items[i], "--help")))) { ap_usage(AP_OUT_FD); AP_EXIT(0); }
    FLAGS_LIST(FLAG)
    else {
      if(!options_terminated && args.items[i][0] == '-' && !IS_NUM(args.items[i][1])) {
        fprintf(AP_ERR_FD, "ERROR: unknown flag %s\n\n", args.items[i]);
        ap_usage(AP_ERR_FD);
        AP_EXIT(1);
      } else { // POSITIONAL
        if(c < ARGS_COUNT) {
          switch(c) {
#define POS_ARG(NAME, REQUIRED, DEFAULT, DESC, ...) case NAME: {                                                          \
  if(0 != strlen(#__VA_ARGS__)) ap_check_variants(#NAME, args.items[i], #__VA_ARGS__);                                      \
  ret.pos_args.NAME = args.items[i];                                                                                            \
} break;
            POS_ARGS_LIST(POS_ARG)
#undef POS_ARG
          }
          c++;
        } else {
#ifdef AP_ALLOW_EXTRAS
          al_append(&ret.pos_args.extras, args.items[i]);
#else
          fprintf(AP_ERR_FD, "ERROR: unknown argument %s\n\n", args.items[i]);
          ap_usage(AP_ERR_FD);
          AP_EXIT(1);
#endif
        }
      }
    }
  }

#undef FLAG

#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT, DESC, ...)                                                                \
  if (REQUIRED && !parsed.NAME) {                                                                                         \
    fprintf(AP_ERR_FD, "ERROR: Flag --" #NAME " required but not found\n\n");                                                \
    ap_usage(AP_ERR_FD);                                                                                                     \
    AP_EXIT(1);                                                                                                              \
  }
  FLAGS_LIST(FLAG)
#undef FLAG

  c = 0;
#define POS_ARG(NAME, REQUIRED, DEFAULT, DESC, ...) if (REQUIRED) {                                                       \
  if(ret.pos_args.NAME == NULL) {                                                                                         \
    fprintf(AP_ERR_FD, "ERROR: Positional arg <" #NAME "> (%d) required but not found\n\n", c);                              \
    ap_usage(AP_ERR_FD);                                                                                                     \
    AP_EXIT(1);                                                                                                              \
  }                                                                                                                       \
  c++;                                                                                                                    \
} else goto unrequired;
  POS_ARGS_LIST(POS_ARG)
#undef POS_ARG

unrequired:
  for (int i = c; i < ARGS_COUNT; ++i) {
    // printf("%d: ---------------\n", i);
    switch(i) {
#define POS_ARG(NAME, REQUIRED, DEFAULT, DESC, ...)                                                                       \
      case NAME: { if(0 != strlen(#DEFAULT) && ret.pos_args.NAME == NULL) ret.pos_args.NAME = DEFAULT "\0"; } break;
      POS_ARGS_LIST(POS_ARG)
#undef POS_ARG
    }
  }

  return ret;
}

#ifndef AP_NO_CLEAN
#undef IS_NUM
#undef IS_HEX
#undef IS_BIN
#undef IS_OCT
#endif


#ifdef __cplusplus
}
#endif //__cplusplus
#endif //__AP_IMP__
#undef AP_IMPLEMENTATIONS
#endif // AP_IMPLEMENTATIONS

// union ArgValue { bool b; int i; float f; char c; char * s; }; enum ArgType {
// BOOL, INT, FLOAT, LONG, CHAR, STRING, , }; typedef struct { char* name; char*
// short_form; ArgType type; ArgValue default_value; bool required; } Arg;
