module cdc_sync(

    input logic clk,
    input logic [1:0] async_in,
    output logic [1:0] sync_out
    );


    (* ASYNC_REG = "TRUE" *) logic [1:0] sync_ff1;
    (* ASYNC_REG = "TRUE" *) logic [1:0] sync_ff2;

    always_ff @(posedge clk) begin
        sync_ff1 <= async_in;
        sync_ff2 <= sync_ff1;
    end

    assign sync_out = sync_ff2;

endmodule
