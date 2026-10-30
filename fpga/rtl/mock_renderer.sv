module mock_renderer(
    input logic clk,
    input logic reset,
    input logic stall,

    output logic [8:0] write_x,
    output logic [8:0] render_x,
    output logic       write_enable,
    output logic [11:0] pixel_in,
    output logic       line_done
    );

    logic [7:0] render_line;

    always_ff @(posedge clk)
    begin
        if (reset) begin
                render_x <= 9'd0;
                render_line <= 8'd0;
                line_done <= 1'b0;
            end
        else begin
            line_done <= 1'b0;

            if (!stall && !line_done) begin
                if(render_x == 9'd319) begin
                    line_done <= 1'b1;
                    render_x <= 9'd0;

                    if(render_line == 8'd239)
                        render_line <= 8'd0;
                    else
                        render_line <= render_line + 8'd1;
                end
                else begin
                    render_x <= render_x + 9'd1;
                end
            end
        end
    end

    assign write_x = render_x;
    assign write_enable = !reset && !stall && !line_done;

    logic [3:0] gradient_y;
    logic [3:0] gradient_x;

    // Sixteen color levels across each visible axis.
    assign gradient_y = render_line / 10'd15;
    assign gradient_x = render_x / 10'd20;

    assign pixel_in = {
        gradient_y,          // red increases downward
        gradient_x,          // green increases to the right
        4'hF - gradient_y    // blue decreases downward
    };
endmodule
