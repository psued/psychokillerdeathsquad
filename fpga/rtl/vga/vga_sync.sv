module vga_sync(
    input logic clk,
    input logic reset,
    output logic hsync,
    output logic vsync,
    output logic video_on,
    output logic [9:0] x,
    output logic [9:0] y
    );
    // 640x480 VGA timing parameters

    // Horizontal timing
    localparam int H_DISPLAY = 640;
    localparam int H_FRONT   = 16;
    localparam int H_SYNC    = 96;
    localparam int H_BACK    = 48;

    localparam int H_MAX        = H_DISPLAY + H_FRONT + H_SYNC + H_BACK - 1;
    localparam int H_SYNC_START = H_DISPLAY + H_FRONT;
    localparam int H_SYNC_END   = H_DISPLAY + H_FRONT + H_SYNC - 1;

    // Vertical timing
    localparam int V_DISPLAY = 480;
    localparam int V_FRONT   = 10;
    localparam int V_SYNC    = 2;
    localparam int V_BACK    = 33;

    localparam int V_MAX        = V_DISPLAY + V_FRONT + V_SYNC + V_BACK - 1;
    localparam int V_SYNC_START = V_DISPLAY + V_FRONT;
    localparam int V_SYNC_END   = V_DISPLAY + V_FRONT + V_SYNC - 1;

    // Horizontal and vertical VGA scan counters
    logic [9:0] h_count_reg;
    logic [9:0] v_count_reg;

    always_ff @(posedge clk) begin
        if (reset)
        begin
            h_count_reg <= 0;
            v_count_reg <= 0;
        end
        else
        begin
            if (h_count_reg == H_MAX)
            begin
                h_count_reg <= 0;

                if (v_count_reg == V_MAX)
                    v_count_reg <= 0;
                else
                    v_count_reg <= v_count_reg + 1'b1;
            end
            else
            begin
                h_count_reg <= h_count_reg + 1'b1;
            end
        end
    end

    //VSYNC / HSYNC signal logic
    assign hsync = ~(
        (h_count_reg >= H_SYNC_START) &&
        (h_count_reg <= H_SYNC_END)
    );

    assign vsync = ~(
        (v_count_reg >= V_SYNC_START) &&
        (v_count_reg <= V_SYNC_END)
    );

    assign video_on =
        (h_count_reg < H_DISPLAY) &&
        (v_count_reg < V_DISPLAY);

    assign x = h_count_reg;
    assign y = v_count_reg;

endmodule
