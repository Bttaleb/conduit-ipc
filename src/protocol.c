#include "protocol.h"
#include <string.h>
#include <errno.h>
#include "io.h"

void encode_result(const struct result *r, uint8_t *out) {
    memcpy(out + 0, &r->job_id, sizeof(r->job_id)); // 4 bytes at offset 0
    memcpy(out + 4, &r->word_count, sizeof(r->word_count)); // 8 bytes at offset 4
    memcpy(out + 12, &r->compute_ns, sizeof(r->compute_ns)); // 8 bytes at offset 12
}

void decode_result(const uint8_t *in, struct result *r) {
    memcpy(&r->job_id, in + 0, sizeof(r->job_id));
    memcpy(&r->word_count, in + 4, sizeof(r->word_count));
    memcpy(&r->compute_ns, in + 12, sizeof(r->compute_ns));
}

ssize_t send_msg(int fd, uint8_t type, const void *payload, uint32_t len) {

    uint8_t header[HEADER_SIZE];

    if (len == 0) {
        errno = EINVAL;
        return -1;
    }

    if (len > MAX_PAYLOAD) {
        errno = EMSGSIZE;
        return -1;
    }

    memcpy(header + 0, &len, sizeof(len));
    header[4] = type;
    ssize_t h = write_all(fd, header, HEADER_SIZE);

    if (h < 0) {
        return -1;
    }

    ssize_t p = write_all(fd, payload, len);

    if (p < 0) {
        return -1;
    }

    return 0;
}
