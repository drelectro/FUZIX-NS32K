#include <termios.h>
#include <unistd.h>

int tcsendbreak(int fd, int duration)
{
  (void)fd;
  (void)duration;
  return 0;
}
