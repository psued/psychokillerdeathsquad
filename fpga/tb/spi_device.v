`timescale 1ns / 1ps

module spi_device;
    reg rst;

    reg SCLK;
    
    wire MISO;
    reg MOSI;
    reg CS;
    
    reg [7:0] data_send;
    wire [7:0] data_recv;
    wire byte_done;
        
    spi device(
        .rst(rst),
        
        .SCLK(SCLK),
        .DOUT(MISO),
        .DIN(MOSI),
        .CS(CS),
        
        .data_send(data_send),
        .data_recv(data_recv),
        .data_received(byte_done)
        );
                
    always @ (negedge CS) begin
        SCLK = 0;
        MOSI = 0;
    end
        
    always #4 begin
        if (!CS)
            SCLK = ~SCLK;
    end
        
    always #8 MOSI <= ~MOSI;
    
    always @ (posedge SCLK) begin
        if (byte_done) begin
            data_send <= 8'b10101010;
        end
    end
        
    always @ (negedge SCLK) begin
        if (byte_done) begin
            MOSI <= 1;
        end
    end
    
    initial begin
        rst <= 0;
        SCLK <= 0;
        MOSI <= 1;
        CS <= 1;
        data_send <= 8'b01010101;
        
        #4 rst <= 1;
        #4 rst <= 0;
        
        #2 CS <= 0;
        #128 CS <= 1;
        #14 CS <= 0;
        #68 CS <= 1;
        
        #2 $finish;
    end
endmodule
