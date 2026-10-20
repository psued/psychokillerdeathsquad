module line_buffer_reader(
    input logic clk,
    input logic reset,

    input logic [11:0] pixel_out,
    input logic buffer_ready,

    input logic hsync_raw,
    input logic vsync_raw,
    input logic video_on_raw,

    output logic       read_enable,
    output logic       hsync,
    output logic       vsync,
    output logic [3:0] red,
    output logic [3:0] green,
    output logic [3:0] blue
    );

    assign read_enable = video_on_raw && !reset && buffer_ready;

    logic video_on;
    logic read_enable_reg;

    always_ff @(posedge clk)
    begin
        hsync <= hsync_raw;
        vsync <= vsync_raw;
        video_on <= video_on_raw;
        read_enable_reg <= read_enable;
    end

    logic [11:0] rgb;

    assign rgb = read_enable_reg ? pixel_out : 12'd0;

    assign red   = rgb[11:8];
    assign green = rgb[7:4];
    assign blue  = rgb[3:0];

endmodule
