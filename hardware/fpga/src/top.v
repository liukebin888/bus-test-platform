`timescale 1ns/1ps
// ============================================================================
// top.v - FPGA coprocessor top level (Artix-7 XC7A35T / EG4S20)
// ----------------------------------------------------------------------------
// Connects the six IP blocks behind a simple 32-bit register interface used
// by STM32H750 (addr/data/we). Per-channel instances: filter/injector/sampler.
//
// Register map (read/write, 8-bit address, 32-bit data):
//   0x00 TS_LOW     : timestamp capture (write = latch, read = latched low)
//   0x04 TS_HIGH    : latched high word
//   0x08 TS_NOW_LOW : free-running counter low  (RO)
//   0x0C TS_NOW_HIGH: free-running counter high (RO)
//   0x10 PPS_DELTA  : last PPS adjustment (RO)
//   0x20 FLT_CFG(ch): filter config for channel ch (base + ch)
//   0x30 INJ_CFG(ch): injector config for channel ch
//   0x40 SMP_CTRL(ch): sampler control for channel ch (bit0 arm, bit1 stop)
//   0x50 TXS_CFG(slot): tx scheduler slot config
// ============================================================================
module top #(
    parameter CHANNELS = 4,
    parameter SLOTS    = 16
)(
    input  wire                clk,          // TCXO 10 MHz
    input  wire                rst_n,
    input  wire                pps_in,
    // MCU register interface
    input  wire [7:0]          mcu_addr,
    input  wire [31:0]         mcu_wdata,
    input  wire                mcu_we,
    output reg  [31:0]         mcu_rdata
);

    // ---- timestamp engine / pps sync ----
    wire [63:0] ts_now;
    wire [63:0] ts_captured;
    wire        pps_detected;
    wire signed [31:0] pps_adj;
    wire        pps_adj_valid;
    reg         capture_latch;

    timestamp_engine #(.TS_WIDTH(64)) u_ts (
        .clk        (clk),
        .rst_n      (rst_n),
        .capture_en (capture_latch),
        .pps_adj_en (pps_adj_valid),
        .adj_delta  (pps_adj),
        .ts_out     (ts_captured),
        .ts_now     (ts_now)
    );

    pps_sync #(.TS_WIDTH(64)) u_pps (
        .clk        (clk),
        .rst_n      (rst_n),
        .pps_in     (pps_in),
        .ts_now     (ts_now),
        .pps_detected(pps_detected),
        .adj_delta  (pps_adj),
        .adj_valid  (pps_adj_valid)
    );

    // ---- per-channel filter / injector / sampler ----
    genvar ch;
    generate
        for (ch = 0; ch < CHANNELS; ch = ch + 1) begin : g_ch
            wire [28:0] frame_id;
            wire        frame_ext;
            wire        frame_valid;
            wire        filter_pass;
            wire        inject;

            // Filter: config register writes are demuxed by address.
            wire        flt_we = mcu_we && (mcu_addr[7:4] == 4'h2) &&
                                 (mcu_addr[3:0] == ch[3:0]);
            wire [28:0] flt_mask  = mcu_wdata[28:0];
            wire [28:0] flt_value = mcu_wdata[28:0];
            wire [28:0] flt_lo    = mcu_wdata[28:0];
            wire [28:0] flt_hi    = mcu_wdata[28:0];
            wire        flt_ext   = mcu_wdata[31];

            hw_filter #(.ID_WIDTH(29)) u_filter (
                .clk       (clk),
                .rst_n     (rst_n),
                .cfg_we    (flt_we),
                .cfg_mask  (flt_mask),
                .cfg_value (flt_value),
                .cfg_lo    (flt_lo),
                .cfg_hi    (flt_hi),
                .cfg_accept_ext (flt_ext),
                .frame_id  (frame_id),
                .frame_ext (frame_ext),
                .frame_valid(frame_valid),
                .pass      (filter_pass)
            );

            // Injector: single 32-bit config word.
            wire inj_we = mcu_we && (mcu_addr[7:4] == 4'h3) &&
                          (mcu_addr[3:0] == ch[3:0]);
            error_injector u_injector (
                .clk        (clk),
                .rst_n      (rst_n),
                .cfg_we     (inj_we),
                .cfg_type   (mcu_wdata[2:0]),
                .cfg_bit_pos(mcu_wdata[18:3]),
                .cfg_glitch_ns(mcu_wdata[26:19]),
                .frame_active(1'b0),     // wired from CAN engine (TODO)
                .bit_tick   (1'b0),      // wired from bit engine (TODO)
                .inject     (inject)
            );

            // Sampler: arm/stop bits only in this skeleton.
            wire smp_we = mcu_we && (mcu_addr[7:4] == 4'h4) &&
                          (mcu_addr[3:0] == ch[3:0]);
            phys_sampler #(.DEPTH(4096), .ADDR_WIDTH(12)) u_sampler (
                .adc_clk   (clk),
                .rst_n     (rst_n),
                .bus_level (1'b0),       // wired from transceiver (TODO)
                .arm       (mcu_wdata[0]),
                .stop      (mcu_wdata[1]),
                .trig      (1'b0),
                .capture_on(),
                .rd_clk    (clk),
                .rd_addr   (12'd0),
                .rd_data   ()
            );
        end
    endgenerate

    // ---- tx scheduler ----
    wire [3:0]  txs_slot;
    wire        txs_pulse;
    tx_scheduler #(.SLOTS(SLOTS), .SLOT_WIDTH(4), .TICK_WIDTH(32)) u_txs (
        .clk          (clk),
        .rst_n        (rst_n),
        .cfg_we       (mcu_we && (mcu_addr[7:4] == 4'h5)),
        .cfg_slot     (mcu_addr[3:0]),
        .cfg_period_ticks(mcu_wdata[31:0]),
        .base_tick    (ts_now[31:0]),
        .slot_ack     (txs_pulse),
        .dispatch_slot(txs_slot),
        .dispatch_pulse(txs_pulse)
    );

    // ---- register read mux ----
    reg [63:0] ts_latched;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n)
            capture_latch <= 1'b0;
        else begin
            capture_latch <= 1'b0;
            if (mcu_we && (mcu_addr == 8'h00))
                capture_latch <= 1'b1;
        end
    end

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n)
            ts_latched <= 64'd0;
        else if (capture_latch)
            ts_latched <= ts_captured;
    end

    always @(*) begin
        case (mcu_addr)
            8'h00: mcu_rdata = ts_latched[31:0];
            8'h04: mcu_rdata = ts_latched[63:32];
            8'h08: mcu_rdata = ts_now[31:0];
            8'h0C: mcu_rdata = ts_now[63:32];
            8'h10: mcu_rdata = pps_adj;
            default: mcu_rdata = 32'hDEAD_BEEF;
        endcase
    end

endmodule
