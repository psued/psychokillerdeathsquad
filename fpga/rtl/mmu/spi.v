`timescale 1ns / 1ps

(* DONT_TOUCH = "yes" *)
module spi(
    input rst,
    
    input SCLK,
    input DIN,
    output reg DOUT,
    input CS,
    
    input [7:0] data_send,
    output reg [7:0] data_recv,
    output reg data_sent,
    output reg data_received
    );
    
    reg [2:0] data_recv_sel;
    reg [2:0] data_send_sel;
    
    always @ (posedge rst or posedge SCLK) begin
        if (rst) begin
            data_recv <= 0;
            data_recv_sel <= 3'b111;
            data_received <= 0;
        end else begin
            if (!CS) begin
                if (data_recv_sel == 3'b111)
                    data_recv[6:0] <= 0;
                data_recv[data_recv_sel] <= DIN;
                data_recv_sel <= data_recv_sel - 1;
                data_received <= data_recv_sel == 0;
            end
        end
    end
    
    always @ (posedge rst or negedge SCLK) begin
        if (rst) begin
            DOUT <= 0;
            data_send_sel <= 3'b111;
            data_sent <= 0;
        end else begin
            if (!CS) begin
                DOUT <= data_send[data_recv_sel];
                data_send_sel <= data_send_sel - 1;
                data_sent <= data_send_sel == 0;
            end
        end
    end
endmodule
