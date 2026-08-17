`timescale 1ns/1ps
// ============================================================================
// tx_scheduler.v - Hardware TX scheduler (periodic / event / burst)
// ----------------------------------------------------------------------------
// Table-driven: each slot stores {period_ticks, id, dlc, payload_base}.
// A 1 ms base tick (from timestamp engine) drives deterministic dispatch;
// jitter is bounded by the scheduler resolution (< 100 us v3.0 target).
// Frame payload lives in an external RAM; this module only generates the
// dispatch pulse and slot index.
// ============================================================================
module tx_scheduler #(
    parameter SLOTS      = 16,
    parameter SLOT_WIDTH = 4,        // $clog2(SLOTS)
    parameter TICK_WIDTH = 32
)(
    input  wire                clk,
    input  wire                rst_n,
    // configuration writes (one per slot)
    input  wire                cfg_we,
    input  wire [SLOT_WIDTH-1:0] cfg_slot,
    input  wire [TICK_WIDTH-1:0] cfg_period_ticks,  // 0 = slot disabled
    // scheduler engine
    input  wire [TICK_WIDTH-1:0] base_tick,         // 100 ns tick from TS engine
    input  wire                slot_ack,            // dispatch acknowledged
    output reg  [SLOT_WIDTH-1:0] dispatch_slot,     // slot to transmit
    output reg                 dispatch_pulse
);

    // per-slot period and next-due tick
    reg [TICK_WIDTH-1:0] period [0:SLOTS-1];
    reg [TICK_WIDTH-1:0] next   [0:SLOTS-1];
    reg [SLOT_WIDTH-1:0] scan;

    integer i;
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            for (i = 0; i < SLOTS; i = i + 1) begin
                period[i] <= {TICK_WIDTH{1'b0}};
                next[i]   <= {TICK_WIDTH{1'b0}};
            end
        end else if (cfg_we) begin
            period[cfg_slot] <= cfg_period_ticks;
            next[cfg_slot]   <= base_tick + cfg_period_ticks; // first dispatch
        end
    end

    // scan all slots each base tick boundary; dispatch the first due slot
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            scan           <= {SLOT_WIDTH{1'b0}};
            dispatch_pulse <= 1'b0;
            dispatch_slot  <= {SLOT_WIDTH{1'b0}};
        end else begin
            dispatch_pulse <= 1'b0;
            if (slot_ack) begin
                next[dispatch_slot] <= next[dispatch_slot] + period[dispatch_slot];
            end
            if (scan == 0) begin
                scan <= 1;
            end else if (scan < SLOTS) begin
                if (period[scan] != 0 && base_tick >= next[scan]) begin
                    dispatch_slot  <= scan;
                    dispatch_pulse <= 1'b1;
                    scan           <= 0;
                end else begin
                    scan <= scan + 1'b1;
                end
            end else begin
                scan <= 0;
            end
        end
    end

endmodule
