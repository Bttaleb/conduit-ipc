#include "io.h"
#include <_string.h>
#include <string.h>
#include <stdio.h>
#include <sys/_types/_ssize_t.h>
#include <unistd.h>
#include <sys/wait.h>

// Test 1: return n on success
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
            printf("FAIL: chunked read (ret=%zd, buf='%.*s')\n", ret, 10, buf);
        }

        close(fds[0]);
        waitpid(pid, NULL, 0);
    }
}

// Test 2: return 0 on clean EOF
static void test_clean_eof(void) {
    int fds[2];

    if (pipe(fds) < 0) {
        perror("pipe");
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        close(fds[0]);
        close(fds[1]);
        _exit(0);
    }

    if (pid > 0) {

        close(fds[1]);
        char buf[10];
        ssize_t ret = read_exact(fds[0], buf, 10);

        if (ret == 0) {
            printf("PASS: Clean EOF\n");
        } else {
            printf("FAIL: clean EOF (expected ret=0, got ret=%zd)\n", ret);
            close(fds[0]);
        }

        close(fds[0]);
        waitpid(pid, NULL, 0);
    }
}

// Test 3: return -1 on truncated EOF
static void test_truncated_eof(void) {
    int fds[2];

    if (pipe(fds) < 0) {
        perror("pipe");
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    // CHILD: writer
    if (pid == 0) {
        // close the reader
        close(fds[0]);
        write(fds[1], "ABCDE", 5); // call a write of 5 bytes
        close(fds[1]); // close the writer
        _exit(0);
    }

    // PARENT: reader
    if (pid > 0) {
        close(fds[1]);
        char buf[10];

        ssize_t ret = read_exact(fds[0], buf, 10);
        if (ret == -1) {
            printf("PASS: Truncated EOF check\n");
        } else {
            printf("FAIL: truncated EOF (expected ret=-1, got ret=%zd)\n", ret);
            close(fds[0]);
        }

        close(fds[0]);
        waitpid(pid, NULL, 0);
    }
}

int main(void) {
    test_chunked_read();
    test_clean_eof();
    test_truncated_eof();
}
