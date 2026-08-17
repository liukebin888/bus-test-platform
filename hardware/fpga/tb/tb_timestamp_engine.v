`timescale 1ns/1ps
// Testbench: timestamp_engine + pps_sync basic behavior.
module tb_timestamp_engine;

    reg clk = 0;
    reg rst_n = 0;
    reg capture_en = 0;
    reg pps_adj_en = 0;
    reg signed [31:0] adj_delta = 0;
    wire [63:0] ts_out;
    wire [63:0] ts_now;

    timestamp_engine #(.TS_WIDTH(64)) dut (
        .clk(clk), .rst_n(rst_n), .capture_en(capture_en),
        .pps_adj_en(pps_adj_en), .adj_delta(adj_delta),
        .ts_out(ts_out), .ts_now(ts_now)
    );

    // 10 MHz clock: period 100 ns
    always #50 clk = ~clk;

    initial begin
        $dumpfile("tb_timestamp_engine.vcd");
        $dumpvars(0, dut);

        #200 rst_n = 1;
        #1000; // 10 ticks elapsed

        // capture
        @(posedge clk) capture_en = 1;
        @(posedge clk) capture_en = 0;
        $display("ts_now = %0d, ts_out = %0d", ts_now, ts_out);
        if (ts_out !== 10) begin
            $display("FAIL: expected ts_out=10, got %0d", ts_out);
            $finish;
        end

        // pps adjustment: apply -5 ticks
        adj_delta = -32'sd5;
        @(posedge clk) pps_adj_en = 1;
        @(posedge clk) pps_adj_en = 0;
        $display("after adj: ts_now = %0d", ts_now);
        if (ts_now !== 6) begin
            $display("FAIL: expected ts_now=6, got %0d", ts_now);
            $finish;
        end

        $display("PASS: timestamp_engine");
        $finish;
    end

endmodule
