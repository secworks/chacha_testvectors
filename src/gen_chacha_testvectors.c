
//======================================================================
//
// gen_chacha_testvectors.c
// ------------------------
// ChaCha reference model and test vector generator. Very much
// needed for ChaCha. The code is heavily based on the chacha ref
// model by DJB. This code is self contained, provides test vectors
// and is somewhat ceaned up. (Does not calls functions Salsa20-
// something etc.)
//
//
// Copyright (c) 2013 Secworks Sweden AB
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or
// without modification, are permitted provided that the following
// conditions are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materials provided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//======================================================================

//------------------------------------------------------------------
// Includes.
//------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>


//------------------------------------------------------------------
// Types.
//------------------------------------------------------------------
// The chacha state context.
typedef struct
{
  uint32_t state[16];
  uint8_t rounds;
} chacha_ctx;


//------------------------------------------------------------------
// Macros.
//------------------------------------------------------------------
// Basic 32-bit operators.
#define ROTATE(v,c) ((uint32_t)((v) << (c)) | ((v) >> (32 - (c))))
#define XOR(v,w) ((v) ^ (w))
#define PLUS(v,w) ((uint32_t)((v) + (w)))
#define PLUSONE(v) (PLUS((v), 1))

// Little endian machine assumed (x86-64).
#define U32TO8_LITTLE(p, v) (((uint32_t*)(p))[0] = v)
#define U8TO32_LITTLE(p) (((uint32_t*)(p))[0])

#define QUARTERROUND(a, b, c, d) \
  x[a] = PLUS(x[a],x[b]); x[d] = ROTATE(XOR(x[d],x[a]),16); \
  x[c] = PLUS(x[c],x[d]); x[b] = ROTATE(XOR(x[b],x[c]),12); \
  x[a] = PLUS(x[a],x[b]); x[d] = ROTATE(XOR(x[d],x[a]), 8); \
  x[c] = PLUS(x[c],x[d]); x[b] = ROTATE(XOR(x[b],x[c]), 7);


//------------------------------------------------------------------
// Constants.
//------------------------------------------------------------------
static const uint8_t SIGMA[16] = "expand 32-byte k";
static const uint8_t TAU[16]   = "expand 16-byte k";


//------------------------------------------------------------------
// doublerounds()
//
// Perform rounds/2 number of doublerounds.
// TODO: Change output format to 16 words.
//------------------------------------------------------------------
static void doublerounds(uint8_t output[64], const uint32_t input[16], uint8_t rounds)
{
  uint32_t x[16];
  int32_t i;

  for (i = 0;i < 16;++i) {
    x[i] = input[i];
  }

  for (i = rounds ; i > 0 ; i -= 2) {
    QUARTERROUND( 0, 4, 8,12)
    QUARTERROUND( 1, 5, 9,13)
    QUARTERROUND( 2, 6,10,14)
    QUARTERROUND( 3, 7,11,15)

    QUARTERROUND( 0, 5,10,15)
    QUARTERROUND( 1, 6,11,12)
    QUARTERROUND( 2, 7, 8,13)
    QUARTERROUND( 3, 4, 9,14)
  }

  for (i = 0;i < 16;++i) {
    x[i] = PLUS(x[i], input[i]);
  }

  for (i = 0;i < 16;++i) {
    U32TO8_LITTLE(output + 4 * i, x[i]);
  }
}


