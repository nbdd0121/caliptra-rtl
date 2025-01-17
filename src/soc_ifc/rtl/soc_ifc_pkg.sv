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

`ifndef SOC_IFC_PKG
`define SOC_IFC_PKG

`include "config_defines.svh"
`include "caliptra_reg_defines.svh"

package soc_ifc_pkg;
    
    parameter SOC_IFC_ADDR_W = 18;
    parameter SOC_IFC_DATA_W = 32;
    parameter SOC_IFC_USER_W = 32;
    parameter SOC_IFC_ID_W   = `CALIPTRA_AXI_ID_WIDTH;
    
    parameter MBOX_SIZE_KB = 128;
    parameter MBOX_SIZE_BYTES = MBOX_SIZE_KB * 1024;
    parameter MBOX_SIZE_DWORDS = MBOX_SIZE_BYTES/4;
    parameter MBOX_DATA_W = 32;
    parameter MBOX_ECC_DATA_W = 7;
    parameter MBOX_DATA_AND_ECC_W = MBOX_DATA_W + MBOX_ECC_DATA_W;
    parameter MBOX_DEPTH = (MBOX_SIZE_KB * 1024 * 8) / MBOX_DATA_W;
    parameter MBOX_ADDR_W = $clog2(MBOX_DEPTH);

    parameter CPTRA_AXI_DMA_DATA_WIDTH = 32;
    parameter CPTRA_AXI_DMA_ID_WIDTH   = 5; // FIXME related to CALIPTRA_AXI_ID_WIDTH?
    parameter CPTRA_AXI_DMA_USER_WIDTH = 32;

    parameter WDT_TIMEOUT_PERIOD_NUM_DWORDS = 2;
    parameter WDT_TIMEOUT_PERIOD_W = WDT_TIMEOUT_PERIOD_NUM_DWORDS * 32;

    parameter SOC_IFC_REG_OFFSET = 32'h3000_0000;

    //memory map
    parameter MBOX_DIR_START_ADDR     = 32'h0000_0000;
    parameter MBOX_DIR_END_ADDR       = 32'h0001_FFFF;
    parameter MBOX_REG_START_ADDR     = `CLP_MBOX_CSR_BASE_ADDR - SOC_IFC_REG_OFFSET;
    parameter MBOX_REG_END_ADDR       = MBOX_REG_START_ADDR + 32'h0000_0FFF;
    parameter SHA_REG_START_ADDR      = `CLP_SHA512_ACC_CSR_BASE_ADDR - SOC_IFC_REG_OFFSET;
    parameter SHA_REG_END_ADDR        = SHA_REG_START_ADDR + 32'h0000_0FFF;
    parameter DMA_REG_START_ADDR      = `CLP_AXI_DMA_REG_BASE_ADDR - SOC_IFC_REG_OFFSET;
    parameter DMA_REG_END_ADDR        = DMA_REG_START_ADDR + 32'h0000_0FFF;
    parameter SOC_IFC_REG_START_ADDR  = `CLP_SOC_IFC_REG_BASE_ADDR - SOC_IFC_REG_OFFSET;
    parameter SOC_IFC_REG_END_ADDR    = SOC_IFC_REG_START_ADDR + 32'h0000_FFFF;
    parameter SOC_IFC_FUSE_START_ADDR = SOC_IFC_REG_START_ADDR + 32'h0000_0200;
    parameter SOC_IFC_FUSE_END_ADDR   = SOC_IFC_REG_START_ADDR + 32'h0000_05FF;

    //Valid AXI_USER
    //Lock the AXI_USER values from integration time
    parameter [4:0] CPTRA_SET_MBOX_AXI_USER_INTEG   = { 1'b0,          1'b0,          1'b0,          1'b0,          1'b0};
    parameter [4:0][31:0] CPTRA_MBOX_VALID_AXI_USER = {32'h4444_4444, 32'h3333_3333, 32'h2222_2222, 32'h1111_1111, 32'h0000_0000};
    parameter [31:0] CPTRA_DEF_MBOX_VALID_AXI_USER  = 32'hFFFF_FFFF;

    parameter CPTRA_SET_FUSE_AXI_USER_INTEG = 1'b0;
    parameter [31:0] CPTRA_FUSE_VALID_AXI_USER = 32'h0000_0000;

    //DMI Register encodings
    //Read only registers
    parameter DMI_REG_MBOX_DLEN = 7'h50;
    parameter DMI_REG_MBOX_DOUT = 7'h51;
    parameter DMI_REG_MBOX_STATUS = 7'h52;
    parameter DMI_REG_BOOT_STATUS = 7'h53;
    parameter DMI_REG_CPTRA_HW_ERRROR_ENC = 7'h54;
    parameter DMI_REG_CPTRA_FW_ERROR_ENC = 7'h55;
    parameter DMI_REG_SS_UDS_SEED_BASE_ADDR_L = 7'h56;
    parameter DMI_REG_SS_UDS_SEED_BASE_ADDR_H = 7'h57;
    parameter DMI_REG_HW_FATAL_ERROR = 7'h58;
    parameter DMI_REG_FW_FATAL_ERROR = 7'h59;
    parameter DMI_REG_HW_NON_FATAL_ERROR = 7'h5a;
    parameter DMI_REG_FW_NON_FATAL_ERROR = 7'h5b;
    //RW registers
    parameter DMI_REG_CPTRA_DBG_MANUF_SERVICE_REG = 7'h60;
    parameter DMI_REG_BOOTFSM_GO = 7'h61;
    parameter DMI_REG_MBOX_DIN = 7'h62;
    parameter DMI_REG_SS_DEBUG_INTENT = 7'h63;
    parameter DMI_REG_SS_CALIPTRA_BASE_ADDR_L = 7'h64;
    parameter DMI_REG_SS_CALIPTRA_BASE_ADDR_H = 7'h65;
    parameter DMI_REG_SS_MCI_BASE_ADDR_L = 7'h66;
    parameter DMI_REG_SS_MCI_BASE_ADDR_H = 7'h67;
    parameter DMI_REG_SS_RECOVERY_IFC_BASE_ADDR_L = 7'h68;
    parameter DMI_REG_SS_RECOVERY_IFC_BASE_ADDR_H = 7'h69;
    parameter DMI_REG_SS_OTP_FC_BASE_ADDR_L = 7'h6A;
    parameter DMI_REG_SS_OTP_FC_BASE_ADDR_H = 7'h6B;
    parameter DMI_REG_SS_STRAP_GENERIC_0 = 7'h6C;
    parameter DMI_REG_SS_STRAP_GENERIC_1 = 7'h6D;
    parameter DMI_REG_SS_STRAP_GENERIC_2 = 7'h6E;
    parameter DMI_REG_SS_STRAP_GENERIC_3 = 7'h6F;
    parameter DMI_REG_SS_DBG_MANUF_SERVICE_REG_REQ = 7'h70;
    parameter DMI_REG_SS_DBG_MANUF_SERVICE_REG_RSP = 7'h71;
    parameter DMI_REG_SS_DBG_UNLOCK_LEVEL0 = 7'h72;
    parameter DMI_REG_SS_DBG_UNLOCK_LEVEL1 = 7'h73;

    
    // This parameter describes the hard-coded implementation in the BOOT FSM
    // that results in noncore reset assertion being delayed from the soft reset
    // (cptra_rst_b) by some integer number of clock cycles, due to synchronization
    // stages and the rst window signaling.
    // This is useful in validation environments for controlling the predicted
    // timing in a reset event.
    parameter SOC_IFC_CPTRA_RST_NONCORE_RST_DELAY = 4;

    //BOOT FSM
    typedef enum logic [2:0] {
        BOOT_IDLE   = 3'b000,
        BOOT_FUSE   = 3'b001,
        BOOT_FW_RST = 3'b010,
        BOOT_WAIT   = 3'b011,
        BOOT_DONE   = 3'b100
    } boot_fsm_state_e;

    //MAILBOX FSM
    typedef enum logic [2:0] {
        MBOX_IDLE         = 3'b000,
        MBOX_RDY_FOR_CMD  = 3'b001,
        MBOX_RDY_FOR_DLEN = 3'b011,
        MBOX_RDY_FOR_DATA = 3'b010,
        MBOX_EXECUTE_UC   = 3'b110,
        MBOX_EXECUTE_SOC  = 3'b100,
        MBOX_EXECUTE_TAP  = 3'b101,
        MBOX_ERROR        = 3'b111
    } mbox_fsm_state_e;

    //SHA FSM
    typedef enum logic [2:0] {
        SHA_IDLE    = 3'b000,
        SHA_BLOCK_0 = 3'b001,
        SHA_BLOCK_N = 3'b011,
        SHA_PAD0    = 3'b010,
        SHA_PAD1    = 3'b110,
        SHA_DONE    = 3'b100
      } sha_fsm_state_e;

    //MAILBOX Status
    typedef enum logic [3:0] {
        CMD_BUSY      = 4'd0,
        DATA_READY    = 4'd1,
        CMD_COMPLETE  = 4'd2,
        CMD_FAILURE   = 4'd3
    } mbox_status_e;

    //Any request into soc ifc block
    typedef struct packed {
        logic   [SOC_IFC_ADDR_W-1:0]   addr;
        logic   [SOC_IFC_DATA_W-1:0]   wdata;
        logic   [SOC_IFC_DATA_W/8-1:0] wstrb;
        logic   [SOC_IFC_USER_W-1:0]   user;
        logic   [SOC_IFC_ID_W  -1:0]   id;
        logic                          write;
        logic                          soc_req;
    } soc_ifc_req_t;
    // ECC protected data
    typedef struct packed {
        logic [MBOX_ECC_DATA_W-1:0] ecc;
        logic [MBOX_DATA_W-1:0]     data;
    } mbox_sram_data_t;
    //Request to mbox sram
    typedef struct packed {
        logic cs;
        logic we;
        logic [MBOX_ADDR_W-1:0] addr;
        mbox_sram_data_t wdata;
    } mbox_sram_req_t;
    //Response from mbox sram
    typedef struct packed {
        mbox_sram_data_t rdata;
    } mbox_sram_resp_t;

    typedef struct packed {
        logic cptra_iccm_ecc_single_error;
        logic cptra_iccm_ecc_double_error;
        logic cptra_dccm_ecc_single_error;
        logic cptra_dccm_ecc_double_error;
    } rv_ecc_sts_t;

    typedef struct packed {
        logic axs_without_lock;
        logic axs_incorrect_order;
    } mbox_protocol_error_t;

    typedef enum logic [1:0] {
        DEVICE_UNPROVISIONED = 2'b00,
        DEVICE_MANUFACTURING = 2'b01,
        DEVICE_PRODUCTION    = 2'b11
    } device_lifecycle_e;

    typedef struct packed {
        logic debug_locked;
        device_lifecycle_e device_lifecycle;
    } security_state_t;

    typedef struct packed {
        logic [31:0] MBOX_DLEN;
        logic [31:0] MBOX_DOUT;
        logic [31:0] MBOX_STATUS;
    } mbox_dmi_reg_t;

endpackage

`endif
