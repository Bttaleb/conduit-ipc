#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include "io.h"

ssize_t read_exact(int fd, void *buf, size_t n) {

    size_t got = 0; // runnning total of bytes across all calls
    while (got < n) { // loop until we've obtained all bytes

        ssize_t r = read(fd, (char *)buf + got, n - got); // assign how many bytes returned from our call to `r`

        if (r < 0) { // Handle fatal errors, retry
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        if (r == 0) { // EOF
            if (got == 0) {
                return 0;
            }
            return -1; // if got != 0 we return an error
        }
        got += r;
    }
    return n;
}

ssize_t write_all(int fd, const void *buf, size_t n) {

    size_t sent = 0;
    while (sent < n) {
        ssize_t w = write(fd, (const char *)buf + sent, n - sent);
        if (w < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        if (w == 0) {
            errno = EIO;
            return -1;
        }
        sent += w;
    }
    return n;
}
