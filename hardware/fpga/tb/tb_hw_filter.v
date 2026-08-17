`timescale 1ns/1ps
// Testbench: hw_filter mask / range / type modes.
module tb_hw_filter;

    reg clk = 0;
    reg rst_n = 0;
    reg cfg_we = 0;
    reg [28:0] cfg_mask = 29'h0;
    reg [28:0] cfg_value = 29'h0;
    reg [28:0] cfg_lo = 29'h0;
    reg [28:0] cfg_hi = 29'h1FFF;
    reg cfg_accept_ext = 1'b1;
    reg [28:0] frame_id = 29'h0;
    reg frame_ext = 0;
    reg frame_valid = 0;
    wire pass;

    hw_filter #(.ID_WIDTH(29)) dut (
        .clk(clk), .rst_n(rst_n), .cfg_we(cfg_we),
        .cfg_mask(cfg_mask), .cfg_value(cfg_value),
        .cfg_lo(cfg_lo), .cfg_hi(cfg_hi), .cfg_accept_ext(cfg_accept_ext),
        .frame_id(frame_id), .frame_ext(frame_ext),
        .frame_valid(frame_valid), .pass(pass)
    );

    always #5 clk = ~clk;

    integer errors = 0;

    task check(input [28:0] id, input ext, input expected);
        begin
            @(posedge clk);
            frame_id = id;
            frame_ext = ext;
            frame_valid = 1;
            #1;
            if (pass !== expected) begin
                $display("FAIL: id=%h ext=%b expected=%b got=%b", id, ext, expected, pass);
                errors = errors + 1;
            end
        end
    endtask

    initial begin
        #100 rst_n = 1;
        #20;

        // mask mode: accept only id 0x123
        cfg_we = 1;
        cfg_mask = 29'h1FFF;
        cfg_value = 29'h123;
        @(posedge clk);
        cfg_we = 0;

        check(29'h123, 0, 1'b1);
        check(29'h124, 0, 1'b0);
        check(29'h123, 1, 1'b0); // type mismatch (ext) => reject

        // range mode: 0x100..0x200
        cfg_we = 1;
        cfg_mask = 0;
        cfg_value = 0;
        cfg_lo = 29'h100;
        cfg_hi = 29'h200;
        @(posedge clk);
        cfg_we = 0;

        check(29'h150, 0, 1'b1);
        check(29'h0FF, 0, 1'b0);
        check(29'h201, 0, 1'b0);

        if (errors == 0)
            $display("PASS: hw_filter");
        else
            $display("FAIL: hw_filter (%0d errors)", errors);
        $finish;
    end

endmodule
