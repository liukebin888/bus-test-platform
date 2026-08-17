`timescale 1ns/1ps
// ============================================================================
// phys_sampler.v - Physical layer sampler with ring buffer (per channel)
// ----------------------------------------------------------------------------
// Samples the bus level at ADC_CLK (up to 1 GS/s via ISERDES in real design).
// Samples land in a ring buffer (depth parameterized; 256 KB total across
// channels in the target design). Trigger/stop logic supports capture on a
// configured ID match, then MCU streams the waveform out.
// ============================================================================
module phys_sampler #(
    parameter DEPTH       = 4096,   // ring buffer depth (samples)
    parameter ADDR_WIDTH  = 12      // $clog2(DEPTH)
)(
    input  wire                adc_clk,    // high-speed sample clock
    input  wire                rst_n,
    input  wire                bus_level,  // sampled bus level
    // control
    input  wire                arm,        // start capturing (until stop/trig)
    input  wire                stop,       // stop capture, freeze buffer
    input  wire                trig,       // trigger (ID match): record index
    output reg                 capture_on,
    // read interface (MCU, slow clock domain; use proper CDC in real design)
    input  wire                rd_clk,
    input  wire [ADDR_WIDTH-1:0] rd_addr,
    output wire                rd_data
);

    reg  [ADDR_WIDTH-1:0] wp;
    reg  [DEPTH-1:0]      mem;

    always @(posedge adc_clk or negedge rst_n) begin
        if (!rst_n) begin
            wp          <= {ADDR_WIDTH{1'b0}};
            capture_on  <= 1'b0;
        end else begin
            if (arm)   capture_on <= 1'b1;
            if (stop)  capture_on <= 1'b0;
            if (capture_on) begin
                mem[wp] <= bus_level;
                wp      <= wp + 1'b1;
            end
            if (trig) wp <= wp;   // keep trigger index stable (freeze point)
        end
    end

    // async read (simple; real design uses dual-port RAM / CDC)
    reg [ADDR_WIDTH-1:0] rd_addr_r;
    always @(posedge rd_clk or negedge rst_n) begin
        if (!rst_n) rd_addr_r <= {ADDR_WIDTH{1'b0}};
        else        rd_addr_r <= rd_addr;
    end
    assign rd_data = mem[rd_addr_r];

endmodule
