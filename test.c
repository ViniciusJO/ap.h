#define FLAGS_LIST                             \
  FLAG(port, p, int, false, 80)                \
  FLAG(xar, _, char, true, 0)                  \
  FLAG(url, u, char *, false, "https://u.r.l") \

#define ARGS_LIST \
  ARG(phone, true, "") \
  ARG(car, true, "") \
  ARG(r, false, "") \
  ARG(test, true, "") \

#define AP_IMPLEMENTATIONS
#include "ap.h"

#include <stdio.h>

int main(int argc, char **argv) {
  Input in = parse_args(argc, argv, NULL);

  printf("exec_name: %s\nphone: %s\ncar: %s\nr: %s\ntest: %s\n", in.args.exec_name, in.args.phone, in.args.car, in.args.r, in.args.test);
  printf("port: %d\nurl:  %s\nxar:  %c\n", in.flags.port, in.flags.url, in.flags.xar);

  return 0;
}
