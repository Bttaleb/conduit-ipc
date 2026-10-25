#include "protocol.h"
#include <string.h>

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
