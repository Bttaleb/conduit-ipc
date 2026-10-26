#include "io.h"
#include "protocol.h"
#include <stdio.h>
#include <inttypes.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

static void test_encode_decode(void) {
    struct result original = {42, 1000, 5555};
    uint8_t bytes[RESULT_PAYLOAD_SIZE];
    struct result back;

    encode_result(&original, bytes);
    decode_result(bytes, &back);

    if (back.job_id == 42 && back.word_count == 1000 && back.compute_ns == 5555) {
        printf("PASS: encode and decode\n");
    } else {
        printf("FAIL: encode and decode (job_id=%" PRIu32 ", word_count=%" PRIu64 ", compute_ns=%" PRIu64 ")\n", back.job_id, back.word_count, back.compute_ns);
    }
}

static void test_send_recv(void) {
    int fds[2];
    char buf[64];
    uint8_t type;

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
        ssize_t n = send_msg(fds[1], MSG_JOB, "hello", 5);
        close(fds[1]);
        if (n == 0) {
            _exit(0);
        } else {
            _exit(1);
        }

    }

    if (pid > 0) {
        close(fds[1]);
        ssize_t n = recv_msg(fds[0], &type, buf, sizeof(buf));
        if (n == 5 && type == MSG_JOB && memcmp(buf, "hello", 5) == 0) {
           printf("PASS: send/recv\n");
        } else {
            printf("FAIL: send/recv (expected n=5 | type=1 | buf=hello, got n=%zd | type=%u | buf=%.*s)\n", n, type, 5, buf);
        }
        close(fds[0]);
        waitpid(pid, NULL, 0);
    }
}

static void test_recv_clean_eof(void) {
    int fds[2];
    char buf[64];
    pid_t pid = fork();
    uint8_t type;

    if (pipe(fds) < 0) {
        perror("pipe");
    }

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
        ssize_t n = recv_msg(fds[0], &type, buf, sizeof(buf));
        if (n == 0) {
            printf("PASS: recv/clean eof\n");
        } else {
            printf("FAIL: recv/clean eof (expected n=0, got n=%zd, errno=%s)\n", n, strerror(errno));
        }
        close(fds[0]);
        waitpid(pid, NULL, 0);
    }
}

static void test_recv_too_big_for_buf(void) {
    int fds[2];
    char buf[64];
    uint8_t type;

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
        uint8_t header[HEADER_SIZE];
        uint32_t bad_len = 100;
        memcpy(header + 0, &bad_len, sizeof(bad_len));
        header[4] = MSG_JOB;
        write_all(fds[1], header, HEADER_SIZE);

        char junk[100];
        memset(junk, 'X', sizeof(junk));
        write_all(fds[1], junk, sizeof(junk));
        close(fds[1]);
        _exit(0);
    }

    if (pid > 0) {
        close(fds[1]);
        ssize_t n = recv_msg(fds[0], &type, buf, sizeof(buf));
        if (n == -1 && errno == EMSGSIZE) {
            printf("PASS: recv too big for buf\n");
        } else {
            printf("FAIL: recv too big for buf (expected n=-1, errno=EMSGSIZE | got n=%zd, errno=%s)\n", n, strerror(errno));
        }
        close(fds[0]);
        waitpid(pid, NULL, 0);
    }
}

int main(void) {
    signal(SIGPIPE, SIG_IGN);
    test_encode_decode();
    test_send_recv();
    test_recv_clean_eof();
    test_recv_too_big_for_buf();
}
