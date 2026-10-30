module line_buffer_read_controller(
    input logic clk,
    input logic reset,
    input logic [9:0] x,
    input logic [9:0] y,
    input logic video_on_raw,

    input logic [1:0] produced_sync,
    output logic [1:0] consumed_toggle,

    output logic buffer_ready,
    output logic read_buffer,
    output logic vga_started

    );

    logic next_read_buffer;

    always_comb begin
        next_read_buffer = ~read_buffer;
    end

    logic buffer_ready_comb;
    logic buffer_ready_reg;

    assign buffer_ready_comb = produced_sync[read_buffer] != consumed_toggle[read_buffer];

    assign buffer_ready = (x == 10'd0 && !y[0] && video_on_raw) ? buffer_ready_comb : buffer_ready_reg;

    always_ff @(posedge clk)
    begin
        if (reset) begin
            vga_started <= 1'b0;
            read_buffer <= 1'b0;
            consumed_toggle <= 2'b00;
            buffer_ready_reg <= 1'b0;
        end
        else if(!vga_started && buffer_ready_comb) begin
            vga_started <= 1'b1;
        end
        else if (x == 10'd0 && !y[0] && video_on_raw) begin
            buffer_ready_reg <= buffer_ready_comb;
        end
        else if (x == 10'd639 && y[0] && video_on_raw && buffer_ready_reg) begin
            consumed_toggle[read_buffer] <= ~consumed_toggle[read_buffer];
            read_buffer <= next_read_buffer;
        end
    end
endmodule
