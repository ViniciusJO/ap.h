# ap.h - argument parser

Simple to use argument parser. Its able to parse `flags` and positional parâmeters (`pos_args`). Its a single header [stb](https://github.com/nothings/stb?tab=readme-ov-file#how-do-i-use-these-libraries) style library where the definition of the args is done by setting [X macros](https://en.wikipedia.org/wiki/X_macro) that need to come before the inclusion of the header.

## API
### Definitions

#### `flags`:

First define `FLAGS_LIST` and bellow it, escaping newlines with `\` each line will define a flag. A template:

```c
#define FLAGS_LIST \
  FLAG(NAME, SHORT_FORM, TYPE, REQUIRED, DEFAULT_VALUE, DESCRIPTION) \
```

`Flags` can be passed in the CLI as `--NAME` or `-SHORT_FORM` in any possition of the command line. `TYPE` is the desired parsed type for the flag, `REQUIRED` is a boolean that tells the lib wether to panic on absense of the flag or not. `DEFAULT_VALUE` is the default value if the flag is not passed, so it must be of type `TYPE`. In the case that `REQUIRED` is set, `DEFAULT_VALUE` is ignored. I this case, put any value of the type `TYPE` as default.

Te possible types are:

1. `none`:  do not receive args. The presence of the flag sets the field as `true` and the absence sets it to `false`;
2. `bool`:  receive a boolean in the literal string format `true` or `false` and parses it.
3. `int`:   expects an integer
4. `float`: expects a floating point number
5. `char`:  expects a single character
6. `char*` or `char *`: expects a string

The `DESCRIPTION` field is used to construct the usage text.

> **IMPORTANT: do note surround with quote any of the parameters besides the char and strings default values!**

#### `pos_args`

Similar to the flags, define `POS_ARGS_LIST` and bellow it each line define a positional argument. Follows the template for the definition:

```c
#define POS_ARGS_LIST \
  POS_ARG(NAME, REQUIRED, DEFAULT_VALUE, DESCRIPTION)
```

> For now, all the `pos_args` are stored as `char*` and are not parsed at all.

The fields names have the same meaning as in the `FLAG` definition. Because of the positional nature of the args, whe a args is marked as not `REQUIRED`, all the following args are inherently not `REQUIRED`, so this field definition are ignored. It is important to reference those args in the order in they will appear in the command line. Since the `flags` are not acounted in the sequence, `flags` and `pos_args` can be interleaved.

The `DESCRIPTION` field is used to construct the usage text.

[libc](https://www.gnu.org/software/libc/) aways sets the first argument in `argv` as the binary name, so there will aways be the `pos_args.exec_name` field.

### Include

After the configurations of the `flags` and the `pos_args`, define the `AP_IMPLEMENTATIONS` and include the `ap.h` header.

```c
#define AP_IMPLEMENTATIONS
#include "ap.h"
```

By default the functions defined in `ap.h` are `extern` functions, which means that setting `AP_IMPLEMENTATIONS` in multiple translation units will cause linking conflicts. Its a standard in [stb](https://github.com/nothings/stb?tab=readme-ov-file#how-do-i-use-these-libraries) style libraries to set the implementation in one single translation unit and refer only to the header in the rest of the files. However there are the `AP_ATTRIBUTES` macro that allows the isolation of the implementations per translation file via:

```c
#define AP_ATTRIBUTES static
```

With this same macro is possible to specify other compiler attributes ([gcc](https://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html), [clang](https://clang.llvm.org/docs/AttributeReference.html), etc.) to the functions.

### Parsing and accessing args

The lib defines the type `Args` that have as fields the flags defined before and in the field `pos_args` all the positional args defined. Use the function `ap_parse_args` to get the filled `Args` instance. So in the main function do:

```c
Args args = ap_parse_args(argc, argv);
```

For this point on, the flags can be accessed as `args.NAME`, where `NAME` is the name of the flag. In the same manner, to access a positional argument with name `NAME`, do `args.pos_args.NAME`.

### Usage

The library exposes the `void ap_usage(FILE *fd);` function which generates and prints to `fd` the usage section containing information about the allowed arguments and its properties. By default every error on parsing the args will show the usage on `stderr` alongside the error message.

## Example

On a phone book application:

```c
// FLAG(NAME, SHORT_FORM, TYPE, REQUIRED, DEFAULT_VALUE, DESCRIPTION)
#define FLAGS_LIST                                 \
  FLAG(contact_list, cl, char *, false, "default", "Contact list name")  \
  FLAG(local, l, char*, true, "", "Location")                  \
  FLAG(DDD, , int, false, 1, "Direct distance dialing code")

// POS_ARG(NAME, REQUIRED, DEFAULT_VALUE, DESCRIPTION)
#define POS_ARGS_LIST                \
  POS_ARG(name, true, "", "Contact name")            \
  POS_ARG(phone, false, "-", "Phone number")         \
  POS_ARG(type, false, "commercial", "Type of contact") 

#define AP_IMPLEMENTATIONS
#include "ap.h"

#include <stdio.h>

int main(int argc, char **argv) {
  Args args = ap_parse_args(argc, argv);

  printf("exec_name: %s\n\n", args.pos_args.exec_name);

  printf("[POSITIONAL ARGS]\n");
  printf("name: %s\n", args.pos_args.name);
  printf("phone: %s\n", args.pos_args.phone);
  printf("type: %s\n\n", args.pos_args.type);

  printf("[FLAGS]\n");
  printf("contact_list: %s\n", args.contact_list);
  printf("local:  %s\n", args.local);
  printf("DDD:  %d\n", args.DDD);

  return 0;
}
```

calling: 

```sh
./test -cl "list01" my_name --local "my local" 99988-7766
```


outputs: 

```
exec_name: ./test

[POSITIONAL ARGS]
name: my_name
phone: 99988-7766
type: commercial

[FLAGS]
contact_list: list01
local:  my local
DDD:  1
```

Calling it without any arguments:

```sh
./test
```

shows the error and the usage section:

```
ERROR: Flag --local required but not found

Usage: ap.h --local <local> <name> [phone] [type]

Arguments:
        <name>          Contact name (default: "")
        [phone]         Phone number (default: "-")
        [type]          Type of contact (default: "commercial")

Flags:
        -cl, --contact_list             Contact list name (default: "default")
        -l , --local                    Location (default: "")
             --DDD                      Direct distance dialing code (default: 1)
```

# TODO

- [x] usage functions
- [ ] type and parse positional args?
- [ ] static and dynamic array of args
- [ ] subcommands
- [ ] shell completions (bash, zsh, fish, ...)

