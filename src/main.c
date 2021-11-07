#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "sha256.h"

#define HEX2DEC(x) ((((x) ^ 16) + 7) % 39)

// Compile-time constants.
char *CHARSET = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
int CHARSET_SIZE = 62;

char *MAGIC_BYTES = "ISVENONACOINBEST";
char *AUTHOR_TOKEN = "QUX5";

struct Args
{
    char *username;
    char *previous_block_hash;
    char *message;
    uint8_t *threshold;
};

int error(bool condition, const char *message, unsigned int ln)
{
    if (condition)
    {
        if (ln >= 0)
            fprintf(stderr, "ERROR: %s on line %d.\n", message, ln);
        else
            fprintf(stderr, "ERROR: %s.\n", message);
        return 1;
    }
    else return 0;
}

int main(int argc, char *argv[])
{
    assert(16 == strlen(MAGIC_BYTES ));
    assert( 4 == strlen(AUTHOR_TOKEN));

    if (argc != 3)
    {
        printf("usage: %s <username> <vminer.conf>\n", argv[0]);
        return 1;
    }

    //
    // Parse arguments from .conf file.
    //

    struct Args args;
    args.username               = NULL;
    args.previous_block_hash    = NULL;
    args.message                = NULL;
    args.threshold              = NULL;

    args.username = calloc(17, sizeof(char));
    memset(args.username, ' ', 16);
    memcpy(args.username, argv[1], strlen(argv[1]));

    FILE *fh = fopen(argv[2], "r");
    char buf1[256], buf2[256];
    unsigned int ln = 0;
    unsigned int errors = 0;

    while (fgets(buf1, sizeof(buf1), fh))
    {
        ++ln;

        // Skip blank lines and comments (which are lines that begin with
        // octothorpes,) we don't need them.
        if (sscanf(buf1, " %s", buf2) == EOF) continue;
        if (sscanf(buf1, " %[#]", buf2) == 1) continue;

        // Get PREVIOUS_BLOCK_HASH.
        if (
            sscanf(buf1, " PREVIOUS_BLOCK_HASH = \"%64[0123456789ABCDEF]\"", buf2) == 1
            || sscanf(buf1, " PREVIOUS_BLOCK_HASH = '%64[0123456789ABCDEF]'", buf2) == 1
            || sscanf(buf1, " PREVIOUS_BLOCK_HASH = %64[0123456789ABCDEF]", buf2) == 1)
        {
            errors += error(args.previous_block_hash != NULL, "Overwriting PREVIOUS_BLOCK_HASH", ln);
            errors += error(strlen(buf2) != 64, "Expected a 64-byte PREVIOUS_BLOCK_HASH", ln);

            args.previous_block_hash = calloc(65, sizeof(char));
            memcpy(args.previous_block_hash, buf2, 64);

            continue;
        }

        // Get MESSAGE.
        if (
            sscanf(buf1, " MESSAGE = \"%[^\"]\"", buf2) == 1
            || sscanf(buf1, " MESSAGE = '%[^']'", buf2) == 1)
        {
            errors += error(args.message != NULL, "Overwriting MESSAGE", ln);
            errors += error(strlen(buf2) > 64, "Got more than 64 byte MESSAGE", ln);

            args.message = calloc(65, sizeof(char));
            memset(args.message, ' ', 64);
            memcpy(args.message, buf2, strlen(buf2));

            continue;
        }

        // Get THRESHOLD.
        if (
            sscanf(buf1, " THRESHOLD = \"%64[0123456789ABCDEF]\"", buf2) == 1
            || sscanf(buf1, " THRESHOLD = '%64[0123456789ABCDEF]'", buf2) == 1
            || sscanf(buf1, " THRESHOLD = %64[0123456789ABCDEF]", buf2) == 1)
        {
            errors += error(args.threshold != NULL, "Overwriting THRESHOLD", ln);
            errors += error(strlen(buf2) != 64, "Expected a 64-byte THRESHOLD", ln);

            // Convert the hex string to a uint8_t array.
            args.threshold = calloc(33, sizeof(uint8_t));
            for (unsigned int i = 0; i < 32; ++i)
                args.threshold[i] = (
                    16*HEX2DEC(buf2[2*i]) + HEX2DEC(buf2[2*i + 1])
                );

            continue;
        }

        // Otherwise, dunno!
        errors += error(true, "Parsing error", ln);
        ++errors;
    }

    // Check for unspecified arguments.
    errors += error(args.username            == NULL, "USERNAME is undefined; set it on the command line.", -1);
    errors += error(args.previous_block_hash == NULL, "PREVIOUS_BLOCK_HASH is undefined; set it in your .conf file.", -1);
    errors += error(args.message             == NULL, "MESSAGE is undefined; set it in your .conf file.", -1);
    errors += error(args.threshold           == NULL, "THRESHOLD is undefined; set it in your .conf file.", -1);

    if (errors > 0)
        printf("\nThere were %d fatal errors. Fix them, and then re-run.\n", errors), exit(1);

    fprintf(stderr,
        "INFO: Got the following input arguments. Please confirm that they are correct! (note that USERNAME and MESSAGE are both padded with spaces to the appropriate length.)\n");
    fprintf(stderr, " - USERNAME = \"%s\"\n", args.username);
    fprintf(stderr, " - PREVIOUS_BLOCK_HASH = \"%s\"\n", args.previous_block_hash);
    fprintf(stderr, " - MESSAGE = \"%s\"\n", args.message);
    fprintf(stderr, " - THRESHOLD = \"");
    for (int i = 0; i < 32; ++i)
        fprintf(stderr, "%02x", args.threshold[i]);
    fprintf(stderr, "\"\n");

    //
    // Compute the block hash.
    //

    SHA256_CTX ctx;

    unsigned char *pt = calloc(144, sizeof(unsigned char));
    memcpy(pt + 0 , MAGIC_BYTES             , 16);
    memcpy(pt + 16, args.previous_block_hash, 64);
    memcpy(pt + 80, args.message            , 64);

    unsigned char *block_hash = calloc(SHA256_BLOCK_SIZE, sizeof(unsigned char));

    sha256_init(&ctx);
    sha256_update(&ctx, pt, 144);
    sha256_final(&ctx, block_hash);

    free(pt);

    fprintf(stderr, " - BLOCK_HASH = \"");
    for (int i = 0; i < 32; ++i)
        fprintf(stderr, "%02x", block_hash[i]);
    fprintf(stderr, "\"\n");
    fprintf(stderr, "\n");

    //
    // Start mining!
    //

    time_t t;
    srand((unsigned)time(&t));

    pt = calloc(64, sizeof(unsigned char));
    memcpy(pt + 0 , block_hash, SHA256_BLOCK_SIZE);
    memcpy(pt + 32, args.username, 16);
    memcpy(pt + 60, AUTHOR_TOKEN, 4);
    
    uint8_t *nonce = calloc(12, sizeof(uint8_t));
    
    // Initialize the nonce with a random value.
    for (int i = 0; i < 12; ++i) {
        nonce[i] = rand() % CHARSET_SIZE;
        *(pt + 48 + i) = CHARSET[nonce[i]];
    }
    
    printf("INFO: Initializing nonce at ");
    for (unsigned char *it = pt + 48; it < pt + 64; ++it)
        printf("%c", *it);
    printf("\n");
 
    clock_t t1 = clock(), t2;
    uint64_t no = 0;
    while (1) {
        // Increment the nonce.
        for (int i = 11; i >= 0; --i) {
            nonce[i] = (nonce[i] + 1) % CHARSET_SIZE;
            *(pt + 48 + i) = CHARSET[nonce[i]];
            if (nonce[i])
                break;
        }

        sha256_init(&ctx);
        sha256_update(&ctx, pt, 64);
        sha256_final(&ctx, block_hash);

        // Is it small enough?
        for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
            if (block_hash[i] == args.threshold[i]) continue;
            if (block_hash[i] <  args.threshold[i]) break;
            if (block_hash[i] >  args.threshold[i]) goto skip;
        }

        printf("\nSUCCESS: Found a valid nonce,\n\n    ");
        for (unsigned char *it = pt + 48; it < pt + 64; ++it)
            printf("%c", *it);
        printf("\n\nThis produces a signature hash of ");
        for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
            printf("%02x", block_hash[i]);
        printf(".\n");

        return 0;

        skip:
        // Count number of attempts, so far.
        no++;
        if (no % 10000000 == 0) {
            t2 = clock();
            double runtime = (double)(t2 - t1) / CLOCKS_PER_SEC;
            printf("INFO: Currently ~%f h/s; %ld total so far...\n", 10000000.0 / runtime, no);

            t1 = clock();
        }
    }
}