//------------------------------------------------------------------
// init()
//
// Initializes the given cipher context with key, iv and constants.
// This also resets the block counter.
//------------------------------------------------------------------
void init(chacha_ctx *x, uint8_t *key, uint32_t keylen, uint8_t *iv)
{
  if (keylen == 256) {
    // 256 bit key.
    x->state[0]  = U8TO32_LITTLE(SIGMA + 0);
    x->state[1]  = U8TO32_LITTLE(SIGMA + 4);
    x->state[2]  = U8TO32_LITTLE(SIGMA + 8);
    x->state[3]  = U8TO32_LITTLE(SIGMA + 12);
    x->state[4]  = U8TO32_LITTLE(key + 0);
    x->state[5]  = U8TO32_LITTLE(key + 4);
    x->state[6]  = U8TO32_LITTLE(key + 8);
    x->state[7]  = U8TO32_LITTLE(key + 12);
    x->state[8]  = U8TO32_LITTLE(key + 16);
    x->state[9]  = U8TO32_LITTLE(key + 20);
    x->state[10] = U8TO32_LITTLE(key + 24);
    x->state[11] = U8TO32_LITTLE(key + 28);
  }

  else {
    // 128 bit key.
    x->state[0]  = U8TO32_LITTLE(TAU + 0);
    x->state[1]  = U8TO32_LITTLE(TAU + 4);
    x->state[2]  = U8TO32_LITTLE(TAU + 8);
    x->state[3]  = U8TO32_LITTLE(TAU + 12);
    x->state[4]  = U8TO32_LITTLE(key + 0);
    x->state[5]  = U8TO32_LITTLE(key + 4);
    x->state[6]  = U8TO32_LITTLE(key + 8);
    x->state[7]  = U8TO32_LITTLE(key + 12);
    x->state[8]  = U8TO32_LITTLE(key + 0);
    x->state[9]  = U8TO32_LITTLE(key + 4);
    x->state[10] = U8TO32_LITTLE(key + 8);
    x->state[11] = U8TO32_LITTLE(key + 12);
  }

  // Reset block counter and add IV to state.
  x->state[12] = 0;
  x->state[13] = 0;
  x->state[14] = U8TO32_LITTLE(iv + 0);
  x->state[15] = U8TO32_LITTLE(iv + 4);
}


//------------------------------------------------------------------
// next()
//
// Given a pointer to the next block m of 64 cleartext bytes will
// use the given context to transform (encrypt/decrypt) the
// block. The result will be stored in c.
//------------------------------------------------------------------
void next(chacha_ctx *ctx, const uint8_t *m, uint8_t *c)
{
  // Temporary internal state x.
  uint8_t x[64];
  uint8_t i;


  // Update the internal state and increase the block counter.
  doublerounds(x, ctx->state, ctx->rounds);
  ctx->state[12] = PLUSONE(ctx->state[12]);
  if (!ctx->state[12]) {
    ctx->state[13] = PLUSONE(ctx->state[13]);
  }

  // XOR the input block with the new temporal state to
  // create the transformed block.
  for (i = 0 ; i < 64 ; ++i) {
    c[i] = m[i] ^ x[i];
  }
}


//------------------------------------------------------------------
// init_ctx()
//
// Init a given ChaCha context by setting state to zero and
// setting the given number of rounds.
//------------------------------------------------------------------
void init_ctx(chacha_ctx *ctx, uint8_t rounds)
{
  uint8_t i;

  for (i = 0 ; i < 16 ; i++) {
    ctx->state[i] = 0;
  }

  ctx->rounds = rounds;
}


//------------------------------------------------------------------
// print_ctx()
//
// Print the state of the given context.
//------------------------------------------------------------------
void print_ctx(chacha_ctx *ctx)
{
  uint8_t i;

  for (i = 0; i < 16; i+= 2) {
    printf("state[%02d - %02d] = 0x%08x 0x%08x\n",
           i, (i + 1), ctx->state[i], ctx->state[(i + 1)]);
  }
  printf("\n");
}


//------------------------------------------------------------------
// print_block()
//
// Print the contents of the given 64 bytes block.
//------------------------------------------------------------------
void print_block(uint8_t block[64])
{
  uint8_t i;

  for (i = 0 ; i < 64 ; i++) {
    /* if ((i % 8) == 0) { */
    /*     printf("block[%02d - %02d]: ", i, (i + 7)); */
    /*   } */

    printf("0x%02x ", block[i]);

    if (((i + 1) % 8) == 0) {
      printf("\n");
    }
  }
  printf("\n");
}


//------------------------------------------------------------------
// print_key_iv()
//
// Print the given key and iv.
//------------------------------------------------------------------
void print_key_iv(uint8_t *key, uint32_t keylen,  uint8_t *iv) {
  uint8_t i;

  printf("Key:    ");
  for (i = 0 ; i < (keylen / 8) ; i++) {
    if ((i > 0) && (0 == (i % 8))) {
        printf("\n        ");
      }
    printf("0x%02x ", key[i]);
  }
  printf("\n");

  printf("IV:     ");
  for (i = 0 ; i < 8 ; i++) {
    printf("0x%02x ", iv[i]);
  }
  printf("\n");
}


