`timescale 1ns / 1ps

module gamestate(
    input clk,
    input rst,

    // begin: for renderer
    input [4:0] layer,
    output [15:0] width,
    output [15:0] height,
    output [15:0] src_x,
    output [15:0] src_y,
    output [15:0] dst_x,
    output [15:0] dst_y,
    output [7:0] z,
    output enabled,
    output flipped,
    output [7:0] bits_per_color,

    input [7:0] addr, // relative to layer
    output [23:0] data,

    // input rendering,
    // end: for renderer

    output SD_SCLK,
    output SD_DI,
    input SD_DO,
    output SD_CS,

    input MCU_SCLK,
    output MCU_MISO,
    input MCU_MOSI,
    input MCU_CS,
    output MCU_READY
    );

    reg selected_mem; // selected for rendering
    wire selected0 = selected_mem == 0;
    wire selected1 = selected_mem == 1;


    wire fs_init_done;
    wire [31:0] file_addr;
    wire fs_read;
    reg [15:0] file_name;
    reg fetch_file;
    wire [31:0] file_size;
    wire file_addr_fetched;
    reg fs_reading;

    wire sd_init_done;
    wire [31:0] sd_size = fs_reading ? 0 : (file_size - 1) >> 9;
    reg sd_read;
    wire sd_read_signal = fs_reading ? fs_read : sd_read;
    wire sd_reading;
    wire [7:0] sd_data;
    wire sd_byte_ready;
    reg sd_byte_ready_prev;
    wire pos_sd_byte_ready = sd_byte_ready && !sd_byte_ready_prev;

    fat32 fs(
        .clk(clk),
        .rst(rst),

        .device_ready(sd_init_done),
        .init_done(fs_init_done),

        .addr(file_addr),
        .read(fs_read),
        .reading(sd_reading),
        .byte_ready(sd_byte_ready),
        .data(sd_data),

        .file_name(file_name),
        .fetch_addr(fetch_file),
        .file_size(file_size),
        .addr_fetched(file_addr_fetched)
        // TODO: deal with addr_fetch_failed
    );

    sdcard sd(
        .clk(clk),
        .rst(rst),

        .SCLK(SD_SCLK),
        .DI(SD_DI),
        .DO(SD_DO),
        .CS(SD_CS),

        // TODO: deal with sd_error
        // .error(sd_error),
        .init_done(sd_init_done),

        .addr(file_addr),
        .size(sd_size),
        .read(sd_read_signal),
        .reading(sd_reading),
        .byte_ready(sd_byte_ready),
        .data(sd_data)
    );

    wire [7:0] mcu_data;
    wire mcu_byte_done;
    reg mcu_byte_done_prev;
    wire pos_mcu_byte_done = mcu_byte_done && !mcu_byte_done_prev;

    spi_sync mcu(
        .clk(clk),
        .rst(rst),

        .SCLK(MCU_SCLK),
        .DOUT(MCU_MISO),
        .DIN(MCU_MOSI),
        .CS(MCU_CS),

        .data_recv(mcu_data),
        .data_received(mcu_byte_done)
    );

    reg [4:0] layer0_sel;
    wire [4:0] layer0 = selected0 ? layer : layer0_sel;
    wire [15:0] width0;
    wire [15:0] height0;
    wire [15:0] src_x0;
    wire [15:0] src_y0;
    wire [15:0] dst_x0;
    wire [15:0] dst_y0;
    wire [7:0] z0;
    wire enabled0;
    wire flipped0;
    wire [7:0] bits_per_color0;

    wire [7:0] addr0 = selected0 ? addr : 0;
    wire [23:0] data0;

    wire [15:0] file_name0;
    wire [15:0] file_name_prev0;

    reg write_en0;
    reg [7:0] write_addr0;
    reg [23:0] write_data0;
    reg write_mcu0;
    reg set_color_bits0;

    memory memory0(
        .clk(clk),
        .rst(rst),

        .layer(layer0),
        .width(width0),
        .height(height0),
        .src_x(src_x0),
        .src_y(src_y0),
        .dst_x(dst_x0),
        .dst_y(dst_y0),
        .z(z0),
        .enabled(enabled0),
        .flipped(flipped0),
        .bits_per_color(bits_per_color0),

        .addr(addr0),
        .data(data0),

        .file_name(file_name0),
        .file_name_prev(file_name_prev0),

        .write_en(write_en0),
        .write_addr(write_addr0),
        .write_data(write_data0),
        .write_mcu(write_mcu0),
        .layer_write(layer0_sel),
        .mcu_data(mcu_data),
        .mcu_byte_done(mcu_byte_done),
        .set_color_bits(set_color_bits0)
    );

    assign width = width0;
    assign height = height0;
    assign src_x = src_x0;
    assign src_y = src_y0;
    assign dst_x = dst_x0;
    assign dst_y = dst_y0;
    assign z = z0;
    assign enabled = enabled0;
    assign flipped = flipped0;
    assign bits_per_color = bits_per_color0;
    assign data = data0;


    reg READY;
    assign MCU_READY = READY && fs_init_done;


    reg [8:0] mcu_byte_counter;
    wire [3:0] mcu_byte_counter_mod = mcu_byte_counter[3:0];
    reg populate_memory;
    reg reading_file;
    reg [31:0] sd_byte_counter;
    reg [1:0] sd_byte_counter_mod;

    always @ (posedge rst, posedge clk) begin
        if (rst) begin
            READY <= 1;
            selected_mem <= 1;

            file_name <= 0;
            fetch_file <= 0;
            fs_reading <= 1;

            sd_read <= 0;
            sd_byte_ready_prev <= 0;
            mcu_byte_done_prev <= 0;

            layer0_sel <= 0;
            write_en0 <= 0;
            write_addr0 <= 0;
            write_data0 <= 0;
            write_mcu0 <= 1;
            set_color_bits0 <= 0;

            mcu_byte_counter <= 0;
            populate_memory <= 0;
            reading_file <= 0;
            sd_byte_counter <= 0;
            sd_byte_counter_mod <= 0;
        end else begin
            mcu_byte_done_prev <= mcu_byte_done;
            sd_byte_ready_prev <= sd_byte_ready;
            if (sd_reading)
                sd_read <= 0;
            if (write_en0)
                write_en0 <= 0;

            if (fetch_file)
                fetch_file <= 0;

            if (file_addr_fetched && reading_file && !fetch_file && !sd_reading) begin
                fs_reading <= 0;
                sd_read <= 1;
            end

            if (pos_mcu_byte_done) begin
                mcu_byte_counter <= mcu_byte_counter + 1;

                if (mcu_byte_counter == 0)
                    READY <= 0;

                if (mcu_byte_counter_mod == 15) begin
                    if (write_mcu0)
                        layer0_sel <= layer0_sel + 1;
                end

                if (mcu_byte_counter == 383) begin
                    write_mcu0 <= 0;
                end else if (mcu_byte_counter == 385) begin
                    mcu_byte_counter <= 0;
                    layer0_sel <= 0;
                    populate_memory <= 1;
                end
            end

            if (populate_memory && !reading_file && !sd_reading) begin
                if (layer0_sel == 24) begin
                    layer0_sel <= 0;
                    populate_memory <= 0;

                    // TODO: change this to only set when memory changes
                    READY <= 1;
                end else begin
                    if (file_name0 == file_name_prev0 || file_name0 != 'h3030) begin
                        layer0_sel <= layer0_sel + 1;
                    end else begin
                        sd_byte_counter <= 0;
                        write_addr0 <= 0;
                        reading_file <= 1;

                        file_name <= file_name0;
                        fetch_file <= 1;
                        fs_reading <= 1;
                    end
                end
            end

            if (pos_sd_byte_ready && !fs_reading && sd_byte_counter < file_size) begin
                sd_byte_counter <= sd_byte_counter + 1;
                if (sd_byte_counter > 4) begin
                    if (sd_byte_counter_mod == 0) begin
                        write_data0[23:16] <= sd_data;
                        if (sd_byte_counter != file_size - 1)
                            sd_byte_counter_mod <= 1;
                        if (sd_byte_counter != 5) // first byte written to ram is offset 5
                            write_addr0 <= write_addr0 + 1;
                    end else if (sd_byte_counter_mod == 1) begin
                        write_data0[15:8] <= sd_data;
                        if (sd_byte_counter != file_size - 1)
                            sd_byte_counter_mod <= 2;
                    end else if (sd_byte_counter_mod == 2) begin
                        write_data0[7:0] <= sd_data;
                        if (sd_byte_counter != file_size - 1) begin
                            sd_byte_counter_mod <= 0;
                            write_en0 <= 1;
                        end
                    end

                    if (sd_byte_counter == file_size - 1) begin
                        if (sd_byte_counter_mod == 0) begin
                            write_data0[15:0] <= 0;
                        end else if (sd_byte_counter_mod == 1) begin
                            write_data0[7:0] <= 0;
                        end
                        write_en0 <= 1;
                        sd_byte_counter_mod <= 0;
                        layer0_sel <= layer0_sel + 1;
                        reading_file <= 0;
                    end
                end else if (sd_byte_counter == 4) begin
                    set_color_bits0 <= sd_data;
                end
            end
        end
    end
endmodule
