# ap.h - argument parser

Single header [stb](https://github.com/nothings/stb?tab=readme-ov-file#how-do-i-use-these-libraries) style command-line argument parser for C. Flags and positional arguments are defined via [X macros](https://en.wikipedia.org/wiki/X_macro) before including the header. The library automatically generates the `Args` struct, parsing logic, usage message, variant validation, and shell completion scripts.

## Quick Start

```c
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

#define AP_IMPLEMENTATIONS
#include "ap.h"

#include <stdio.h>

int main(int argc, char **argv) {
  Args args = ap_parse_args(argc, argv);

  printf("Verbose: %d\n", args.verbose);
  printf("Config: %s\n", args.config);
  printf("Threads: %d\n", args.threads);
  printf("Mode: %s\n", args.mode);
  printf("Input: %s\n", args.pos_args.input);
  printf("Output: %s\n", args.pos_args.output);
}
```

## API

### Flags

Define `FLAGS_LIST` as a parameterized macro that receives the `FLAG` macro name:

```c
// FLAG(NAME, SHORT_FORM, TYPE, REQUIRED, DEFAULT, DESCRIPTION, ...VARIANTS)
#define FLAGS_LIST(FLAG)                                               \
  FLAG(contact_list, cl, char*, false, "default", "Contact list name") \
  FLAG(local, l, char*, true, "", "Location")                          \
  FLAG(DDD, , int, false, 11, "DDD code", 31, 11, 95)
```

Flags can be passed in the CLI as `--NAME` or `-SHORT_FORM` in any position. Values can be assigned with a space (`--flag value`) or with `=` syntax (`--flag=value`, `-f=value`).

Leave the short form empty to disable it: `FLAG(long_only, , int, ...)`.

#### Types

| Type | Description | Consumes next arg |
|------|-------------|-------------------|
| `none` | Boolean flag, presence sets `true` | No |
| `bool` | Expects literal `true` or `false` | Yes |
| `int` | Integer (decimal, hex `0x`, octal `0`, binary `0b`) | Yes |
| `float` | Floating point number | Yes |
| `char` | Single character | Yes |
| `char*` / `char *` | String | Yes |

> **IMPORTANT: do not surround with quotes any of the parameters besides descriptions and the char and string default values!**

### Positional Arguments

Define `POS_ARGS_LIST` as a parameterized macro:

```c
// POS_ARG(NAME, REQUIRED, DEFAULT, DESCRIPTION, ...VARIANTS)
#define POS_ARGS_LIST(POS_ARG)                                                                \
  POS_ARG(name, true, "", "Contact name")                                                     \
  POS_ARG(type, false, "commercial", "Type of contact", commercial, personal, organizational)
```

Positional arguments are always stored as `char*`. They are matched in the order they are defined. When an argument is marked as not required, all following arguments are inherently optional. Flags and positional arguments can be interleaved.

[libc](https://www.gnu.org/software/libc/) always sets the first argument in `argv` as the binary name, so the `pos_args.exec_name` field is always available.

### Variants

Both flags and positional arguments support an optional variadic list of allowed values. These are validated at parse time:

```c
FLAG(mode, m, char*, false, "fast", "Run mode", fast, safe, debug)
POS_ARG(action, true, "", "Action", start, stop, restart)
FLAG(DDD, , int, false, 11, "DDD code", 31, 11, 95)
```

The usage message shows the allowed variants, and on invalid input:

```
invalid_variant: unknown not in { start, stop, restart } allowed for action
```

### Include

After defining `FLAGS_LIST` and `POS_ARGS_LIST`, define `AP_IMPLEMENTATIONS` and include the header:

```c
#define AP_IMPLEMENTATIONS
#include "ap.h"
```

By default the functions defined in `ap.h` are `extern`, which means that setting `AP_IMPLEMENTATIONS` in multiple translation units will cause linking conflicts. Use the `AP_ATTRIBUTES` macro to isolate implementations per translation unit:

```c
#define AP_ATTRIBUTES static
```

This same macro can be used to specify compiler attributes ([gcc](https://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html), [clang](https://clang.llvm.org/docs/AttributeReference.html)).

### Parsing and Accessing Args

The library defines the `Args` struct with all flags as typed fields and positional arguments inside `pos_args`:

```c
Args args = ap_parse_args(argc, argv);

// Flags accessed directly
args.verbose     // bool
args.config      // char*
args.threads     // int

// Positional args accessed via pos_args
args.pos_args.exec_name   // always argv[0]
args.pos_args.input       // char*
args.pos_args.output      // char*
```

### Usage

The `void ap_usage(FILE *fd)` function prints the usage message with all defined arguments, their types, defaults, and variants. Every parse error automatically shows usage on `AP_ERR_FD` alongside the error message.

### Built-in Subcommands

`ap_parse_args` automatically handles two subcommands before parsing:

| Subcommand | Description |
|---|---|
| `help` | Prints usage message to `AP_OUT_FD` and exits |
| `completions <shell>` | Generates shell completion script and exits |

The `--help` / `-h` flags are also always available.

### Shell Completions

Completion script generation is built in for zsh (bash and fish are planned). The generated scripts include flag names, short forms, descriptions, and variant values.

```sh
# Zsh
eval "$(prog completions zsh)"

# Add to .zshrc for permanent installation
eval "$(prog completions zsh)"
```

### Command-line Syntax

```sh
# Positional arguments
$ prog input.txt

# Flags with space-separated values
$ prog --verbose --threads 8 input.txt

# Short form flags
$ prog -v -t 8 input.txt

# Value assignment with = syntax
$ prog --threads=8 -t=8 input.txt

# Mixed short and long form
$ prog -v --threads 8 -c custom.ini input.txt

# Option terminator (everything after -- is positional)
$ prog input.txt -- --not-a-flag.txt
```

## Optional Features

```c
#define AP_ALLOW_EXTRAS    // Enable collection of extra positional arguments
#define AP_NO_CLEAN        // Keep IS_NUM, IS_HEX, IS_BIN, IS_OCT macros
#define AP_ATTRIBUTES      // Add custom attributes to generated functions
```

### Extra Arguments

With `AP_ALLOW_EXTRAS`, positional arguments beyond the defined ones are collected:

```c
#define AP_ALLOW_EXTRAS
#define AP_IMPLEMENTATIONS
#include "ap.h"

// ...
for (size_t i = 0; i < args.pos_args.extras.count; i++) {
  printf("Extra %zu: %s\n", i, args.pos_args.extras.items[i]);
}
```

Without `AP_ALLOW_EXTRAS`, unknown positional arguments cause a parse error.

## Behavior Overrides

These macros can be defined **before** including `ap.h` to change how the parser handles errors, output, and exit:

| Macro | Default | Description |
|---|---|---|
| `AP_EXIT(code)` | `exit(code)` | Called on parse errors and help/completions |
| `AP_FPRINTF` | `fprintf` | Used for error and variant messages |
| `AP_ERR_FD` | `stderr` | File descriptor for error output |
| `AP_OUT_FD` | `stdout` | File descriptor for help and completions |

`al.h` (required dependency) also provides its own overrides:

| Macro | Default | Description |
|---|---|---|
| `AL_ASSERT` | `assert` | Assertion for memory allocation failures |
| `AL_REALLOC` | `realloc` | Memory allocator for dynamic arrays |
| `AL_INIT_CAP` | `256` | Initial capacity for dynamic arrays |
| `AL_FPRINTF` | `fprintf` | Used for al_print and array error messages |
| `AL_EXIT(code)` | `exit(code)` | Called on array bounds errors |

### Recoverable Parsing

Replace `exit()` with `longjmp()` to recover from parse errors:

```c
#include <setjmp.h>
static jmp_buf ap_err_jmp;

#define AP_EXIT(code) longjmp(ap_err_jmp, (code))
#define AP_IMPLEMENTATIONS
#include "ap.h"

int main(int argc, char **argv) {
  int err = setjmp(ap_err_jmp);
  if (err) {
    printf("Parse failed with code %d\n", err);
    return 1;
  }
  Args args = ap_parse_args(argc, argv);
  // ...
}
```

### Silence Output

```c
#define AP_FPRINTF(fd, ...) ((void)0)
```

### Redirect Output

```c
#define AP_ERR_FD my_log_file
#define AP_OUT_FD my_log_file
```

## Example

Phone book application:

```c
#define FLAGS_LIST(FLAG)                                                                      \
  FLAG(local, l, char*, true, "", "Location")                                                 \
  FLAG(contact_list, cl, char *, false, "default", "Contact list name")                       \
  FLAG(DDD, , int, false, 11, "Direct distance dialing code", 31, 11, 95)

#define POS_ARGS_LIST(POS_ARG)                                                                \
  POS_ARG(name, true, "", "Contact name")                                                     \
  POS_ARG(phone, false, "-", "Phone number")                                                  \
  POS_ARG(type, false, "commercial", "Type of contact", commercial, personal, organizational)

#define AP_ALLOW_EXTRAS
#define AP_IMPLEMENTATIONS
#include "ap.h"

#include <stdio.h>

int main(int argc, char **argv) {
  Args args = ap_parse_args(argc, argv);

  printf("exec_name: %s\n\n", args.pos_args.exec_name);

  printf("[POSITIONAL ARGS]\n");
  printf("name: %s\n", args.pos_args.name);
  printf("phone: %s\n", args.pos_args.phone);
  printf("type: %s\n", args.pos_args.type);

  printf("[FLAGS]\n");
  printf("contact_list: %s\n", args.contact_list);
  printf("local:  %s\n", args.local);
  printf("DDD:  %d\n", args.DDD);

  return 0;
}
```

Calling:

```sh
./test -cl "list01" my_name --local "my local" 99988-7766
```

Output:

```
exec_name: ./test

[POSITIONAL ARGS]
name: my_name
phone: 99988-7766
type: commercial

[FLAGS]
contact_list: list01
local:  my local
DDD:  11
```

Calling without arguments shows the error and usage:

```
ERROR: Flag --local required but not found

Usage: ./test --local <local> <name> [phone] [type] [subcommand]

Arguments:
  [help]         Shows this help message
  <name>         Contact name  (default: "")
  [phone]        Phone number  (default: "-")
  [type]         Type of contact  (default: "commercial")  (variants: commercial, personal, organizational)

Flags:
  -h , --help                    Shows this help message
  -l , --local         <string>  Location (default: "")
  -cl, --contact_list  <string>  Contact list name (default: "default")
       --DDD           <int>     Direct distance dialing code (default: 11) (variants: 31, 11, 95)
```

## Dependencies

- `al.h` - array list implementation (included in this repository)

## TODO

- [ ] Fix completions for bash
- [ ] Fix completions for fish
- [ ] Recognize flag concatenation, so that `-f -l -a` could be `-fla` (behind a macro, disabled by default)
