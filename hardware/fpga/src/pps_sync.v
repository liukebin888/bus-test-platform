`timescale 1ns/1ps
// ============================================================================
// pps_sync.v - PPS multi-device synchronization
// ----------------------------------------------------------------------------
// Detects the (debounced) PPS rising edge, compares the local 100 ns counter
// against the expected epoch period, and produces a signed adjustment for
// timestamp_engine so that all cascaded devices stay within < 1 us.
// ============================================================================
module pps_sync #(
    parameter TS_WIDTH = 64,
    parameter SYNC_TICKS = 20   // debounce ticks
)(
    input  wire                clk,
    input  wire                rst_n,
    input  wire                pps_in,          // raw PPS input
    input  wire [TS_WIDTH-1:0] ts_now,          // local timestamp (100 ns)
    output reg                 pps_detected,    // strobe on valid edge
    output reg  signed [31:0]  adj_delta,       // signed adjustment (ticks)
    output reg                 adj_valid        // strobe to timestamp_engine
);

    reg [SYNC_TICKS-1:0] debounce_shift;
    wire pps_filtered = &debounce_shift;        // all-ones => high
    reg  pps_prev;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            debounce_shift <= {SYNC_TICKS{1'b0}};
        end else begin
            debounce_shift <= {debounce_shift[SYNC_TICKS-2:0], pps_in};
        end
    end

    // PPS nominal period at 100 ns ticks: 1 second = 10_000_000 ticks.
    localparam NOMINAL_PERIOD = 32'd10_000_000;

    reg [TS_WIDTH-1:0] last_ts;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            pps_prev     <= 1'b0;
            pps_detected <= 1'b0;
            adj_valid    <= 1'b0;
            adj_delta    <= 32'sd0;
            last_ts      <= {TS_WIDTH{1'b0}};
        end else begin
            pps_detected <= 1'b0;
            adj_valid    <= 1'b0;
            pps_prev     <= pps_filtered;
            if (pps_filtered && !pps_prev) begin
                // rising edge of PPS
                pps_detected <= 1'b1;
                adj_delta    <= $signed(NOMINAL_PERIOD) -
                                $signed(ts_now - last_ts);
                adj_valid    <= 1'b1;
                last_ts      <= ts_now;
            end
        end
    end

endmodule
