#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "sha256.h"

// Compile-time constants.
char *MAGIC_BYTES = "ISVENONACOINBEST";
char *AUTHOR_TOKEN = "QUX5";

struct Args
{
    char *username;
    char *previous_block_hash;
    char *message;
    uint8_t *threshold;
};

int main(int argc, char *argv[])
{
    assert(16 == strlen(MAGIC_BYTES ));
    assert( 4 == strlen(AUTHOR_TOKEN));

    if (argc != 2)
    {
        printf("usage: %s <vminer.conf>\n", argv[0]);
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

    FILE *fh = fopen(argv[1], "r");
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

        // Get USERNAME. 
        if (
            sscanf(buf1, " USERNAME = \"%[^\"]\"", buf2) == 1
            || sscanf(buf1, " USERNAME = '%[^']'", buf2) == 1
            || sscanf(buf1, " USERNAME = %s", buf2) == 1)
        {
            if (args.username != NULL)
            {
                fprintf(stderr,
                    "ERROR: Overwriting USERNAME on line %d.\n", ln);
                ++errors;
            } 

            if (strlen(buf2) > 16)
            {
                fprintf(stderr,
                    "ERROR: The USERNAME specified on line %d, \"%s\", is too long.", ln, buf2);
                ++errors;
            }

            args.username = calloc(16, sizeof(char));
            memset(args.username, ' ', 16);
            memcpy(args.username, buf2, strlen(buf2)); 

            continue;
        }

        // Get PREVIOUS_BLOCK_HASH.
        if (
            sscanf(buf1, " PREVIOUS_BLOCK_HASH = \"%64[0123456789ABCDEF]\"", buf2) == 1
            || sscanf(buf1, " PREVIOUS_BLOCK_HASH = '%64[0123456789ABCDEF]'", buf2) == 1
            || sscanf(buf1, " PREVIOUS_BLOCK_HASH = %64[0123456789ABCDEF]", buf2) == 1)
        {
            if (args.previous_block_hash != NULL)
            {
                fprintf(stderr,
                    "ERROR: Overwriting PREVIOUS_BLOCK_HASH on line %d.\n", ln);
                ++errors;
            }

            if (strlen(buf2) != 64)
            {
                fprintf(stderr,
                    "ERROR: The PREVIOUS_BLOCK_HASH specified on line %d is not 64 bytes long.", ln);
                ++errors;
            }

            args.previous_block_hash = calloc(64, sizeof(char));
            memcpy(args.previous_block_hash, buf2, 64);
        
            continue;
        }

        // Get MESSAGE.
        if (
            sscanf(buf1, " MESSAGE = \"%[^\"]\"", buf2) == 1
            || sscanf(buf1, " MESSAGE = '%[^']'", buf2) == 1)
        {
            if (args.message != NULL)
            {
                fprintf(stderr,
                    "ERROR: Overwriting MESSAGE on line %d.\n", ln);
                ++errors;
            } 

            if (strlen(buf2) > 64)
            {
                fprintf(stderr,
                    "ERROR: The MESSAGE specified on line %d is too long.", ln);
                ++errors;
            }

            args.message = calloc(64, sizeof(char));
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
            if (args.threshold != NULL)
            {
                fprintf(stderr,
                    "ERROR: Overwriting THRESHOLD on line %d.\n", ln);
                ++errors;
            }

            if (strlen(buf2) != 64)
            {
                fprintf(stderr,
                    "ERROR: The THRESHOLD specified on line %d is not 64 bytes long.", ln);
                ++errors;
            }

            // Convert the hex string to a uint8_t array.
            char *it = buf2;
            args.threshold = calloc(32, sizeof(uint8_t));
            for (unsigned int i = 0; i < 32; ++i, it = it + 2)
                sscanf(it, "%2hhx", &args.threshold[i]);
            
            continue;
        }

        // Otherwise, dunno!
        printf("ERROR: Couldn't parse line %d.\n", ln);
        ++errors;
    }

    // Check for unspecified arguments.
    if (args.username            == NULL) { fprintf(stderr, "ERROR: USERNAME is undefined; set it in %s.\n"           , argv[1]); ++errors; }
    if (args.previous_block_hash == NULL) { fprintf(stderr, "ERROR: PREVIOUS_BLOCK_HASH is undefined; set it in %s.\n", argv[1]); ++errors; }
    if (args.message             == NULL) { fprintf(stderr, "ERROR: MESSAGE is undefined; set it in %s.\n"            , argv[1]); ++errors; }
    if (args.threshold           == NULL) { fprintf(stderr, "ERROR: THRESHOLD is undefined; set it in %s.\n"          , argv[1]); ++errors; }

    if (errors > 0)
    {
        printf("\nThere were %d fatal errors. Fix them, and then re-run.\n", errors);
        exit(1);
    }

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
    
    //
    // Start mining!
    // 

    time_t t;
    srand((unsigned)time(&t));
    
    pt = calloc(64, sizeof(unsigned char));
    memcpy(pt + 0 , block_hash, SHA256_BLOCK_SIZE);
    memcpy(pt + 32, args.username, 16);
    memcpy(pt + 60, AUTHOR_TOKEN, 4);
    
    for (int i = 0; i < 12; ++i)
        *(pt + 48 + i) = (unsigned char)('a' + (rand() % 26));

    clock_t t1 = clock(), t2;
    int i = 0;
    while (1) {
        // Modify the nonce, slightly.
        *(pt + 48 + (i % 12)) = (unsigned char)('a' + (rand() % 26));

        sha256_init(&ctx);
        sha256_update(&ctx, pt, 64);
        sha256_final(&ctx, block_hash);

        // Is it small enough?
        for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
            if (block_hash[i] == args.threshold[i]) continue;
            if (block_hash[i] <  args.threshold[i]) break;
            if (block_hash[i] >  args.threshold[i]) goto skip;
        }

        printf("\nINFO: Found a valid nonce,\n\n    ");
        for (unsigned char *it = pt + 48; it < pt + 64; ++it)
            printf("%c", *it);
        printf("\n\nThis produces a signature hash of ");
        for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
            printf("%02x", block_hash[i]);
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
    }
}
