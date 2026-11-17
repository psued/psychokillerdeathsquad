`timescale 1ns / 1ps

(* DONT_TOUCH = "yes" *)
module fat32(
    input clk,
    input rst,
    
    input device_ready,
    output reg init_done,
    
    output reg [31:0] addr,
    output reg read,
    input reading,
    input byte_ready,
    input [7:0] data,
    
    input [15:0] file_name,
    input fetch_addr,
    output reg [31:0] file_size,
    output reg addr_fetched,
    output reg addr_fetch_failed
    );
    
    reg byte_ready_prev;
    wire pos_byte_ready = byte_ready && !byte_ready_prev;
    reg fetch_addr_prev;
    wire pos_fetch_addr = fetch_addr && !fetch_addr_prev;
    
    reg [31:0] part_start;
    reg [7:0] sects_per_clus;
    reg [15:0] reserved_sects;
    reg [7:0] num_fats;
    reg [31:0] fat_size;
    reg [31:0] data_start;
    reg [31:0] clus;
    reg [31:0] clus_idx;
    reg [31:0] clus_sect;
    
    reg mbr_read;
    reg [8:0] counter;
    wire [4:0] counter_mod = counter[4:0];
    reg [7:0] rootdir_sectors_read;
    
    reg fn1_match;
    reg fn2_match;
    wire fn_match = fn1_match && fn2_match;
    reg flag_match;
    wire match = fn_match && flag_match;
    
    always @ (posedge rst or posedge clk) begin
        if (rst) begin
            init_done <= 0;
            addr <= 0;
            read <= 0;
            file_size <= 0;
            addr_fetched <= 0;
            addr_fetch_failed <= 0;
            
            byte_ready_prev <= 0;
            fetch_addr_prev <= 0;
            
            part_start <= 0;
            sects_per_clus <= 0;
            reserved_sects <= 0;
            num_fats <= 0;
            fat_size <= 0;
            data_start <= 0;
            
            mbr_read <= 0;
            counter <= 0;
            rootdir_sectors_read <= 0;
            
            fn1_match <= 0;
            fn2_match <= 0;
            flag_match <= 0;
        end else if (device_ready) begin
            byte_ready_prev <= byte_ready;
            fetch_addr_prev <= fetch_addr;
            if (init_done) begin
                if (pos_fetch_addr && !reading) begin
                    addr_fetched <= 0;
                    addr_fetch_failed <= 0;
                    addr <= data_start;
                    read <= 1;
                    
                    fn1_match <= 0;
                    fn2_match <= 0;
                    flag_match <= 0;
                    
                    rootdir_sectors_read <= 0;
                end
                
                if (pos_byte_ready) begin
                    counter <= counter + 1;
                    if (!addr_fetched) begin
                        if (counter_mod == 0) begin
                            if (data == file_name[15:8])
                                fn1_match <= 1;
                            else
                                fn1_match <= 0;
                        end else if (counter_mod == 1) begin
                            if (data == file_name[7:0])
                                fn2_match <= 1;
                            else
                                fn2_match <= 0;
                        end else if (counter_mod == 11) begin
                            if (data == 8'h0F || data & 8'h10) // long name or dir
                                flag_match <= 0;
                            else
                                flag_match <= 1;
                        end else if (counter_mod == 20) begin
                            clus[23:16] <= data;
                        end else if (counter_mod == 21) begin
                            clus[31:24] <= data;
                        end else if (counter_mod == 26) begin
                            clus[7:0] <= data;
                        end else if (counter_mod == 27) begin
                            clus[15:8] <= data;
                        end else if (counter_mod == 28) begin
                            file_size[7:0] <= data;
                            clus_idx <= clus - 2;
                        end else if (counter_mod == 29) begin
                            file_size[15:8] <= data;
                            clus_sect <= clus_idx * sects_per_clus;
                        end else if (counter_mod == 30) begin
                            file_size[23:16] <= data;
                        end else if (counter_mod == 31) begin
                            file_size[31:24] <= data;
                            
                            if (match) begin
                                addr_fetched <= 1;
                                addr <= data_start + clus_sect;
                                read <= 0;
                            end
                        end
                        
                        if (counter == 511 && !match) begin // addr not fetched and end reached
                            if (rootdir_sectors_read < sects_per_clus) begin
                                addr <= addr + 1;
                                rootdir_sectors_read <= rootdir_sectors_read + 1;
                            end else begin
                                addr_fetch_failed <= 1;
                                read <= 0;
                            end
                        end
                    end
                end
            end else begin
                if (mbr_read) begin
                    if (!reading) begin
                        addr <= part_start;
                        read <= 1;
                    end else begin
                        read <= 0;
                    end
                    
                    if (pos_byte_ready) begin
                        counter <= counter + 1;
                        if (counter == 13)
                            sects_per_clus <= data;
                        else if (counter == 14)
                            reserved_sects[7:0] <= data;
                        else if (counter == 15)
                            reserved_sects[15:8] <= data;
                        else if (counter == 16)
                            num_fats <= data;
                        else if (counter == 36)
                            fat_size[7:0] <= data;
                        else if (counter == 37)
                            fat_size[15:8] <= data;
                        else if (counter == 38)
                            fat_size[23:16] <= data;
                        else if (counter == 39)
                            fat_size[31:24] <= data;
                        else if (counter == 511) begin
                            data_start <= part_start + reserved_sects + (fat_size * num_fats);
                            init_done <= 1;
                        end
                    end
                end else begin
                    if (!reading) begin
                        addr <= 0;
                        read <= 1;
                    end else begin
                        read <= 0;
                    end
                    
                    if (pos_byte_ready) begin
                        counter <= counter + 1;
                        if (counter == 454)
                            part_start[7:0] <= data;
                        else if (counter == 455)
                            part_start[15:8] <= data;
                        else if (counter == 456)
                            part_start[23:16] <= data;
                        else if (counter == 457)
                            part_start[31:24] <= data;
                        else if (counter == 511)
                            mbr_read <= 1;
                    end
                end
            end
        end
    end
endmodule
