#define FLAGS_LIST \
  FLAG(port, p, int, false, 80) \
  FLAG(xar, _, char, true, '\0') \
  FLAG(url, _, char *, false, "https://u.r.l") \

#define AP_IMPLEMENTATIONS
#include "ap.h"

#include <stdio.h>

int main(int argc, char **argv) {
  Input in = parse_flags(argc, argv, NULL);

  printf("exec_name: %s\n", in.args.exec_name);
  printf("port: %d\nurl:  %s\nxar:  %c\n", in.flags.port, in.flags.url, in.flags.xar);

  return 0;
}
