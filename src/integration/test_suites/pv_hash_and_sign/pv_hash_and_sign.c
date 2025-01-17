// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "caliptra_defines.h"
#include "caliptra_isr.h"
#include "riscv-csr.h"
#include "riscv_hw_if.h"
#include <string.h>
#include <stdint.h>
#include "printf.h"
#include "keyvault.h"
#include "sha512.h"

volatile uint32_t* stdout           = (uint32_t *)STDOUT;
volatile uint32_t  intr_count = 0;
#ifdef CPT_VERBOSITY
    enum printf_verbosity verbosity_g = CPT_VERBOSITY;
#else
    enum printf_verbosity verbosity_g = MEDIUM;
#endif

volatile caliptra_intr_received_s cptra_intr_rcv = {0};

void main() {
    VPRINTF(LOW,"---------------------------\n");
    VPRINTF(LOW," KV PCR Hash Extend Test !!\n");
    VPRINTF(LOW,"---------------------------\n");

    uint32_t msg[] = {0x00000000,
                      0x00000000,
                      0x00000000,
                      0x00000000,
                      0x00000000,
                      0x00000000,
                      0x00000000,
                      0x00000000,
                      0x00000000,
                      0x00000000,
                      0x00000000,
                      0x00000000};

    uint32_t exp1[] = {0xf57bb7ed,
                       0x82c6ae4a,
                       0x29e6c987,
                       0x9338c592,
                       0xc7d42a39,
                       0x135583e8,
                       0xccbe3940,
                       0xf2344b0e,
                       0xb6eb8503,
                       0xdb0ffd6a,
                       0x39ddd00c,
                       0xd07d8317};

    uint32_t exp2[] = {0x11143121,
                       0xbeb365e6,
                       0x3826e7de,
                       0x89f9c76a,
                       0xe1100411,
                       0xfb9643d1,
                       0x98e730b7,
                       0x603a83a4,
                       0x977c76ee,
                       0xe6ddf74f,
                       0xa0b43fbf,
                       0x49897978};

    uint32_t exp3[] = {0x2ab50775,
                       0xdab80928,
                       0xb8de848c,
                       0xabbdfa56,
                       0x897ed7b1,
                       0x45be930e,
                       0x1e340ce0,
                       0x8c5bfc73,
                       0x1d997144,
                       0x0c43f5a5,
                       0xaa48ac99,
                       0x8dbe6bb9};

    uint32_t exp_sign_r[] = {0x871e6ea4, 
                             0xddc5432c, 
                             0xddaa60fd, 
                             0x7f055472, 
                             0xd3c4dd41, 
                             0xa5bfb267, 
                             0x09e88c31, 
                             0x1a970935, 
                             0x99a7c8f5, 
                             0x5b3974c1, 
                             0x9e4f5a7b, 
                             0xfc1dd2ac};

    uint32_t exp_sign_s[] = {0x3e5552de, 
                             0x6403350e, 
                             0xe70ad74e, 
                             0x4b854d2d, 
                             0xc4126bbf, 
                             0x9c153a5d, 
                             0x7a07bd4b, 
                             0x85d06e45, 
                             0xf850920e, 
                             0x898fb7d3, 
                             0x4f80796d, 
                             0xae29365c};

    uint32_t nonce[] = {0x01234567,
                        0x11111111,
                        0x22222222,
                        0x33333333,
                        0x44444444,
                        0x55555555,
                        0x66666666,
                        0x77777777};

    volatile uint32_t* reg_ptr;
    uint8_t offset;
    uint32_t read_data;
    uint32_t reg;

    //init_interrupts();

    //set hash extend for entry 0
    pv_hash_extend(0x1f);
    kv_poll_valid(CLP_SHA512_REG_SHA512_VAULT_RD_STATUS);
    kv_error_check(CLP_SHA512_REG_SHA512_VAULT_RD_STATUS);

    //Write junk to test lock
    reg_ptr = (uint32_t*) CLP_SHA512_REG_SHA512_BLOCK_0;
    while (reg_ptr < (uint32_t*) CLP_SHA512_REG_SHA512_BLOCK_12) {
        *reg_ptr++ = 0x01234567;
    }
    //Extend hash
    VPRINTF(MEDIUM,"KV: Writing SHA Block with known answer\n");
    offset = 0;
    while (reg_ptr < (uint32_t*) CLP_SHA512_REG_SHA512_BLOCK_24) {
        *reg_ptr++ = msg[offset++];
    }
    //Add padding to extended value
    VPRINTF(MEDIUM,"KV: Padding extended hash value\n");
    *reg_ptr++ = 0x80000000;
    while (reg_ptr < (uint32_t*) CLP_SHA512_REG_SHA512_BLOCK_31) {
        *reg_ptr++ = 0x00000000;
    }
    //Add length to last dword
    *reg_ptr = 0x00000300;

    //run the sha function
    VPRINTF(MEDIUM,"KV: polling for sha ready\n");
    sha512_poll_ready();
    sha_init_last(SHA512_384_MODE);
    VPRINTF(MEDIUM,"KV: polling for sha valid\n");
    sha512_poll_valid();
    VPRINTF(MEDIUM,"KV: polling for kv valid\n");
    kv_poll_valid(CLP_SHA512_REG_SHA512_KV_WR_STATUS);
    kv_error_check(CLP_SHA512_REG_SHA512_KV_WR_STATUS);

    //check expected output from PCR31
    reg_ptr = (uint32_t*) CLP_PV_REG_PCR_ENTRY_31_0;
    offset = 0;
    while (reg_ptr <= (uint32_t*) CLP_PV_REG_PCR_ENTRY_31_11) {
        read_data = *reg_ptr++;
        if (exp1[offset] != read_data) {
            VPRINTF(FATAL,"SHA Result Mismatch - EXP: 0x%x RECVD: 0x%x\n", exp1[offset], read_data);
            SEND_STDOUT_CTRL( 0x01);
        }
        offset++;
    }

    //read 384 bits from PCR31 and write to SHA512 BLOCK
    VPRINTF(MEDIUM,"KV: polling for kv ready\n");
    kv_poll_ready(CLP_SHA512_REG_SHA512_VAULT_RD_STATUS);
    pv_hash_extend(0x1f);
    VPRINTF(MEDIUM,"KV: polling for kv valid\n");
    kv_poll_valid(CLP_SHA512_REG_SHA512_VAULT_RD_STATUS);
    kv_error_check(CLP_SHA512_REG_SHA512_VAULT_RD_STATUS);

    //Write junk to test lock
    reg_ptr = (uint32_t*) CLP_SHA512_REG_SHA512_BLOCK_0;
    while (reg_ptr < (uint32_t*) CLP_SHA512_REG_SHA512_BLOCK_12) {
        *reg_ptr++ = 0x01234567;
    }
    //Extend hash
    VPRINTF(MEDIUM,"KV: Writing SHA Block with known answer\n");
    offset = 0;
    while (reg_ptr < (uint32_t*) CLP_SHA512_REG_SHA512_BLOCK_24) {
        *reg_ptr++ = msg[offset++];
    }
    //Add padding to extended value
    VPRINTF(MEDIUM,"KV: Padding extended hash value\n");
    *reg_ptr++ = 0x80000000;
    while (reg_ptr < (uint32_t*) CLP_SHA512_REG_SHA512_BLOCK_31) {
        *reg_ptr++ = 0x00000000;
    }
    //Add length to last dword
    *reg_ptr = 0x00000300;

    VPRINTF(MEDIUM,"KV: polling for sha ready\n");
    sha512_poll_ready();
    sha_init_last(SHA512_384_MODE);
    VPRINTF(MEDIUM,"KV: polling for sha valid\n");
    sha512_poll_valid();
    VPRINTF(MEDIUM,"KV: polling for kv valid\n");
    kv_poll_valid(CLP_SHA512_REG_SHA512_KV_WR_STATUS);
    kv_error_check(CLP_SHA512_REG_SHA512_KV_WR_STATUS);

    //check expected output from PCR31
    reg_ptr = (uint32_t*) CLP_PV_REG_PCR_ENTRY_31_0;
    offset = 0;
    while (reg_ptr <= (uint32_t*) CLP_PV_REG_PCR_ENTRY_31_11) {
        read_data = *reg_ptr++;
        if (exp2[offset] != read_data) {
            VPRINTF(FATAL,"SHA Result Mismatch - EXP: 0x%x RECVD: 0x%x\n", exp2[offset], read_data);
            SEND_STDOUT_CTRL( 0x01);
        }
        offset++;
    }

    //check endianness of result
    //Write first msg to block
    offset = 0;
    reg_ptr = (uint32_t*) CLP_SHA512_REG_SHA512_BLOCK_0;
    while (reg_ptr < (uint32_t*) CLP_SHA512_REG_SHA512_BLOCK_12) {
        *reg_ptr++ = msg[offset++];
    }
    //Extend hash
    VPRINTF(MEDIUM,"KV: Writing SHA Block with known answer\n");
    offset = 0;
    while (reg_ptr < (uint32_t*) CLP_SHA512_REG_SHA512_BLOCK_24) {
        *reg_ptr++ = msg[offset++];
    }
    //Add padding to extended value
    VPRINTF(MEDIUM,"KV: Padding extended hash value\n");
    *reg_ptr++ = 0x80000000;
    while (reg_ptr < (uint32_t*) CLP_SHA512_REG_SHA512_BLOCK_31) {
        *reg_ptr++ = 0x00000000;
    }
    //Add length to last dword
    *reg_ptr = 0x00000300;

    //run the sha function
    VPRINTF(MEDIUM,"KV: polling for sha ready\n");
    sha512_poll_ready();
    sha_init_last(SHA512_384_MODE);
    VPRINTF(MEDIUM,"KV: polling for sha valid\n");
    sha512_poll_valid();

    //check expected output from digest
    reg_ptr = (uint32_t*) CLP_SHA512_REG_SHA512_DIGEST_0;
    offset = 0;
    while (reg_ptr <= (uint32_t*) CLP_SHA512_REG_SHA512_DIGEST_11) {
        read_data = *reg_ptr++;
        if (exp1[offset] != read_data) {
            VPRINTF(FATAL,"SHA Result Mismatch - EXP: 0x%x RECVD: 0x%x\n", exp1[offset], read_data);
            SEND_STDOUT_CTRL( 0x01);
        }
        offset++;
    }

    sha_poll_gen_hash_ready();
    reg_ptr = (uint32_t*) CLP_SHA512_REG_SHA512_GEN_PCR_HASH_NONCE_0;
    offset = 0;
    while (reg_ptr <= (uint32_t*) CLP_SHA512_REG_SHA512_GEN_PCR_HASH_NONCE_7) {
        *reg_ptr++ = nonce[offset++];
    }
    sha_gen_hash_start();
    sha_poll_gen_hash_valid();

    //check expected output from digest
    reg_ptr = (uint32_t*) CLP_SHA512_REG_SHA512_GEN_PCR_HASH_DIGEST_0;
    offset = 0;
    while (reg_ptr <= (uint32_t*) CLP_SHA512_REG_SHA512_GEN_PCR_HASH_DIGEST_11) {
        read_data = *reg_ptr++;
        if (exp3[offset] != read_data) {
            VPRINTF(FATAL,"SHA Result Mismatch - EXP: 0x%x RECVD: 0x%x\n", exp3[offset], read_data);
            SEND_STDOUT_CTRL( 0x01);
        }
        offset++;
    }

    //inject seed to kv key reg (in RTL)
    printf("ECC: Inject PRIVKEY into KV slot 7\n");
    printf("ECC: Inject MSG into msg_reg\n");
    printf("%c", 0x90);

    VPRINTF(MEDIUM,"ECC: Running PCR Sign Function\n");
    //run ECC signing on PCR
    reg = ((1 << ECC_REG_ECC_CTRL_PCR_SIGN_LOW) & ECC_REG_ECC_CTRL_PCR_SIGN_MASK) |
          ((2 << ECC_REG_ECC_CTRL_CTRL_LOW) & ECC_REG_ECC_CTRL_CTRL_MASK) |
          ((0 << ECC_REG_ECC_CTRL_ZEROIZE_LOW) & ECC_REG_ECC_CTRL_ZEROIZE_MASK);
    lsu_write_32(CLP_ECC_REG_ECC_CTRL,reg);

    VPRINTF(MEDIUM,"ECC: Polling for PCR Sign to be complete\n");
    // wait for ECC SIGNING process to be done
    while((lsu_read_32(CLP_ECC_REG_ECC_STATUS) & ECC_REG_ECC_STATUS_READY_MASK) == 0);

    //check expected output from sign r
    reg_ptr = (uint32_t*) CLP_ECC_REG_ECC_SIGN_R_0;
    offset = 0;
    while (reg_ptr <= (uint32_t*) CLP_ECC_REG_ECC_SIGN_R_11) {
        read_data = *reg_ptr++;
        if (exp_sign_r[offset] != read_data) {
            VPRINTF(FATAL,"ECC SIGN R Result Mismatch - EXP: 0x%x RECVD: 0x%x\n", exp_sign_r[offset], read_data);
            SEND_STDOUT_CTRL( 0x01);
        }
        offset++;
    }

    //check expected output from sign s
    reg_ptr = (uint32_t*) CLP_ECC_REG_ECC_SIGN_S_0;
    offset = 0;
    while (reg_ptr <= (uint32_t*) CLP_ECC_REG_ECC_SIGN_S_11) {
        read_data = *reg_ptr++;
        if (exp_sign_s[offset] != read_data) {
            VPRINTF(FATAL,"ECC SIGN S Result Mismatch - EXP: 0x%x RECVD: 0x%x\n", exp_sign_s[offset], read_data);
            SEND_STDOUT_CTRL( 0x01);
        }
        offset++;
    }

    VPRINTF(LOW,"----------------------------------\n");
    VPRINTF(LOW," KV PCR Hash Extend Test Complete!\n");
    VPRINTF(LOW,"----------------------------------\n");
    
    SEND_STDOUT_CTRL( 0xff);
}
