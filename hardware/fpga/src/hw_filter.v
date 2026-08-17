`timescale 1ns/1ps
// ============================================================================
// hw_filter.v - Single-channel hardware ID filter
// ----------------------------------------------------------------------------
// Three filtering modes, OR-ed (accept if any enabled mode matches):
//   MASK_MODE : (id & mask) == value
//   RANGE_MODE: lo <= id <= hi
//   TYPE_MODE : accept only frames with given extended bit
// Zero CPU cost; instantiated once per channel (4 channels parallel in top).
// ============================================================================
module hw_filter #(
    parameter ID_WIDTH = 29   // 11-bit std / 29-bit ext (incl. ext flag in bit 28)
)(
    input  wire                 clk,
    input  wire                 rst_n,
    // runtime configuration (written by MCU over the register interface)
    input  wire                 cfg_we,
    input  wire [ID_WIDTH-1:0]  cfg_mask,
    input  wire [ID_WIDTH-1:0]  cfg_value,
    input  wire [ID_WIDTH-1:0]  cfg_lo,
    input  wire [ID_WIDTH-1:0]  cfg_hi,
    input  wire                 cfg_accept_ext,
    // frame under evaluation
    input  wire [ID_WIDTH-1:0]  frame_id,
    input  wire                 frame_ext,
    input  wire                 frame_valid,
    output reg                  pass            // combinatorial accept decision
);

    reg [ID_WIDTH-1:0] mask_r;
    reg [ID_WIDTH-1:0] value_r;
    reg [ID_WIDTH-1:0] lo_r;
    reg [ID_WIDTH-1:0] hi_r;
    reg                accept_ext_r;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            mask_r      <= {ID_WIDTH{1'b0}};
            value_r     <= {ID_WIDTH{1'b0}};
            lo_r        <= {ID_WIDTH{1'b0}};
            hi_r        <= {ID_WIDTH{1'b1}};
            accept_ext_r <= 1'b1;
        end else if (cfg_we) begin
            mask_r       <= cfg_mask;
            value_r      <= cfg_value;
            lo_r         <= cfg_lo;
            hi_r         <= cfg_hi;
            accept_ext_r <= cfg_accept_ext;
        end
    end

    wire mask_hit  = ((frame_id & mask_r) == value_r);
    wire range_hit = (frame_id >= lo_r) && (frame_id <= hi_r);
    wire type_hit  = (frame_ext == accept_ext_r);

    always @(*) begin
        pass = frame_valid && (mask_hit || range_hit || type_hit);
    end

endmodule
