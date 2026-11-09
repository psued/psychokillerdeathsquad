`timescale 1ns / 1ps

module spi_sync(
    input clk,
    input rst,

    input SCLK,
    input DIN,
    output DOUT,
    input CS,

    output [7:0] data_recv,
    output data_received
    );

    wire data_received_async;
    (* ASYNC_REG = "TRUE" *) reg [1:0] data_received_shift;
    assign data_received = data_received_shift[1];

    spi spi_async(
        .rst(rst),

        .SCLK(SCLK),
        .DIN(DIN),
        .DOUT(DOUT),
        .CS(CS),

        .data_send(0),
        .data_recv(data_recv),
        .data_received(data_received_async)
    );

    always @ (posedge rst, posedge clk) begin
        if (rst)
            data_received_shift <= 0;
        else
            data_received_shift <= {data_received_shift[0], data_received_async};
    end
endmodule
