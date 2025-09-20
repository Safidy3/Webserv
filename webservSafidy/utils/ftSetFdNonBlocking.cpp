

#include <fcntl.h>		// for non-blocking fd

void set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		flags = 0;
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}