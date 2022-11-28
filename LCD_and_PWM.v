
//=======================================================
//  This code is generated by Terasic System Builder
//=======================================================

module LCD_and_PWM(

	//////////// CLOCK //////////
	input 		          		CLOCK2_50,
	input 		          		CLOCK3_50,
	input 		          		CLOCK4_50,
	input 		          		CLOCK_50,

	//////////// SW //////////
	input 		     [9:0]		SW,
	input            [3:0]     KEY,

	//////////// GPIO, GPIO connect to GPIO Default //////////
	inout 		    [35:0]		GPIO
);

firmware (
	.clk_clk(CLOCK_50),
	.switch_wire_export(SW),
	.button_wire_export(KEY),
	.lcd_wire_export(GPIO[35:26]),
	.pwm_wire_export(GPIO[25:24]),
	.reset_reset_n(1'b1)
);

endmodule
