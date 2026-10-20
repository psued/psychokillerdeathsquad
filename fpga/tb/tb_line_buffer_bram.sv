

// Test three 640-pixel buffers by writing values and reading them back.
// Address = buffer * 640 + x.
// Buffer address ranges: 0..639, 640..1279, and 1280..1919.
//
// Check the first/last addresses, both buffer boundaries, and one interior
// pixel in each buffer. Each test group writes all its values before reading
// them back, so overlapping locations can show up as overwritten data.
// These are spot checks, not a complete test of every address or BRAM behavior.
module tb_line_buffer_bram;

    // Delays use nanoseconds; timing precision is 1 ps.
    timeunit 1ns;
    timeprecision 1ps;

    // Separate clocks for writing and reading.
    logic wr_clk;
    logic rd_clk;

    // Write port: select a buffer and pixel position, then supply pixel_in.
    // The BRAM stores the value on a rising wr_clk edge when wr_en is high.
    logic [1:0]  write_buffer;
    logic [9:0] write_x;
    logic        wr_en;
    logic [11:0] pixel_in;

    // Read port: select a buffer and pixel position.
    // The BRAM updates pixel_out on a rising rd_clk edge when rd_en is high.
    logic [1:0]  read_buffer;
    logic [9:0] read_x;
    logic        rd_en;
    logic [11:0] pixel_out;

    // 100 MHz write clock: toggle every 5 ns for a 10 ns period.
    initial wr_clk = 1'b0;
    always #5 wr_clk = ~wr_clk;

    // 25 MHz read clock: toggle every 20 ns for a 40 ns period.
    initial rd_clk = 1'b0;
    always #20 rd_clk = ~rd_clk;

    // Set inputs on falling edges; the BRAM reads/writes on rising edges.
    // This gives the inputs time to settle before the BRAM samples them.
    // For reads, the rising edge updates pixel_out, and the next falling
    // edge checks it. This matches the BRAM's single synchronous read stage.
    initial begin
        // Set known input values and disable both ports before testing.
        // This does not clear BRAM memory; only written locations are checked.
        write_buffer = 2'd0;
        write_x = 10'd0;
        wr_en = 1'b0;
        pixel_in = 12'h000;

        read_buffer = 2'd0;
        read_x = 10'd0;
        rd_en = 1'b0;

        // Test 1: First and last memory locations (addresses 0 and 1919).
        // Buffer 0, x=0 is the first pixel; buffer 2, x=639 is the last.
        // Store different values at these endpoints, then check both.
        @(negedge wr_clk);
        write_buffer = 2'd0;
        write_x = 10'd0;
        wr_en = 1'b1;
        pixel_in = 12'hABC;

        @(negedge wr_clk);
        write_buffer = 2'd2;
        write_x = 10'd639;
        pixel_in = 12'hDEF;

        @(negedge wr_clk);

        // Last write is complete. Stop writing and begin readback.
        wr_en = 1'b0;

        @(negedge rd_clk);
        rd_en = 1'b1;

        read_buffer = 2'd0;
        read_x = 10'd0;

        @(negedge rd_clk);

        assert(pixel_out == 12'hABC) else $fatal("Test failed: pixel_out = %h, expected 0xABC", pixel_out);
        $display("PASS: buffer 0, x=0 = %h", pixel_out);

        read_buffer = 2'd2;
        read_x = 10'd639;

        @(negedge rd_clk);

        assert(pixel_out == 12'hDEF) else $fatal("Test failed: pixel_out = %h, expected 0xDEF", pixel_out);
        $display("PASS: buffer 2, x=639 = %h", pixel_out);

        rd_en = 1'b0;

        // Test 2: Last pixel of one buffer and first pixel of the next.
        // Buffer 0, x=639 and buffer 1, x=0 map to addresses 639 and 640.
        // Buffer 1, x=639 and buffer 2, x=0 map to addresses 1279 and 1280.
        // Each pair must remain separate even though the addresses are adjacent.
        // Write four different values before reading back to catch overlap.
        @(negedge wr_clk);
        wr_en = 1'b1;

        write_buffer = 2'd0;
        write_x = 10'd639;
        pixel_in = 12'h123;

        @(negedge wr_clk);

        write_buffer = 2'd1;
        write_x = 10'd0;
        pixel_in = 12'h456;

        @(negedge wr_clk);

        write_buffer = 2'd1;
        write_x = 10'd639;
        pixel_in = 12'h789;

        @(negedge wr_clk);

        write_buffer = 2'd2;
        write_x = 10'd0;
        pixel_in = 12'hABC;

        @(negedge wr_clk);
        // Last write is complete. Stop writing and begin readback.
        wr_en = 1'b0;


        @(negedge rd_clk);
        rd_en = 1'b1;

        read_buffer = 2'd0;
        read_x = 10'd639;

        @(negedge rd_clk);

        assert(pixel_out == 12'h123) else $fatal("Test failed: pixel_out = %h, expected 0x123", pixel_out);
        $display("PASS: buffer 0, x=639 = %h", pixel_out);

        read_buffer = 2'd1;
        read_x = 10'd0;

        @(negedge rd_clk);

        assert(pixel_out == 12'h456) else $fatal("Test failed: pixel_out = %h, expected 0x456", pixel_out);
        $display("PASS: buffer 1, x=0 = %h", pixel_out);

        read_buffer = 2'd1;
        read_x = 10'd639;

        @(negedge rd_clk);
        assert(pixel_out == 12'h789) else $fatal("Test failed: pixel_out = %h, expected 0x789", pixel_out);
        $display("PASS: buffer 1, x=639 = %h", pixel_out);

        read_buffer = 2'd2;
        read_x = 10'd0;

        @(negedge rd_clk);
        assert(pixel_out == 12'hABC) else $fatal("Test failed: pixel_out = %h, expected 0xABC", pixel_out);
        $display("PASS: buffer 2, x=0 = %h", pixel_out);

        rd_en = 1'b0;

        // Test 3: Pixel x=123 in each buffer (addresses 123, 763, 1403).
        // Keep x the same and change only the buffer selection.
        // AAA, BBB, and CCC must all remain stored independently. If the
        // buffer offset is ignored, later writes overwrite earlier values.
        @(negedge wr_clk);
        wr_en = 1'b1;

        write_buffer = 2'd0;
        write_x = 10'd123;
        pixel_in = 12'hAAA;

        @(negedge wr_clk);

        write_buffer = 2'd1;
        write_x = 10'd123;
        pixel_in = 12'hBBB;

        @(negedge wr_clk);

        write_buffer = 2'd2;
        write_x = 10'd123;
        pixel_in = 12'hCCC;

        @(negedge wr_clk);
        // Last write is complete. Stop writing and begin readback.
        wr_en = 1'b0;

        @(negedge rd_clk);
        rd_en = 1'b1;

        read_buffer = 2'd0;
        read_x = 10'd123;

        @(negedge rd_clk);
        assert(pixel_out == 12'hAAA) else $fatal("Test failed: pixel_out = %h, expected 0xAAA", pixel_out);
        $display("PASS: buffer 0, x=123 = %h", pixel_out);

        read_buffer = 2'd1;
        read_x = 10'd123;

        @(negedge rd_clk);
        assert(pixel_out == 12'hBBB) else $fatal("Test failed: pixel_out = %h, expected 0xBBB", pixel_out);
        $display("PASS: buffer 1, x=123 = %h", pixel_out);

        read_buffer = 2'd2;
        read_x = 10'd123;

        @(negedge rd_clk);
        assert(pixel_out == 12'hCCC) else $fatal("Test failed: pixel_out = %h, expected 0xCCC", pixel_out);
        $display("PASS: buffer 2, x=123 = %h", pixel_out);

        rd_en = 1'b0;

        //Test for verifying the BRAM is not being written to when wr_en is low.
        @(negedge wr_clk);
        write_buffer = 2'd1;
        write_x = 10'd200;
        pixel_in = 12'hABC;
        wr_en = 1'b1;

        @(negedge wr_clk);
        wr_en = 1'b0; // Disable write enable
        pixel_in = 12'hDEF; // Change pixel_in value

        @(negedge rd_clk);
        rd_en = 1'b1;
        read_buffer = 2'd1;
        read_x = 10'd200;

        @(negedge rd_clk);
        // Now read back the value at buffer 1, x=200 to verify the original value was not overwritten.
        assert(pixel_out == 12'hABC) else $fatal("Test failed: pixel_out = %h, expected 0xABC", pixel_out);
        $display("PASS: buffer 1, x=200 = %h (write enable low, value should not change)", pixel_out);

        rd_en = 1'b0;

        // Each assertion above stops the simulation if a value is incorrect.
        // Reaching here means all ten readback checks passed.
        // End the simulation explicitly because the clocks run continuously.
        $display("All tests passed.");
        $finish;
    end

    // Device under test (DUT): connect the testbench signals to the line buffer.
    line_buffer_bram dut (
        .write_buffer(write_buffer),
        .write_x(write_x),
        .write_enable(wr_en),
        .write_clk(wr_clk),
        .pixel_in(pixel_in),

        .read_buffer(read_buffer),
        .read_x(read_x),
        .read_enable(rd_en),
        .read_clk(rd_clk),
        .pixel_out(pixel_out)
    );


endmodule
