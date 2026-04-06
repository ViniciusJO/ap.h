// FLAG(NAME, SHORT_FORM, TYPE, REQUIRED, DEFAULT_VALUE, DESCRIPTION, __VARIADIC_VARIANTS__)
#define FLAGS_LIST(FLAG)                                                                      \
  FLAG(local, l, char*, true, "", "Location")                                                 \
  FLAG(contact_list, cl, char *, false, "default", "Contact list name")                       \
  FLAG(DDD, , int, false, 1, "Direct distance dialing code", 318, 350)                        \
  FLAG(x, , int, false, 0, "Direct distance dialing code")

// POS_ARG(NAME, REQUIRED, DEFAULT_VALUE, DESCRIPTION, __VARIADIC_VARIANTS__)
#define POS_ARGS_LIST(POS_ARG)                                                                \
  POS_ARG(name, true, "", "Contact name")                                                     \
  POS_ARG(phone, false, "-", "Phone number")                                                  \
  POS_ARG(type, false, "commercial", "Type of contact", commercial, personal, organizational) \
  POS_ARG(subcommand, false, "help", "Subcmd", A,B,C,D) 

#define AP_ALLOW_EXTRAS
#define AP_IMPLEMENTATIONS
#include "ap.h"

#include <stdio.h>

#define T(DEFAULT) #DEFAULT


Args tests(int argc, char **argv) {
  Args args = ap_parse_args(argc, argv);

  printf("exec_name: %s\n\n", args.pos_args.exec_name);

  printf("[POSITIONAL ARGS]\n");
  printf("name: %s\n", args.pos_args.name);
  printf("phone: %s\n", args.pos_args.phone);
  printf("type: %s\n", args.pos_args.type);
  printf("subcommand: %s\n", args.pos_args.subcommand);
#ifdef AP_ALLOW_EXTRAS
  printf("extras: ");
  al_print(&args.pos_args.extras, "\n");
#endif//AP_ALLOW_EXTRAS
  printf("\n");

  printf("[FLAGS]\n");
  printf("contact_list: %s\n", args.contact_list);
  printf("local:  %s\n", args.local);
  printf("DDD:  %d\n", args.DDD);
  return args;
}

int main(int argc, char **argv) {
  tests(argc, argv);
  // ap_completions(stdout);
  return 0;
}
