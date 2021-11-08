// main.c
// Copyright (c) 2021.

/**
 * vminer [v2.0.0]
 * A VenonaCoin miner.
 */

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define ROR32(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROR32(x,2) ^ ROR32(x,13) ^ ROR32(x,22))
#define EP1(x) (ROR32(x,6) ^ ROR32(x,11) ^ ROR32(x,25))
#define SIG0(x) (ROR32(x,7) ^ ROR32(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROR32(x,17) ^ ROR32(x,19) ^ ((x) >> 10))

#define HEX2DEC(x) ((((x) ^ 16) + 7) % 39)

static const uint32_t k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static const uint32_t w1[64] = {
    0x80000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000200,
    0x80000000, 0x01400000, 0x00205000, 0x00005088, 0x22000800, 0x22550014, 0x05089742, 0xa0000020,
    0x5a880000, 0x005c9400, 0x0016d49d, 0xfa801f00, 0xd33225d0, 0x11675959, 0xf6e6bfda, 0xb30c1549,
    0x08b2b050, 0x9d7c4c27, 0x0ce2a393, 0x88e6e1ea, 0xa52b4335, 0x67a16f49, 0xd732016f, 0x4eeb2e91,
    0x5dbf55e5, 0x8eee2335, 0xe2bc5ec2, 0xa83f4394, 0x45ad78f7, 0x36f3d0cd, 0xd99c05e8, 0xb0511dc7,
    0x69bc7ac4, 0xbd11375b, 0xe3ba71e5, 0x3b209ff2, 0x18feee17, 0xe25ad9e7, 0x13375046, 0x0515089d,
    0x4f0d0f04, 0x2627484e, 0x310128d2, 0xc668b434, 0x420841cc, 0x62d311b8, 0xe59ba771, 0x85a7a484,
};

static const uint8_t *CHARSET =
    (uint8_t *)"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static const size_t CHARSET_SIZE = 62;

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "usage: %s <block hash> <username> <threshold>\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t i, j;

    // Setup m.
    uint8_t m[64];
    memset(m, ' ', 64);

    // Get `block hash` argument.
    if (strlen(argv[1]) != 64)
    {
        fprintf(stderr, "PANIC: `block hash` argument should be 64 uppercase hex characters.\n");
        return EXIT_FAILURE;
    }
    for (i = 0; i < 32; ++i)
        m[i] = (
            16*HEX2DEC(argv[1][2*i]) + HEX2DEC(argv[1][2*i + 1])
        );

    // Get `username` argument.
    if (strlen(argv[2]) > 16)
    {
        fprintf(stderr, "PANIC: `username` argument is too long!\n");
        return EXIT_FAILURE;
    }
    memcpy(m + 32, argv[2], strlen(argv[2]));

    // Set miner token.
    memset(m + 48, '_', 16);
    m[60] = 'Q';
    m[61] = 'U';
    m[62] = 'X';
    m[63] = '5';

    // Get `threshold` argument.
    uint32_t threshold[8];
    if (strlen(argv[3]) != 64)
    {
        fprintf(stderr, "PANIC: `threshold` argument should be 64 uppercase hex characters.\n");
        return EXIT_FAILURE;
    }
    for (i = 0; i < 8; ++i)
        threshold[i] = (
              ((uint32_t)(HEX2DEC(argv[3][8*i + 0])) << 28)
            + ((uint32_t)(HEX2DEC(argv[3][8*i + 1])) << 24)
            + ((uint32_t)(HEX2DEC(argv[3][8*i + 2])) << 20)
            + ((uint32_t)(HEX2DEC(argv[3][8*i + 3])) << 16)
            + ((uint32_t)(HEX2DEC(argv[3][8*i + 4])) << 12)
            + ((uint32_t)(HEX2DEC(argv[3][8*i + 5])) << 8)
            + ((uint32_t)(HEX2DEC(argv[3][8*i + 6])) << 4)
            + ((uint32_t)(HEX2DEC(argv[3][8*i + 7])) << 0)
        );

    // Pick a starting nonce.
    time_t t;
    srand((unsigned)time(&t));

    uint8_t nonce[12];
    for (i = 0; i < 12; ++i)
        nonce[i] = rand() % CHARSET_SIZE, m[48 + i] = CHARSET[nonce[i]];

    printf("INFO: Starting attack at ");
    for (i = 0; i < 16; ++i)
        printf("%c", m[48 + i]);
    printf(".\n");

    clock_t c1 = clock(), c2;
    for (uint64_t no = 1;; ++no)
    {
        // Increment the nonce.
        for (i = 11; i >= 0; --i)
        {
            nonce[i] = (nonce[i] + 1) % CHARSET_SIZE;
            m[48 + i] = CHARSET[nonce[i]];
            if (nonce[i])
                break;
        }

        // Now, calculate the digest!
        uint32_t h0 = 0x6a09e667,
            h1 = 0xbb67ae85,
            h2 = 0x3c6ef372,
            h3 = 0xa54ff53a,
            h4 = 0x510e527f,
            h5 = 0x9b05688c,
            h6 = 0x1f83d9ab,
            h7 = 0x5be0cd19;
        uint32_t a, b, c, d, e, f, g, h;
        uint32_t t1, t2;
        uint32_t w0[64];

        // Chunk 0
        for (i = 0, j = 0; i < 16; ++i, j = j + 4)
            w0[i] = (m[j] << 24) | (m[j + 1] << 16) | (m[j + 2] << 8) | (m[j + 3]);
        for (; i < 64; ++i)
            w0[i] = SIG1(w0[i - 2]) + w0[i - 7] + SIG0(w0[i - 15]) + w0[i - 16];

        a = 0x6a09e667;
        b = 0xbb67ae85;
        c = 0x3c6ef372;
        d = 0xa54ff53a;
        e = 0x510e527f;
        f = 0x9b05688c;
        g = 0x1f83d9ab;
        h = 0x5be0cd19;

        for (i = 0; i < 64; ++i)
        {
            t1 = h + EP1(e) + CH(e, f, g) + k[i] + w0[i];
            t2 = EP0(a) + MAJ(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        h0 += a; h1 += b; h2 += c; h3 += d; h4 += e; h5 += f; h6 += g; h7 += h;

        // Chunk 1
        a = h0; b = h1; c = h2; d = h3; e = h4; f = h5; g = h6; h = h7;

        for (i = 0; i < 64; ++i) {
            t1 = h + EP1(e) + CH(e, f, g) + k[i] + w1[i];
            t2 = EP0(a) + MAJ(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        h0 += a; h1 += b; h2 += c; h3 += d; h4 += e; h5 += f; h6 += g; h7 += h;

        // Now, check if it's less than the threshold.
        if (h0 > threshold[0]) goto reject;
        if (h0 < threshold[0]) goto accept;
        if (h1 > threshold[1]) goto reject;
        if (h1 < threshold[1]) goto accept;
        if (h2 > threshold[2]) goto reject;
        if (h2 < threshold[2]) goto accept;
        if (h3 > threshold[3]) goto reject;
        if (h3 < threshold[3]) goto accept;

        accept:
        printf("SUCCESS: Found nonce ");
        for (i = 0; i < 16; ++i)
            printf("%c", m[48 + i]);
        printf(".\n");

        reject:
        if (no % 10000000 == 0) {
            c2 = clock();
            double runtime = (double)(c2 - c1) / CLOCKS_PER_SEC;
            printf("INFO: Currently averaging ~%f h/s...\n", 10000000.0 / runtime);

            c1 = clock();
        }
    }

    return EXIT_SUCCESS;
}
