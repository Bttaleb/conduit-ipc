#include "io.h"
#include <stdbool.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

static void run_read_case(const char *name, const char *chunks[], size_t nchunks, size_t ask_n, ssize_t expect_ret, const char *expect_buf) {
    int fds[2];

    char buf[64]; // create a big enough buffer for all our calls

    if (ask_n > sizeof buf) {
        printf("FAIL: %s (ask_n too large for buffer)\n", name);
        return;
    }

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
        for (size_t i = 0; i < nchunks; i++) {
            write(fds[1], chunks[i], strlen(chunks[i]));
            usleep(1000);
        }
        close(fds[1]);
        _exit(0);
    }

    if (pid > 0) {
        close(fds[1]);
        ssize_t ret = read_exact(fds[0], buf, ask_n);
        bool ok = (ret == expect_ret); // does our return match our expected return?

        if (expect_buf != NULL) {
            ok = ok && memcmp(buf, expect_buf, ask_n) == 0; // this test cares about contents, they must match as well
        }

        if (ok) {
            printf("PASS: %s\n", name);
        } else {
            printf("FAIL: %s (expected ret=%zd, got ret=%zd, ", name, expect_ret, ret);
            printf("expected buf=%.*s, got buf=%.*s)\n", (int)ask_n, expect_buf,  (int)ask_n, buf);
        }

        close(fds[0]);
        waitpid(pid, NULL, 0);
    }
}

 // Test 1: return n on success
static void test_chunked_read(void) {

    // create pipe
    int fds[2];

    // fds[0] = read end, fds[1] = write end
    if (pipe(fds) < 0) {
        perror("pipe");
        return;
    }

    // fork & branch -> parent if pid > 0 (reads) | child if pid == 0 (writes)
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

        // if we've returned 10 bytes and those bytes match the ones in our buffer then we pass
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
            printf("PASS: clean EOF\n");
        } else {
            printf("FAIL: clean EOF (expected ret=0, got ret=%zd)\n", ret);
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
        }

        close(fds[0]);
        waitpid(pid, NULL, 0);
    }
}

static void test_bad_fd(void) {
    char buf[10];
    ssize_t ret = read_exact(-1, buf, 10);

    if (ret < 0 && errno == EBADF) {
        printf("PASS: real error\n");
    } else {
        printf("FAIL: bad fd (expected ret=-1, got ret=%zd)\n", ret);
    }

}


int main(void) {
    const char *chunked[] = {"ABCD", "EFG", "HIJ"};
    run_read_case("chunked read", chunked, 3, 10, 10, "ABCDEFGHIJ");
    run_read_case("clean eof", NULL, 0, 10, 0, NULL);
    run_read_case("truncated eof check", chunked, 1, 10, -1, NULL); // sends only "ABCD" (4 of 10)
    test_bad_fd();
}
