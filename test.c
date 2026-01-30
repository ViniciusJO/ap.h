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
