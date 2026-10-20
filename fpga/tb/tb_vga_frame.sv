module tb_vga_frame;
    timeunit 1ns;
    timeprecision 1ps;

    localparam int WIDTH  = 640;
    localparam int HEIGHT = 480;

    logic clk = 1'b0;
    logic reset = 1'b1;
    logic hsync;
    logic vsync;
    logic [3:0] red;
    logic [3:0] green;
    logic [3:0] blue;

    integer image_file;
    int pixel_count = 0;

    // Model the board's 100 MHz oscillator. The DUT generates vga_clk.
    always #5 clk = ~clk;

    top dut (
        .clk   (clk),
        .reset (reset),
        .hsync (hsync),
        .vsync (vsync),
        .red   (red),
        .green (green),
        .blue  (blue)
    );

    initial begin
        // Hold reset for 200 ns, then let the Clocking Wizard lock.
        repeat (20) @(negedge clk);
        reset = 1'b0;
    end

    initial begin
        wait (dut.vga_timing_reset === 1'b0);

        // Skip the startup frame: buffer 0 initially contains unwritten data.
        // Wait for vertical sync, then the start of the following frame.
        @(negedge dut.vsync_raw);
        do begin
            @(negedge dut.vga_clk);
        end while (!(dut.x == 10'd0 && dut.y == 10'd0));

        image_file = $fopen("vga_frame.ppm", "w");
        if (image_file == 0)
            $fatal(1, "Could not open vga_frame.ppm");

        // ASCII PPM header: format, dimensions, maximum channel value.
        $fwrite(image_file, "P3\n%0d %0d\n255\n", WIDTH, HEIGHT);
        $display("Capturing one VGA frame...");

        // At raw (0,0), the output still represents the preceding blanking
        // pixel. Start sampling at the next falling edge to capture pixel 0.
        // Continue through one complete frame, including its blanking time.
        do begin
            @(negedge dut.vga_clk);

            if (dut.vga_timing_reset !== 1'b0)
                $fatal(1, "Reset or clock-lock loss during capture");

            if ($isunknown(dut.line_buffer_reader_inst.video_on))
                $fatal(1, "Unknown video_on during capture");

            // The delayed visible flag matches the BRAM output.
            // Falling-edge sampling avoids racing rising-edge updates.
            if (dut.line_buffer_reader_inst.video_on) begin
                if ($isunknown({red, green, blue}))
                    $fatal(1, "Unknown RGB at pixel %0d", pixel_count);

                if (pixel_count >= WIDTH * HEIGHT)
                    $fatal(1, "Too many visible pixels in one frame");

                // Expand each 4-bit channel to 8 bits: 0..15 -> 0..255.
                $fwrite(image_file, "%0d %0d %0d\n",
                        int'(red)   * 17,
                        int'(green) * 17,
                        int'(blue)  * 17);
                pixel_count++;
            end
        end while (!(dut.x == 10'd0 && dut.y == 10'd0));

        $fclose(image_file);

        if (pixel_count != WIDTH * HEIGHT)
            $fatal(1, "Captured %0d pixels; expected %0d",
                   pixel_count, WIDTH * HEIGHT);

        $display("PASS: Saved vga_frame.ppm with %0d pixels", pixel_count);
        $finish;
    end

    // Stop if the clock never locks or frame timing fails to progress.
    initial begin
        #100ms;
        $fatal(1, "Timed out waiting for VGA frame capture");
    end
endmodule
