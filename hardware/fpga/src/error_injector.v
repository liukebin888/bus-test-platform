`timescale 1ns/1ps
// ============================================================================
// error_injector.v - Bit-level error frame injector (single channel)
// ----------------------------------------------------------------------------
// Injection types (v3.0 solution): bit / CRC / ACK / stuffing / glitch.
//   - BIT   : force tx line opposite at bit_position for 1 bit time
//   - CRC   : corrupt CRC field (bit_position = offset into CRC field)
//   - ACK   : force dominant ACK slot (bit_position = ACK slot offset)
//   - STUFF : insert extra stuff bit (bit_position = offset)
//   - GLITCH: toggle line for glitch_width_ns
// Timing driven by bit_tick strobe (one per bit period).
// ============================================================================
module error_injector #(
    parameter BIT_TIME_TICKS = 8    // 1 bit = 8 clk ticks @500kbit vs 4MHz
)(
    input  wire                clk,
    input  wire                rst_n,
    // control (MCU register)
    input  wire                cfg_we,
    input  wire [2:0]          cfg_type,      // 0=off,1=bit,2=crc,3=ack,4=stuff,5=glitch
    input  wire [15:0]         cfg_bit_pos,   // bit position within frame
    input  wire [7:0]          cfg_glitch_ns, // glitch duration in bit fractions
    // frame engine handshake
    input  wire                frame_active,  // frame currently on bus
    input  wire                bit_tick,      // strobe: next bit starts
    output reg                 inject         // asserted to corrupt line
);

    reg [2:0]  type_r;
    reg [15:0] bit_pos_r;
    reg [7:0]  glitch_ns_r;
    reg [15:0] bit_cnt;
    reg [7:0]  glitch_cnt;
    reg        armed;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            type_r     <= 3'd0;
            bit_pos_r  <= 16'd0;
            glitch_ns_r <= 8'd1;
        end else if (cfg_we) begin
            type_r     <= cfg_type;
            bit_pos_r  <= cfg_bit_pos;
            glitch_ns_r <= cfg_glitch_ns;
        end
    end

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            bit_cnt   <= 16'd0;
            glitch_cnt <= 8'd0;
            armed     <= 1'b0;
            inject    <= 1'b0;
        end else begin
            inject <= 1'b0;
            if (type_r == 3'd0) begin
                armed <= 1'b0;
            end else if (frame_active) begin
                if (bit_tick) begin
                    bit_cnt <= bit_cnt + 1'b1;
                    if (bit_cnt == bit_pos_r) begin
                        if (type_r == 3'd5) begin
                            // glitch: pulse for cfg_glitch_ns ticks
                            glitch_cnt <= glitch_ns_r;
                            armed <= 1'b1;
                        end else begin
                            // bit/crc/ack/stuff: single bit-time corruption
                            inject <= 1'b1;
                        end
                    end
                end
                if (armed) begin
                    inject <= 1'b1;
                    if (glitch_cnt == 8'd0)
                        armed <= 1'b0;
                    else
                        glitch_cnt <= glitch_cnt - 1'b1;
                end
            end else begin
                bit_cnt  <= 16'd0;
                glitch_cnt <= 8'd0;
                armed    <= 1'b0;
            end
        end
    end

endmodule
