#include "protocol.h"
#include <stdio.h>
#include <inttypes.h>

int main(void) {
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
