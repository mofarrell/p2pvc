#include <stdio.h>

#ifdef VCONLY
int main(void) {
  printf("hello vc\n");
  return 0;
}
#endif
