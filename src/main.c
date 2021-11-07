#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "sha256.h"

#define DECIMAL(x) ((('0' <= (x) && (x) <= '9' ? (x) : (x) ^ 32) + 30) % 39)

SHA256_CTX ctx;
time_t t;

void readopt(FILE *fh, const char *prefix, unsigned char *dest, int size) {
    #define READLINE_BUFFER_SIZE 128    

    // Allocate a buffer, 
    char buf[READLINE_BUFFER_SIZE];
    memset(buf, ' ', READLINE_BUFFER_SIZE);
    
    // Read into the buffer.
    (void)fgets(buf, READLINE_BUFFER_SIZE, fh);
    for (int i = 0; i < READLINE_BUFFER_SIZE; i++)
        if (buf[i] == '\x00' || buf[i] == '\n')
            buf[i] = ' ';

    // Check to make sure that the prefix matches the expected value.
    if (memcmp(prefix, buf, strlen(prefix)) != 0) {
        printf("WARN: Your configuration file is in an unexpected format.");
    }

    memcpy(dest, buf + strlen(prefix), size);

    #undef READLINE_BUFFER_SIZE 
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s <vminer.conf>\n", argv[0]);
        return 1;
    }

    //
    // Program arguments.
    //

    unsigned char *MAGIC_BYTES     = calloc(16, sizeof(unsigned char));
    unsigned char *USERNAME        = calloc(16, sizeof(unsigned char));
    unsigned char *PREV_BLOCK_HASH = calloc(64, sizeof(unsigned char));
    unsigned char *MESSAGE         = calloc(64, sizeof(unsigned char));
    unsigned char *THRESHOLD_HSTR  = calloc(64, sizeof(unsigned char));

    //
    // Read vminer.conf.
    //

    FILE *fh = fopen(argv[1], "r");

    readopt(fh, "MAGIC_BYTES  =", MAGIC_BYTES    , 16);
    readopt(fh, "USERNAME     =", USERNAME       , 16);
    readopt(fh, "PREVIOUS_HASH=", PREV_BLOCK_HASH, 64);
    readopt(fh, "MESSAGE      =", MESSAGE        , 64);
    readopt(fh, "THRESHOLD    =", THRESHOLD_HSTR , 64);

    fclose(fh);

    printf("====[ VMiner V1.0.0 ]============================================================\n");
    printf("    MAGIC_BYTES: %s\n", MAGIC_BYTES    );
    printf("       USERNAME: %s\n", USERNAME       );
    printf("PREV_BLOCK_HASH: %s\n", PREV_BLOCK_HASH);
    printf("        MESSAGE: %s\n", MESSAGE        );
    printf("      THRESHOLD: %s\n", THRESHOLD_HSTR );
    printf("=================================================================================\n");
    printf("\n");

    //
    // Convert the "THRESHOLD_HSTR" to bytes.
    //
    unsigned char *THRESHOLD = calloc(32, sizeof(unsigned char));
    for (int i = 0; i < 32; i++)
        THRESHOLD[i] = (DECIMAL(THRESHOLD_HSTR[2*i]) << 4) + DECIMAL(THRESHOLD_HSTR[2*i + 1]);

    //
    // Compute the block hash.
    //

    unsigned char *pt = calloc(144, sizeof(unsigned char));
    memcpy(pt + 0 , MAGIC_BYTES    , 16);
    memcpy(pt + 16, PREV_BLOCK_HASH, 64);
    memcpy(pt + 80, MESSAGE        , 64);

    unsigned char *BLOCK_HASH = calloc(SHA256_BLOCK_SIZE, sizeof(unsigned char));

    sha256_init(&ctx);
    sha256_update(&ctx, pt, 144);
    sha256_final(&ctx, BLOCK_HASH);

    free(pt);

    printf("BLOCK_HASH: ");
    for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
        printf("%02x", BLOCK_HASH[i]);
    printf("\nCheck to make sure that this matches the 'BLOCK HASH' listed");
    printf(" on the website. If it doesn't, then something has gone wrong!\n\n");

    //
    // Now, start mining.
    //

    srand((unsigned)time(&t));

    pt = calloc(64, sizeof(unsigned char));
    memcpy(pt + 0 , BLOCK_HASH, SHA256_BLOCK_SIZE);
    memcpy(pt + 32, USERNAME , 16);
    *(pt + 60) = 'Q';
    *(pt + 61) = 'U';
    *(pt + 62) = 'X';
    *(pt + 63) = '5';

    // Fill a random nonce.
    for (unsigned char *it = pt + 48; it < pt + 60; ++it) {
        *it = (unsigned char)('a' + (rand() % 26));
    }

    unsigned char *SIGNATURE = calloc(SHA256_BLOCK_SIZE, sizeof(unsigned char));

    clock_t t1 = clock(), t2;
    int i = 0;
    while (1) {
        // Modify the current nonce, slightly.
        *(pt + 48 + (i % 12)) = (unsigned char)('a' + (rand() % 26));

        sha256_init(&ctx);
        sha256_update(&ctx, pt, 64);
        sha256_final(&ctx, SIGNATURE);

        // Is it small enough?
        for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
            if (SIGNATURE[i] == THRESHOLD[i]) continue;
            if (SIGNATURE[i] <  THRESHOLD[i]) break;
            if (SIGNATURE[i] >  THRESHOLD[i]) goto skip;
        }

        printf("\nINFO: Found a valid nonce,\n\n    ");
        for (unsigned char *it = pt + 48; it < pt + 64; ++it)
            printf("%c", *it);
        printf("\n\nThis produces a signature hash of ");
        for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
            printf("%02x", SIGNATURE[i]);
        printf(".\n");

        return 0;

        skip:
        // Count number of attempts, so far.
        i++;
        if (i % 10000000 == 0) {
            t2 = clock();
            double runtime = (double)(t2 - t1) / CLOCKS_PER_SEC;
            printf("INFO: Currently averaging %f hashes per second...\n", 10000000.0 / runtime);


            t1 = clock();
        }

        continue;
    }
}
