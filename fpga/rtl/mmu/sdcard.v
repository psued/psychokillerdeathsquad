`timescale 1ns / 1ps

(* DONT_TOUCH = "yes" *)
module sdcard(
    input clk,
    input rst,
    
    output SCLK,
    output DI,
    input DO,
    output reg CS,
    
    input CD,
    
    output reg error,
    output reg init_done,
    
    input [31:0] addr,
    input [15:0] size, // sectors - 1
    input read,
    output reg reading,
    output byte_ready,
    output [7:0] data
    );
    
    reg [7:0] data_send;
    wire [7:0] data_recv;
    wire data_sent;
    wire data_received;
    
    reg reading_data_block;
    reg reading_data_block_start;
    reg [8:0] byte_count;
    
    assign data = data_recv;
    assign byte_ready = data_received && reading_data_block;
    
    spi spi0(
        .rst(rst),
        
        .SCLK(SCLK),
        .DOUT(DI),
        .DIN(DO),
        .CS(CS),
        
        .data_send(data_send),
        .data_recv(data_recv),
        .data_sent(data_sent),
        .data_received(data_received)
        );
    
    reg [16:0] init_wait_counter;
    
    reg SCLK_en;
    reg [8:0] clk_div;
    reg SCLK_prev;
    wire posSCLK = SCLK && !SCLK_prev;
    wire negSCLK = !SCLK && SCLK_prev;
    
    assign SCLK = SCLK_en ? (!init_done ? clk_div[8] : clk_div[2]) : 0;
    
    reg [47:0] send_queue;
    
    reg cmd_ready;
    reg awaiting_resp;
    reg resp_ext;
    
    reg [6:0] resp_status;
    reg resp_ext_stage;
    reg [1:0] resp_ext_counter;
    reg [31:0] resp_reg;
    
    reg [2:0] init_state;
    reg [6:0] dummy_counter;
    
    reg [15:0] multiread_counter;
    
    task send_cmd;
        input [5:0] cmd;
        input [31:0] addr;
        input [6:0] crc;
    begin
        send_queue[47:46] <= 2'b01;
        send_queue[45:40] <= cmd;
        
        send_queue[39:8] <= addr;
        
        send_queue[7:1] <= crc;
        send_queue[0]   <= 1;
        
        cmd_ready <= 0;
        awaiting_resp <= 1;
        if (cmd == 8 || cmd == 58)
            resp_ext <= 1;
        else
            resp_ext <= 0;
    end
    endtask
    
    // based on https://elm-chan.org/docs/mmc/rc/sdinit.png
    task initialize;
    begin
        case (init_state)
            0: begin // wait
                init_wait_counter <= init_wait_counter + 1;
                if (init_wait_counter == 0) begin
                    SCLK_en <= 1;
                    clk_div <= 0;
                    init_state <= 1;
                end
            end
            1: begin // dummy clock
                if (posSCLK)
                    dummy_counter <= dummy_counter + 1;
                if (dummy_counter == 80) begin
                    init_state <= 2;
                    cmd_ready <= 1;
                end
            end
            2: begin // CMD0: reset + spi mode
                if (cmd_ready) begin
                    CS <= 0;
                    send_cmd(0, 0, 7'b1001010);
                end else begin
                    cmd_ready <= 1;
                    if (resp_status == 1)
                        init_state <= 3;
                    else
                        error <= 1;
                end
            end
            3: begin // CMD8: check voltage
                if (cmd_ready) begin
                    send_cmd(8, 'h000001AA, 7'b1000011);
                end else begin
                    cmd_ready <= 1;
                    if (resp_status == 1 && resp_reg[11:0] == 12'h1AA)
                        init_state <= 4;
                    else
                        error <= 1;
                    // (or SD v1 if illegal command?)
                end
            end
            4: begin // ACMD41 - CMD55: start init
                if (cmd_ready) begin
                    send_cmd(55, 0, 0);
                end else begin
                    cmd_ready <= 1;
                    init_state <= 5;
                end
            end
            5: begin // ACMD41 - CMD41: init + check finished
                if (cmd_ready) begin
                    send_cmd(41, 'h40000000, 0);
                end else begin
                    cmd_ready <= 1;
                    if (resp_status == 0)
                        init_state <= 6;
                    else if (resp_status == 1)
                        init_state <= 4;
                    else
                        error <= 1;
                end
            end
            6: begin // CMD58: read OCR
                if (cmd_ready) begin
                    send_cmd(58, 0, 0);
                end else begin
                    cmd_ready <= 1;
                    if (resp_status == 0) begin
                        if (resp_reg[30] == 1)
                            init_done <= 1;
                        else
                            init_state <= 7;
                    end else begin
                        error <= 1;
                    end
                end
            end
            7: begin // CMD16: set block size
                if (cmd_ready) begin
                    send_cmd(16, 'h00000200, 0);
                end else begin
                    cmd_ready <= 1;
                    if (resp_status == 0)
                        init_done <= 1;
                    else
                        error <= 1;
                end
            end
        endcase
    end
    endtask
    
    always @ (posedge rst or posedge clk) begin
        if (rst) begin
            error <= 0;
            CS <= 1;
            data_send <= 'hFF;
        
            init_done <= 0;
            init_wait_counter <= 1;
            
            SCLK_en <= 0;
            clk_div <= 0;
            SCLK_prev <= 0;
            
            send_queue <= 'hFFFFFFFFFFFF;
            
            cmd_ready <= 0;
            awaiting_resp <= 0;
            resp_ext <= 0;
            
            resp_status <= 0;
            resp_ext_stage <= 0;
            resp_ext_counter <= 0;
            resp_reg <= 0;
            
            init_state <= 0;
            dummy_counter <= 0;
            
            reading <= 0;
            reading_data_block <= 0;
            reading_data_block_start <= 0;
            byte_count <= 0;
            
            multiread_counter <= 0;
        end else begin
            clk_div <= clk_div + 1;
            SCLK_prev <= SCLK;
        
            if (!CD) begin end // TODO: handle no inserted card, currently assuming card present
        
            if (posSCLK && reading_data_block_start) begin
                reading_data_block_start <= 0;
                reading_data_block <= 1;
            end
        
            if (posSCLK && data_sent) begin
                data_send <= send_queue[47:40];
                send_queue <= (send_queue << 8) | 8'b11111111;
            end
            
            if (negSCLK && data_received) begin
                if (awaiting_resp && data_recv[7] == 0 && !resp_ext_stage) begin
                    resp_status <= data_recv[6:0];
                    if (resp_ext)
                        resp_ext_stage <= 1;
                    else begin
                        awaiting_resp <= 0;
                    end
                    
                    if (size != 0 && multiread_counter == size) begin // CMD12 only
                        cmd_ready <= 1;
                        reading <= 0;
                    end
                end
                
                if (resp_ext && resp_ext_stage) begin
                    resp_reg <= resp_reg << 8;
                    resp_reg[7:0] <= data_recv;
                    resp_ext_counter <= resp_ext_counter + 1;
                    
                    if (resp_ext_counter == 3) begin
                        awaiting_resp <= 0;
                        resp_ext <= 0;
                        resp_ext_stage <= 0;
                    end
                end
                
                if (reading && !awaiting_resp && data_recv == 8'b11111110) begin // data token received
                    reading_data_block_start <= 1;
                end
                
                if (reading && reading_data_block) begin
                    byte_count <= byte_count + 1;
                    if (byte_count == 511) begin
                        reading_data_block <= 0;
                        if (size == 0) begin
                            reading <= 0;
                            cmd_ready <= 1;
                        end else if (multiread_counter == size) begin
                            send_cmd(12, 0, 0);
                        end else begin
                            multiread_counter <= multiread_counter + 1;
                        end
                    end
                end
            end
            
            if (!init_done && !error && !awaiting_resp) begin
                initialize();
            end
            
            if (init_done && read && cmd_ready) begin
                // TODO: check status of read command somehow
                if (size == 0)
                    send_cmd(17, addr, 0);
                else begin
                    multiread_counter <= 0;
                    send_cmd(18, addr, 0);
                end
                reading <= 1;
            end
        end
    end
endmodule
