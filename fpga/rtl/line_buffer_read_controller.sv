module line_buffer_read_controller(
    input logic clk,
    input logic reset,
    input logic [9:0] x,
    input logic video_on_raw,

    input logic [2:0] produced_sync,
    output logic [2:0] consumed_toggle,

    output logic buffer_ready,
    output logic [1:0]  read_buffer,
    output logic vga_started

    );

    logic [1:0] next_read_buffer;

    always_comb begin
        if (read_buffer == 2'd2) begin
            next_read_buffer = 2'd0;
        end
        else begin
            next_read_buffer = read_buffer + 2'd1;
        end
    end

    logic buffer_ready_comb;
    logic buffer_ready_reg;

    assign buffer_ready_comb = produced_sync[read_buffer] != consumed_toggle[read_buffer];

    assign buffer_ready = (x == 10'd0) ? buffer_ready_comb : buffer_ready_reg;

    always_ff @(posedge clk)
    begin
        if (reset) begin
            vga_started <= 1'b0;
            read_buffer <= 2'd0;
            consumed_toggle <= 3'b000;
            buffer_ready_reg <= 1'b0;
        end
        else if(!vga_started && buffer_ready) begin
            vga_started <= 1'b1;
        end
        else if (x == 10'd0) begin
            buffer_ready_reg <= buffer_ready_comb;
        end
        else if (x == 10'd639 && video_on_raw && buffer_ready_reg) begin
            consumed_toggle[read_buffer] <= ~consumed_toggle[read_buffer];
            read_buffer <= next_read_buffer;
        end
    end
endmodule
