module line_buffer_write_controller(
    input logic clk,
    input logic reset,

    input logic [9:0] render_x,

    input logic [2:0] consumed_sync,
    output logic [2:0] produced_toggle,

    output logic [1:0]  write_buffer,
    output logic stall
    );

    assign stall = consumed_sync[write_buffer] != produced_toggle[write_buffer];

    always_ff @(posedge clk)
    begin
        if (reset) begin
            write_buffer <= 2'd0;
            produced_toggle <= 3'b000;
            end
        else if (!stall) begin
            if(render_x == 10'd639) begin
                produced_toggle[write_buffer] <= ~produced_toggle[write_buffer];
                if (write_buffer == 2'd2) begin
                    write_buffer <= 2'd0;
                end
                else begin
                    write_buffer <= write_buffer + 2'd1;
                end
            end
        end
    end
endmodule