//------------------------------------------------------------------
// gen_testvectors()
//
// Given a key and iv generates test vectors for 128 and 256 bit
// keys, for 1 and 2 blocks all for 8, 10, 12 and 20 rounds.
//------------------------------------------------------------------
void gen_testvectors(uint8_t *key, uint8_t *iv)
{
  uint32_t keylengths[2] = {128, 256};
  uint8_t rounds[4] = {8, 12, 20};

  uint8_t data[64] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  uint8_t result0[64] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  uint8_t result1[64] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  uint32_t ki;
  uint8_t ri;
  chacha_ctx my_ctx;

  // For a given key and iv we process two consecutive blocks
  // using 8, 12 or 20 rounds.
  for (ki = 0 ; ki < 2 ; ki++) {
    for (ri = 0 ; ri < 3 ; ri++) {
      print_key_iv(key, keylengths[ki], iv);
      printf("Rounds: %d\n\n", rounds[ri]);

      init_ctx(&my_ctx, rounds[ri]);
      init(&my_ctx, key, keylengths[ki], iv);

      printf("Internal state after init:\n");
      print_ctx(&my_ctx);

      next(&my_ctx, data, result0);

      next(&my_ctx, data, result1);

      printf("Keystream block 0:\n");
      print_block(result0);
      printf("Keystream block 1:\n");
      print_block(result1);
      printf("\n");
    }
  }
}


//------------------------------------------------------------------
// main()
//
// Set up context and generate test vectors for different
// combinations of key, iv, blocks etc.
//------------------------------------------------------------------
int main(void)
{
  printf("Test vectors for the ChaCha stream cipher\n");
  printf("=========================================\n\n");


  printf("TC1: All zero key and IV.\n");
  printf("-------------------------\n");
  uint8_t tc1_key[32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t tc1_iv[8]   = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  gen_testvectors(tc1_key, tc1_iv);
  printf("\n");


  printf("TC2: Single bit in key set. All zero IV.\n");
  printf("----------------------------------------\n");
  uint8_t tc2_key[32] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t tc2_iv[8]   = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  gen_testvectors(tc2_key, tc2_iv);
  printf("\n");


  printf("TC3: Single bit in IV set. All zero key.\n");
  printf("----------------------------------------\n");
  uint8_t tc3_key[32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t tc3_iv[8]   = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  gen_testvectors(tc3_key, tc3_iv);
  printf("\n");


  printf("TC4: All bits in key and IV are set.\n");
  printf("------------------------------------\n");
  uint8_t tc4_key[32] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  uint8_t tc4_iv[8]   = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  gen_testvectors(tc4_key, tc4_iv);
  printf("\n");


  printf("TC5: Every even bit set in key and IV.\n");
  printf("--------------------------------------\n");
  uint8_t tc5_key[32] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                         0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                         0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                         0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
  uint8_t tc5_iv[8]   = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
  gen_testvectors(tc5_key, tc5_iv);
  printf("\n");


  printf("TC6: Every odd bit set in key and IV.\n");
  printf("-------------------------------------\n");
  uint8_t tc6_key[32] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                         0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                         0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                         0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
  uint8_t tc6_iv[8]   = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
  gen_testvectors(tc6_key, tc6_iv);
  printf("\n");


  // TC4: Sequence patterns.
  printf("TC7: Sequence patterns in key and IV.\n");
  printf("-------------------------------------\n");
  uint8_t tc7_key[32] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                         0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
                         0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88,
                         0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};
  uint8_t tc7_iv[8]   = {0x0f, 0x1e, 0x2d, 0x3c, 0x4b, 0x5a, 0x69, 0x78};
  gen_testvectors(tc7_key, tc7_iv);
  printf("\n");


  // TC 8: A random key and IV.
  // key: echo -n "All your base are belong to us" | openssl dgst -sha256
  // IV: echo -n "Internet Engineering Task Force" | openssl dgst -sha256
  printf("TC8: key: 'All your base are belong to us!, IV: 'IETF2013'\n");
  printf("----------------------------------------------------------\n");
  uint8_t tc8_key[32] = {0xc4, 0x6e, 0xc1, 0xb1, 0x8c, 0xe8, 0xa8, 0x78,
                         0x72, 0x5a, 0x37, 0xe7, 0x80, 0xdf, 0xb7, 0x35,
                         0x1f, 0x68, 0xed, 0x2e, 0x19, 0x4c, 0x79, 0xfb,
                         0xc6, 0xae, 0xbe, 0xe1, 0xa6, 0x67, 0x97, 0x5d};
  uint8_t tc8_iv[8]   = {0x1a, 0xda, 0x31, 0xd5, 0xcf, 0x68, 0x82, 0x21};
  gen_testvectors(tc8_key, tc8_iv);

  return 0;
}

//======================================================================
// EOF gen_chacha_testvectors.c
//======================================================================
