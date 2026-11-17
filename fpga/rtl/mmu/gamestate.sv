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
    output [15:0] asset_width,

    input [15:0] addr, // relative to layer
    output [23:0] data,

    input frame_rendered,
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

    reg selected_mem; // selected for writing


    wire fs_init_done;
    wire [31:0] file_addr;
    wire fs_read;
    reg [15:0] file_name;
    reg fetch_file;
    wire [31:0] file_size;
    wire file_addr_fetched;
    (* MARK_DEBUG = "TRUE" *) reg fs_reading;

    wire sd_init_done;
    wire sd_error;
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

        .error(sd_error),
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

    reg [4:0] write_layer_sel;

    // mem_size only depends on layer, not instance.
    // Since both memories are controlled by write_layer_sel,
    // only the mem_size from memory0 is used.
    wire [15:0] mem_size;


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
    wire [15:0] asset_width0;

    wire [23:0] data0;

    wire [15:0] file_name0;
    wire [15:0] file_name_prev0;
    wire enabled_fetch0;

    reg write_en0;
    reg [15:0] write_addr0;
    reg [23:0] write_data0;
    wire [23:0] read_write_data0;
    reg write_mcu0;
    reg write_metadata0;
    reg [7:0] set_color_bits0;
    reg [15:0] set_asset_width0;

    memory memory0(
        .clk(clk),
        .rst(rst),

        .layer(layer),
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
        .asset_width(asset_width0),

        .addr(addr),
        .data(data0),

        .file_name(file_name0),
        .file_name_prev(file_name_prev0),
        .mem_size(mem_size),
        .enabled_fetch(enabled_fetch0),

        .write_en(write_en0),
        .write_addr(write_addr0),
        .write_data(write_data0),
        .read_write_data(read_write_data0),
        .write_mcu(write_mcu0),
        .layer_write(write_layer_sel),
        .mcu_data(mcu_data),
        .mcu_byte_done(mcu_byte_done),
        .write_metadata(write_metadata0),
        .set_color_bits(set_color_bits0),
        .set_asset_width(set_asset_width0)
    );

    wire [15:0] width1;
    wire [15:0] height1;
    wire [15:0] src_x1;
    wire [15:0] src_y1;
    wire [15:0] dst_x1;
    wire [15:0] dst_y1;
    wire [7:0] z1;
    wire enabled1;
    wire flipped1;
    wire [7:0] bits_per_color1;
    wire [15:0] asset_width1;

    wire [23:0] data1;

    wire [15:0] file_name1;
    wire [15:0] file_name_prev1;
    wire enabled_fetch1;

    reg write_en1;
    reg [15:0] write_addr1;
    reg [23:0] write_data1;
    wire [23:0] read_write_data1;
    reg write_mcu1;
    reg write_metadata1;
    reg [7:0] set_color_bits1;
    reg [15:0] set_asset_width1;

    // Entry 0 is shared background; 1..6 and 7..12 cover the two banks.
    localparam int CACHE_ENTRIES = 13;
    reg [CACHE_ENTRIES-1:0] cache_valid;
    reg [15:0] cache_image_id [0:CACHE_ENTRIES-1];
    reg [15:0] cache_asset_width [0:CACHE_ENTRIES-1];
    reg [7:0] cache_color_bits [0:CACHE_ENTRIES-1];
    reg [3:0] loading_cache_slot;
    reg cache_metadata_pending;

    function automatic [3:0] cache_slot_for_layer(
        input [4:0] slot,
        input bank
    );
        reg [3:0] region;
        begin
            case (slot)
                0: region = 0;
                1: region = 1;
                2: region = 2;
                3: region = 3;
                default: begin
                    if (slot < 12)
                        region = 4;
                    else if (slot < 16)
                        region = 5;
                    else
                        region = 6;
                end
            endcase
            cache_slot_for_layer = (region == 0) ? 4'd0
                                 : region + (bank ? 4'd6 : 4'd0);
        end
    endfunction

    wire [3:0] cache_slot = cache_slot_for_layer(write_layer_sel, selected_mem);
    wire [15:0] requested_image_id = selected_mem ? file_name1 : file_name0;
    wire requested_enabled = selected_mem ? enabled_fetch1 : enabled_fetch0;
    wire cache_hit = cache_valid[cache_slot]
                  && cache_image_id[cache_slot] == requested_image_id;

    memory memory1(
        .clk(clk),
        .rst(rst),

        .layer(layer),
        .width(width1),
        .height(height1),
        .src_x(src_x1),
        .src_y(src_y1),
        .dst_x(dst_x1),
        .dst_y(dst_y1),
        .z(z1),
        .enabled(enabled1),
        .flipped(flipped1),
        .bits_per_color(bits_per_color1),
        .asset_width(asset_width1),

        .addr(addr),
        .data(data1),

        .file_name(file_name1),
        .file_name_prev(file_name_prev1),
        .enabled_fetch(enabled_fetch1),

        .write_en(write_en1),
        .write_addr(write_addr1),
        .write_data(write_data1),
        .read_write_data(read_write_data1),
        .write_mcu(write_mcu1),
        .layer_write(write_layer_sel),
        .mcu_data(mcu_data),
        .mcu_byte_done(mcu_byte_done),
        .write_metadata(write_metadata1),
        .set_color_bits(set_color_bits1),
        .set_asset_width(set_asset_width1)
    );

    reg bg_write_en;
    wire [15:0] bg_write_addr = selected_mem ? write_addr1 : write_addr0;
    wire [23:0] bg_write_data = selected_mem ? write_data1 : write_data0;
    wire [23:0] bg_data;

    ram #(.ADDR_SIZE(15), .RAM_SIZE(24050)) bg_layer (
        .clk(clk),
        .write_en(bg_write_en),
        .read_addr(addr),
        .write_addr(bg_write_addr),

        .data_in(bg_write_data),
        .data_out(bg_data)
    );

    // selected_mem is for writing, so it's reversed here
    assign width = selected_mem ? width0 : width1;
    assign height = selected_mem ? height0 : height1;
    assign src_x = selected_mem ? src_x0 : src_x1;
    assign src_y = selected_mem ? src_y0 : src_y1;
    assign dst_x = selected_mem ? dst_x0 : dst_x1;
    assign dst_y = selected_mem ? dst_y0 : dst_y1;
    assign z = selected_mem ? z0 : z1;
    assign enabled = selected_mem ? enabled0 : enabled1;
    assign flipped = selected_mem ? flipped0 : flipped1;
    assign bits_per_color = selected_mem ? bits_per_color0 : bits_per_color1;
    assign asset_width = selected_mem ? asset_width0 : asset_width1;
    assign data = layer == 0 ? bg_data : (selected_mem ? data0 : data1);


    reg READY;
    assign MCU_READY = READY && fs_init_done;


    reg [8:0] mcu_byte_counter;
    wire [3:0] mcu_byte_counter_mod = mcu_byte_counter[3:0];
    reg populate_memory;
    reg reading_file;
    reg [31:0] sd_byte_counter;
    reg [1:0] sd_byte_counter_mod;
    reg read_mem;
    reg [23:0] addr_end;
    reg ram_wait;
    reg state_written;

    always @ (posedge rst, posedge clk) begin
        if (rst) begin
            READY <= 1;
            selected_mem <= 0;

            file_name <= 0;
            fetch_file <= 0;
            fs_reading <= 1;

            sd_read <= 0;
            sd_byte_ready_prev <= 0;
            mcu_byte_done_prev <= 0;

            write_layer_sel <= 0;

            write_en0 <= 0;
            write_addr0 <= 0;
            write_data0 <= 0;
            write_mcu0 <= 1;
            write_metadata0 <= 0;
            set_color_bits0 <= 0;
            set_asset_width0 <= 0;

            write_en1 <= 0;
            write_addr1 <= 0;
            write_data1 <= 0;
            write_mcu1 <= 0;
            write_metadata1 <= 0;
            set_color_bits1 <= 0;
            set_asset_width1 <= 0;

            bg_write_en <= 0;

            mcu_byte_counter <= 0;
            populate_memory <= 0;
            reading_file <= 0;
            sd_byte_counter <= 0;
            sd_byte_counter_mod <= 0;
            read_mem <= 0;
            addr_end <= 0;
            ram_wait <= 0;
            state_written <= 0;
            cache_valid <= '0;
            loading_cache_slot <= 0;
            cache_metadata_pending <= 0;
        end else begin
            mcu_byte_done_prev <= mcu_byte_done;
            sd_byte_ready_prev <= sd_byte_ready;
            if (sd_reading)
                sd_read <= 0;
            if (write_en0 && !read_mem)
                write_en0 <= 0;
            if (write_en1 && !read_mem)
                write_en1 <= 0;
            if (bg_write_en)
                bg_write_en <= 0;
            if (write_metadata0)
                write_metadata0 <= 0;
            if (write_metadata1)
                write_metadata1 <= 0;

            if (fetch_file)
                fetch_file <= 0;

            if (file_addr_fetched && reading_file && fs_reading
                    && !fetch_file && !sd_reading) begin
                fs_reading <= 0;
                sd_read <= 1;
            end

            if (state_written && frame_rendered) begin
                READY <= 1;
                selected_mem <= !selected_mem;
                state_written <= 0;

                // inverted since the control signal hasn't changed yet
                if (selected_mem)
                    write_mcu0 <= 1;
                else
                    write_mcu1 <= 1;
            end

            if (pos_mcu_byte_done) begin
                mcu_byte_counter <= mcu_byte_counter + 1;

                if (mcu_byte_counter == 0)
                    READY <= 0;

                if (mcu_byte_counter_mod == 15) begin
                    if (write_mcu0 || write_mcu1)
                        write_layer_sel <= write_layer_sel + 1;
                end

                if (mcu_byte_counter == 383) begin
                    write_mcu0 <= 0;
                    write_mcu1 <= 0;
                end else if (mcu_byte_counter == 385) begin
                    mcu_byte_counter <= 0;
                    write_layer_sel <= 0;
                    populate_memory <= 1;
                end
            end

            if (populate_memory && !reading_file && !sd_reading) begin
                if (cache_metadata_pending) begin
                    // The memory module writes metadata using the current layer index.
                    cache_metadata_pending <= 0;
                    write_layer_sel <= write_layer_sel + 1;
                end else if (write_layer_sel == 24) begin
                    write_layer_sel <= 0;
                    populate_memory <= 0;
                    state_written <= 1;
                end else if (!requested_enabled) begin
                    write_layer_sel <= write_layer_sel + 1;
                end else if (cache_hit) begin
                    if (selected_mem) begin
                        set_asset_width1 <= cache_asset_width[cache_slot];
                        set_color_bits1 <= cache_color_bits[cache_slot];
                        write_metadata1 <= 1;
                    end else begin
                        set_asset_width0 <= cache_asset_width[cache_slot];
                        set_color_bits0 <= cache_color_bits[cache_slot];
                        write_metadata0 <= 1;
                    end
                    cache_metadata_pending <= 1;
                end else begin
                    cache_valid[cache_slot] <= 0;
                    loading_cache_slot <= cache_slot;
                    sd_byte_counter <= 0;
                    write_addr0 <= 0;
                    write_addr1 <= 0;
                    reading_file <= 1;
                    file_name <= requested_image_id;
                    fetch_file <= 1;
                    fs_reading <= 1;
                end
            end

            if (read_mem) begin
                if ((!selected_mem && write_addr0 == addr_end)
                        || (selected_mem && write_addr1 == addr_end)) begin
                    write_layer_sel <= write_layer_sel + 1;
                    reading_file <= 0;
                    read_mem <= 0;
                end else begin
                    if (selected_mem)
                        write_data1 <= read_write_data0;
                    else
                        write_data0 <= read_write_data1;

                    if (ram_wait) begin
                        ram_wait <= 0;

                        // put the address read from ahead to account for synchronous ram
                        if (selected_mem)
                            write_addr0 <= write_addr0 + 1;
                        else
                            write_addr1 <= write_addr1 + 1;
                    end else begin
                        write_addr0 <= write_addr0 + 1;
                        write_addr1 <= write_addr1 + 1;
                    end
                end
            end

            if (reading_file && !fs_reading && sd_byte_counter == file_size
                    && !sd_read && !sd_reading && !sd_error
                    && !write_en0 && !write_en1 && !bg_write_en) begin
                if (file_size >= 7) begin
                    cache_image_id[loading_cache_slot] <= file_name;
                    cache_asset_width[loading_cache_slot] <=
                        selected_mem ? set_asset_width1 : set_asset_width0;
                    cache_color_bits[loading_cache_slot] <=
                        selected_mem ? set_color_bits1 : set_color_bits0;
                    cache_valid[loading_cache_slot] <= 1;
                end
                write_layer_sel <= write_layer_sel + 1;
                reading_file <= 0;
            end

            if (pos_sd_byte_ready && !fs_reading && sd_byte_counter < file_size) begin
                sd_byte_counter <= sd_byte_counter + 1;
                if (sd_byte_counter > 4) begin
                    if (sd_byte_counter_mod == 0) begin
                        if (selected_mem)
                            write_data1[23:16] <= sd_data;
                        else
                            write_data0[23:16] <= sd_data;
                        if (sd_byte_counter != file_size - 1)
                            sd_byte_counter_mod <= 1;
                        if (sd_byte_counter != 5) begin // first byte written to ram is offset 5
                            if (selected_mem)
                                write_addr1 <= write_addr1 + 1;
                            else
                                write_addr0 <= write_addr0 + 1;
                        end
                    end else if (sd_byte_counter_mod == 1) begin
                        if (selected_mem)
                            write_data1[15:8] <= sd_data;
                        else
                            write_data0[15:8] <= sd_data;
                        if (sd_byte_counter != file_size - 1)
                            sd_byte_counter_mod <= 2;
                    end else if (sd_byte_counter_mod == 2) begin
                        if (selected_mem)
                            write_data1[7:0] <= sd_data;
                        else
                            write_data0[7:0] <= sd_data;
                        if (sd_byte_counter != file_size - 1) begin
                            sd_byte_counter_mod <= 0;
                            if (write_layer_sel == 0)
                                bg_write_en <= 1;
                            else if (selected_mem)
                                write_en1 <= 1;
                            else
                                write_en0 <= 1;
                        end
                    end

                    if (sd_byte_counter == file_size - 1) begin
                        if (sd_byte_counter_mod == 0) begin
                            if (selected_mem)
                                write_data1[15:0] <= 0;
                            else
                                write_data0[15:0] <= 0;
                        end else if (sd_byte_counter_mod == 1) begin
                            if (selected_mem)
                                write_data1[7:0] <= 0;
                            else
                                write_data0[7:0] <= 0;
                        end
                        if (write_layer_sel == 0)
                            bg_write_en <= 1;
                        else if (selected_mem)
                            write_en1 <= 1;
                        else
                            write_en0 <= 1;
                        sd_byte_counter_mod <= 0;
                    end
                end else if (sd_byte_counter == 4) begin
                    if (selected_mem) begin
                        set_color_bits1 <= sd_data;
                        write_metadata1 <= 1;
                    end else begin
                        set_color_bits0 <= sd_data;
                        write_metadata0 <= 1;
                    end
                end else if (sd_byte_counter == 1) begin
                    if (selected_mem)
                        set_asset_width1[15:8] <= sd_data;
                    else
                        set_asset_width0[15:8] <= sd_data;
                end else if (sd_byte_counter == 0) begin
                    if (selected_mem)
                        set_asset_width1[7:0] <= sd_data;
                    else
                        set_asset_width0[7:0] <= sd_data;
                end
            end
        end
    end
endmodule
