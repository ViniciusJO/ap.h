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

#ifndef POS_ARGS_LIST
#define POS_ARGS_LIST POS_ARG(default_arg, false, NULL)
#endif // FLAGS_LIST

typedef bool none;

// typedef struct {
// #ifdef FLAGS_LIST
// #define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT) TYPE NAME;
//   FLAGS_LIST
// #undef FLAG
// #endif//FLAGS_LIST
// } Flags;

// struct ___Short_Flags {
// #define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT) TYPE SF;
//   FLAGS_LIST
// #undef FLAG
// };

typedef struct {
  char *exec_name;
#ifdef POS_ARGS_LIST
#define POS_ARG(NAME, REQUIRED, DEFAULT) char *NAME;
  POS_ARGS_LIST
#undef POS_ARG
#endif//POS_ARGS_LIST
} PosArgs;

typedef struct {
#ifdef FLAGS_LIST
#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT) TYPE NAME;
  FLAGS_LIST
#undef FLAG
#endif//FLAGS_LIST
  PosArgs pos_args;
} Args;

// static const char ArgsSequence[] = {
// #define POS_ARG(NAME, TYPE, REQUIRED, DEFAULT) #NAME,
//   POS_ARGS_LIST
// #undef POS_ARG
// };

enum ArgsSequence {
#define POS_ARG(NAME, REQUIRED, DEFAULT) NAME,
  POS_ARGS_LIST
#undef POS_ARG
  ARGS_COUNT
};

AP_ATTRIBUTES Args parse_args(int argc, char **argv);

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
      fprintf(stderr, "ERROR: Expected argument for --%s", name);
      exit(1);
    }
  } else if (!strcmp(type, "bool")) {

    if (!strcmp(argv[i + 1], "true")) {
      ret.value.b = true;
      ret.type = 'b';
    } else if (!strcmp(argv[i + 1], "false")) {
      ret.value.b = false;
      ret.type = 'b';
    } else {
      fprintf(stderr, "ERROR: Invalid argument %s, expected: %s", argv[i + 1], "bool");
      exit(1);
    }
  } else if (!strcmp(type, "int")) {

    int base = get_numeric_string_base(argv[i + 1]);
    if (base <= 0) {
      fprintf(stderr, "ERROR: Invalid argument %s, expected: %s", argv[i + 1], "int");
      exit(1);
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
      fprintf(stderr, "ERROR: Invalid argument %s, expected: %s", argv[i + 1], "float");
      exit(1);
    };
    return ret;

  } else if (!strcmp(type, "char")) {

    ret.value.c = argv[i + 1][0];
    ret.type = 'c';
    return ret;

  } else if (!strcmp(type, "char*")) {

    ret.value.s = argv[i + 1];
    ret.type = 's';
    return ret;

  } else { assert(false && "UNRECHEABLE: no flag type"); }

  return ret;
}

AP_ATTRIBUTES Args parse_args(int argc, char **argv) {
  int c = 0;
  // PosArgs args = {0};
  Args ret = {0};
  struct Parsed_Args parsed = {0};

#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT) ret.NAME = (TYPE)DEFAULT;
  FLAGS_LIST
#undef FLAG

#define FLAG(NAME, SF, TYPE, REQUIRED, DEFAULT)                                         \
  else if (!strcmp(argv[i], "--" #NAME) || !strcmp(argv[i], "-" #SF)) {                 \
    ArgValue av = __parse_arg((char*)#NAME, (char*)#TYPE, argc, argv, i, REQUIRED);     \
    switch (av.type) {                                                                  \
      case 'b': ret.NAME = *(TYPE*)&av.value.b; break;                                  \
      case 'i': ret.NAME = *(TYPE*)&av.value.i; break;                                  \
      case 'f': ret.NAME = *(TYPE*)&av.value.f; break;                                  \
      case 'c': ret.NAME = *(TYPE*)&av.value.c; break;                                  \
      case 's': ret.NAME = *(TYPE*)&av.value.s; break;                                  \
    };                                                                                  \
    if(strcmp(#TYPE, "none")) ++i;                                                      \
    parsed.NAME = true;                                                                 \
  }

  ret.pos_args.exec_name = argv[0];

  for (int i = 1; i < argc; i++) {
    if (false) assert(false && "UNRECHEABLE");
    FLAGS_LIST
    else {
      if(argv[i][0] == '-' && !IS_NUM(argv[i][1])) {
        fprintf(stderr, "ERROR: unknown flag %s\n", argv[i]);
        exit(1);
      } else {
        if(c < ARGS_COUNT) {
          switch(c) {
#define POS_ARG(NAME, REQUIRED, DEFAULT) case NAME: { ret.pos_args.NAME = argv[i]; } break;
            POS_ARGS_LIST
#undef POS_ARG
          }
          c++;
        } else {
          fprintf(stderr, "ERROR: unknown flag %s\n", argv[i]);
          exit(1);
        }
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

  c = 0;
#define POS_ARG(NAME, REQUIRED, DEFAULT) if (REQUIRED) {                                \
  if(ret.pos_args.NAME == NULL) {                                                               \
    fprintf(stderr, "ERROR: Positional arg " #NAME "(%d) required but not found\n", c); \
    exit(1);                                                                            \
  }                                                                                     \
  c++;                                                                                  \
} else goto unrequired;
  POS_ARGS_LIST
#undef POS_ARG

unrequired:
  for (int i = c; i < ARGS_COUNT; ++i) {
    // printf("%d: ---------------\n", i);
    switch(i) {
#define POS_ARG(NAME, REQUIRED, DEFAULT) case NAME: { if(ret.pos_args.NAME == NULL && DEFAULT) ret.pos_args.NAME = DEFAULT; } break;
      POS_ARGS_LIST
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
