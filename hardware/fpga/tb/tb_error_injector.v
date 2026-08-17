`timescale 1ns/1ps
// Testbench: error_injector single bit-time corruption.
module tb_error_injector;

    reg clk = 0;
    reg rst_n = 0;
    reg cfg_we = 0;
    reg [2:0] cfg_type = 3'd1;
    reg [15:0] cfg_bit_pos = 16'd5;
    reg [7:0] cfg_glitch_ns = 8'd3;
    reg frame_active = 0;
    reg bit_tick = 0;
    wire inject;

    error_injector dut (
        .clk(clk), .rst_n(rst_n), .cfg_we(cfg_we),
        .cfg_type(cfg_type), .cfg_bit_pos(cfg_bit_pos),
        .cfg_glitch_ns(cfg_glitch_ns),
        .frame_active(frame_active), .bit_tick(bit_tick), .inject(inject)
    );

    always #5 clk = ~clk;

    integer errors = 0;

    task wait_ticks(input integer n);
        integer k;
        begin
            for (k = 0; k < n; k = k + 1) @(posedge clk);
        end
    endtask

    initial begin
        #100 rst_n = 1;
        #20;

        // configure: bit error at position 5
        cfg_we = 1;
        @(posedge clk);
        cfg_we = 0;

        frame_active = 1;
        // advance bits 0..4 (no injection)
        wait_ticks(5);
        if (inject !== 1'b0) begin
            $display("FAIL: early injection at bit<5");
            errors = errors + 1;
        end
        // bit 5: one strobe => inject
        bit_tick = 1;
        @(posedge clk);
        bit_tick = 0;
        @(posedge clk);
        if (inject !== 1'b1) begin
            $display("FAIL: no injection at bit 5");
            errors = errors + 1;
        end
        wait_ticks(4);
        if (inject !== 1'b0) begin
            $display("FAIL: injection not cleared");
            errors = errors + 1;
        end

        frame_active = 0;
        wait_ticks(2);

        if (errors == 0)
            $display("PASS: error_injector");
        else
            $display("FAIL: error_injector (%0d errors)", errors);
        $finish;
    end

endmodule
