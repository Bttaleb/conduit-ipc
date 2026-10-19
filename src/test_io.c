#include "io.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

static void test_chunked_read(void) {

    // create pipe
    int fds[2];

    // fds[0] = read end, fds[1] = write end
    if (pipe(fds) < 0) {
        perror("pipe");
        return;
    }

    // fork & branch -> parent if pid > 0 | child if pid == 0
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    // CHILD: scripted writer
    if (pid == 0) {

        // we don't need the read end, close it
        close(fds[0]);

        // write the 4 byte string-"ABCD" into the pipe
        write(fds[1], "ABCD", 4);
        //simulate work
        usleep(1000);

        write(fds[1], "EFG", 3);
        usleep(1000);

        write(fds[1], "HIJ", 3);
        usleep(1000);

        close(fds[1]);
        _exit(0);
    }

    //PARENT: scripted reader
    if (pid > 0) {

        close(fds[1]);

        char buf[10];
        ssize_t ret = read_exact(fds[0], buf, 10);

        if (ret == 10 && memcmp(buf, "ABCDEFGHIJ", 10) == 0) {
            printf("PASS: chunked read\n");
        } else {
            printf("FAIL: chunked read\n");
        }

        close(fds[0]);
        waitpid(pid, NULL, 0);
    }
}

int main(void) {
    test_chunked_read();
}
