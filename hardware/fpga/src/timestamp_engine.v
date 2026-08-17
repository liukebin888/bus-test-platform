`timescale 1ns/1ps
// ============================================================================
// timestamp_engine.v - Global 100 ns timestamp engine
// ----------------------------------------------------------------------------
// Reference clock: TCXO 10 MHz (period = exactly 100 ns).
// All captured bus frames get a uniform 64-bit tick value (v3.0: 100 ns res.).
// PPS adjustment (signed) comes from pps_sync.v for multi-device alignment.
// ============================================================================
module timestamp_engine #(
    parameter TS_WIDTH = 64
)(
    input  wire                clk,          // TCXO 10 MHz reference
    input  wire                rst_n,
    input  wire                capture_en,   // strobe: latch current count
    input  wire                pps_adj_en,   // strobe: apply signed adjustment
    input  wire signed [31:0]  adj_delta,    // signed tick adjustment
    output reg  [TS_WIDTH-1:0] ts_out,       // latched timestamp (100 ns ticks)
    output wire [TS_WIDTH-1:0] ts_now        // free-running counter (read-only)
);

    reg [TS_WIDTH-1:0] cnt;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            cnt <= {TS_WIDTH{1'b0}};
        end else if (pps_adj_en) begin
            cnt <= cnt + $signed({{(TS_WIDTH-32){adj_delta[31]}}, adj_delta});
        end else begin
            cnt <= cnt + 1'b1;
        end
    end

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            ts_out <= {TS_WIDTH{1'b0}};
        end else if (capture_en) begin
            ts_out <= cnt;
        end
    end

    assign ts_now = cnt;

endmodule
