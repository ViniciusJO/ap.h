#ifndef __AP_H__
#define __AP_H__
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#ifndef AP_ATTRIBUTES
#define AP_ATTRIBUTES
#endif // FLAGS_ATTRIBUTES

#include <stdbool.h>

#ifndef FLAGS_LIST
#define FLAGS_LIST FLAG(default_flag, _, none, false, NULL)
#endif // FLAGS_LIST

#ifndef ARGS_LIST
#define ARGS_LIST ARG(default_arg, char *, false, NULL)
#endif // FLAGS_LIST

typedef bool none;

typedef struct {
#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT) TYPE NAME;
  FLAGS_LIST
#undef FLAG
} Flags;

// struct ___Short_Flags {
// #define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT) TYPE SF;
//   FLAGS_LIST
// #undef FLAG
// };

typedef struct {
  char *exec_name;
#define ARG(NAME, TYPE, REQUIRED, DEFAULT) TYPE NAME;
  ARGS_LIST
#undef ARG
} Args;

typedef struct {
  Args args;
  Flags flags;
} Input;

static const char ArgsSequence[] = {
#define ARG(NAME, TYPE, REQUIRED, DEFAULT) #NAME,
  ARGS_LIST
#undef ARG
};

AP_ATTRIBUTES Input parse_flags(int argc, char **argv, char *error_msg);

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Parsed_Args {
#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT) bool NAME;
  FLAGS_LIST
#undef FLAG
};

#define IS_NUM(C) ((C) >= '0' && (C) <= '9')
#define IS_HEX(C)                                                              \
  (((C) >= '0' && (C) <= '9') || ((C) >= 'a' && (C) <= 'f') ||                 \
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

AP_ATTRIBUTES Input parse_flags(int argc, char **argv, char *error_msg) {
  (void)error_msg;
  Args args = {0};
  Flags ret = {0};
  struct Parsed_Args parsed = {0};

#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT) ret.NAME = (TYPE)DEFAULT;
  FLAGS_LIST
#undef FLAG

#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT)                                \
  else if (!strcmp(argv[i], "--" #NAME) || !strcmp(argv[i], "-" #SF)) {        \
    if (!strcmp(#TYPE, "none")) {                                              \
      bool result = true;                                                      \
      ret.NAME = *(TYPE *)&result;                                             \
      parsed.NAME = true;                                                      \
      continue;                                                                \
    }                                                                          \
                                                                               \
    if (argc < i + 1) {                                                        \
      if (REQUIRED) {                                                          \
        fprintf(stderr, "ERROR: Expected argument for %s", "--" #NAME);        \
        exit(1);                                                               \
      }                                                                        \
    } else if (!strcmp(#TYPE, "bool")) {                                       \
                                                                               \
      bool result = false;                                                     \
                                                                               \
      if (!strcmp(argv[i + 1], "true"))                                        \
        result = true;                                                         \
      else if (!strcmp(argv[i + 1], "false"))                                  \
        result = false;                                                        \
      else {                                                                   \
        fprintf(stderr, "ERROR: Invalid argument %s, expected: %s",            \
            argv[i + 1], "bool");                                              \
        exit(1);                                                               \
      }                                                                        \
      ret.NAME = *(TYPE *)&result;                                             \
      parsed.NAME = true;                                                      \
      i++;                                                                     \
                                                                               \
    } else if (!strcmp(#TYPE, "int")) {                                        \
                                                                               \
      int base = get_numeric_string_base(argv[i + 1]);                         \
      if (base <= 0) {                                                         \
        fprintf(stderr, "ERROR: Invalid argument %s, expected: %s",            \
            argv[i + 1], "int");                                               \
        exit(1);                                                               \
      }                                                                        \
      char *start = argv[i + 1];                                               \
      int r = (int)strtol(start, NULL, base);                                  \
      ret.NAME = *(TYPE *)&r;                                                  \
      parsed.NAME = true;                                                      \
      i++;                                                                     \
                                                                               \
    } else if (!strcmp(#TYPE, "float")) {                                      \
                                                                               \
      char *endptr = NULL;                                                     \
      float f = strtof(argv[i + 1], &endptr);                                  \
      if (f == 0.0f && endptr == argv[i + 1]) {                                \
        fprintf(stderr, "ERROR: Invalid argument %s, expected: %s",            \
            argv[i + 1], "float");                                             \
        exit(1);                                                               \
      };                                                                       \
      ret.NAME = *(TYPE *)&f;                                                  \
      parsed.NAME = true;                                                      \
      i++;                                                                     \
                                                                               \
    } else if (!strcmp(#TYPE, "char")) {                                       \
                                                                               \
      ret.NAME = *(TYPE *)&argv[i + 1][0];                                     \
      parsed.NAME = true;                                                      \
      i++;                                                                     \
                                                                               \
    } else if (!strcmp(#TYPE, "char *")) {                                     \
                                                                               \
      ret.NAME = *(TYPE *)&argv[i + 1];                                        \
      parsed.NAME = true;                                                      \
      i++;                                                                     \
                                                                               \
    } else {                                                                   \
      assert(false && "UNRECHEABLE: no flag type");                                          \
    }                                                                          \
  }

  
  args.exec_name = argv[0];

  for (int i = 1; i < argc; i++) {
    if (false) assert(false && "UNRECHEABLE");
    FLAGS_LIST
    else {
      if(argv[i][0] == '-' && !IS_NUM(argv[i][1])) {
        fprintf(stderr, "ERROR: unknown flag %s\n", argv[i]);
        exit(1);
      } else {
        // push arg
      }
    }
  }

#undef FLAG

#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT)                                \
  if (REQUIRED && !parsed.NAME) {                                              \
    fprintf(stderr, "ERROR: Flag --" #NAME " required but not found\n");                \
    exit(1);                                                                   \
  }
  FLAGS_LIST
#undef FLAG

  return (Input){ .args = args, .flags = ret };
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
