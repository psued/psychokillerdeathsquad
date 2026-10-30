module line_buffer_bram(
    input  logic         write_buffer,
    input  logic         [8:0] write_x,
    input  logic         write_enable,
    input  logic         write_clk,
    input  logic         [11:0] pixel_in,

    input  logic         read_buffer,
    input  logic         [8:0] read_x,
    input  logic         read_enable,
    input  logic         read_clk,
    output logic         [11:0] pixel_out
    );

    logic [11:0] buffer [0:639]; // Two scanlines of 320 RGB444 pixels each.

    logic [9:0] write_addr;
    logic [9:0] read_addr;

    assign write_addr = (write_buffer * 320) + write_x;
    assign read_addr  = (read_buffer * 320) + read_x;

    always_ff @(posedge write_clk)
    begin
        if (write_enable)
            buffer[write_addr] <= pixel_in;
    end

    always_ff @(posedge read_clk)
    begin
        if (read_enable) begin
            pixel_out <= buffer[read_addr];
        end
    end
endmodule
