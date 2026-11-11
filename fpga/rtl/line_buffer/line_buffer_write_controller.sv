module line_buffer_write_controller(
    input logic clk,
    input logic reset,

    input logic line_done,

    input logic [1:0] consumed_sync,
    output logic [1:0] produced_toggle,

    output logic write_buffer,
    output logic stall
    );

    assign stall = consumed_sync[write_buffer] != produced_toggle[write_buffer];

    always_ff @(posedge clk)
    begin
        if (reset) begin
            write_buffer <= 1'b0;
            produced_toggle <= 2'b00;
            end
        else if (!stall) begin
            if(line_done) begin
                produced_toggle[write_buffer] <= ~produced_toggle[write_buffer];
                write_buffer <= ~write_buffer;
            end
        end
    end
endmodule
