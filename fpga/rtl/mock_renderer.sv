module mock_renderer(
    input logic clk,
    input logic reset,
    input logic stall,

    output logic [9:0] write_x,
    output logic [9:0] render_x,
    output logic       write_enable,
    output logic [11:0] pixel_in
    );

    logic [9:0] render_line;

    always_ff @(posedge clk)
    begin
        if (reset) begin
                render_x <= 10'd0;
                render_line <= 10'd0;
            end
        else if (!stall) begin
            if(render_x == 10'd639) begin
                render_x <= 10'd0;

                if(render_line == 10'd479)
                    render_line <= 10'd0;
                else
                    render_line <= render_line + 10'd1;
            end
            else begin
                render_x <= render_x + 10'd1;
            end
        end
    end

    assign write_x = render_x;
    assign write_enable = !reset && !stall;

    logic [3:0] gradient_y;
    logic [3:0] gradient_x;

    // Sixteen color levels across each visible axis.
    assign gradient_y = render_line / 10'd30;
    assign gradient_x = render_x / 10'd40;

    assign pixel_in = {
        gradient_y,          // red increases downward
        gradient_x,          // green increases to the right
        4'hF - gradient_y    // blue decreases downward
    };
endmodule
