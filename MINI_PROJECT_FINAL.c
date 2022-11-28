/*
	HO CHI MINH UNIVERSITY OF TECHNOLOGY

	MINI PROJECT

	Subject Name: Microprocessor Lab
	Subject Code: EE3414

	Class Code: TT02
	Instructor: Trinh Vu Dang Nguyen

	Code by:   Nguyen Truong Phu,    ID: 2051058
	Report by: Nguyen Ngoc Minh Anh, ID: 2051033

--------------------------------------------------------------------------------------------------

	Hardware used: DE10-Standard
	Software used: Quartus Prime Lite Edition

	Description:
	- Build a Nios II system with switches, buttons, GPIOs, a 16x2 LCD, and an L298 motor driver.
	- When SW0 is on, blink "Hello World !!" on the first row of the LCD with frequency of 1 Hz.
	- When SW1 is on, control the motor through two PWM signals that will feed into the driver:
		+ The PWM's frequency and duty cycle is shown on the LCD's second row.
		+ SW2 is responsible for the direction of the motor.
		+ KEY1 and KEY2 are responsible for increasing and decreasing the motor speed, respectively.
		+ KEY1/KEY2 increases/decreases duty cycle by 5%. Maximum is 100%, minimum is 5%.
	- When a switch is off, turn off the system related to that switch.
	- All actions should be independent from each other, i.e. LCD communication should not interfere with PWM generation.
*/

#include <stdio.h>
#include <alt_types.h>
#include <sys/alt_timestamp.h>
#include <altera_avalon_pio_regs.h>
#include <system.h>
#include <unistd.h>

#define HALF 25000000       // Number of clock cycles for half a second
#define DELAY 100           // constant used for usleep()
#define HUNDRED_MICROS 5000 // Number of clock cycles for 100 microseconds

unsigned char text[]  = " Hello World !! "; // String to display on first row
unsigned char empty[] = "                "; // Empty string for clearing a specific row
unsigned char fp[]    = "f: 1KHz DC:    %"; // String to display on second row
unsigned short DC;                          // Duty cycle. Only store values from 0 to 100, so unsigned short is used to save memory
unsigned long UP, DOWN;                     // On time and off time within a period

// Marks to use with alt_timestamp(). "now" is global, "wait" is exclusive for syncing LCD and PWM operations.
// More details further below.
unsigned long long PWM_mark, LCD_mark, now, wait;

// "wait_time" is used exclusively for PWM generation.
// Since the frequency of such PWM will be 1 KHz or faster, it will need no more than 50000 clock cycles.
// Therefore, it is assigned "unsigned short" to save memory.
unsigned short LCD_state = 1, PWM_state, wait_time; // States to determine whether LCD/PWM should turn on or off.


// The names says it all. This function creates PWM signal to feed into the L298 motor driver.
void generate_PWM() {

	// Check the state of SW1, ignoring every other switches
	// by shifting left one bit, placing the state of SW1 at the LSB
	// Then use "& 1" to clear every bit except for the LSB
	if (((IORD_ALTERA_AVALON_PIO_DATA(SWITCH_BASE) >> 1) & 1) == 1) {

		// If SW1 is on, continue to check SW2
		// by shifting left two bits instead of one, then do exactly like above
		if (((IORD_ALTERA_AVALON_PIO_DATA(SWITCH_BASE) >> 2) & 1) == 1)

		     // If SW2 is on, reverse the direction of the PWM
			 // by shifting current PWM state one bit to the left, i.e. 01 will become 10
		     IOWR_ALTERA_AVALON_PIO_DATA(PWM_BASE, PWM_state<<1);
		else IOWR_ALTERA_AVALON_PIO_DATA(PWM_BASE, PWM_state); // If SW2 is off, output PWM signal in default direction
	} else IOWR_ALTERA_AVALON_PIO_DATA(PWM_BASE, 0);           // If SW1 is off, turn off PWM completely

	// alt_timestamp() counts the number of clock cycles elapsed.
	// It will only increase, as long as the power is on.
	// This PWM generation function will also be called continuously by the main infinite loop.
	// The conditional statements below will determine when to turn on, and when to turn off, the signal,
	// which indirectly determines its frequency and duty cycle.
	now = alt_timestamp(); // Update timestamp to current point

	if (now - PWM_mark  >= wait_time) {

		// If the number of clock cycles stored in wait_time has passed,
		// toggle the PWM state, aka turn on becomes off, and off becomes on.
		PWM_state = !PWM_state;

		// then depending on the current state, determine what the next wait time will be
		if (PWM_state == 0) wait_time = DOWN;
		else                wait_time = UP;

		// Finally, mark the end of state-checking to get ready for the next call
		PWM_mark = alt_timestamp();
	}
}

// LCD_BASE:
// 0b x  x  x  x  x  x  x  x  x  x
//   RS  E D7 D6 D5 D4 D3 D2 D1 D0

// Refer to HD44780U datasheet for more details.

// This function clears the entire display, then move the cursor to the top left, aka. first row first column.
void LCD_clear() {

	// Bring E high, and settle all other lines for at least 100 microseconds.
	// In this case, RS should be 0, and D[7..0] should be 00000001, or 0x01.
	IOWR_ALTERA_AVALON_PIO_DATA(LCD_BASE, 0b0100000001);
	usleep(DELAY);

	// Then bring E low (other lines MUST be kept the same as above). The command is now sent to the LCD.
	IOWR_ALTERA_AVALON_PIO_DATA(LCD_BASE, 0b0000000001);
	usleep(DELAY); // Do NOT remove this delay, otherwise the LCD won't have enough time to respond to new commands.
}

// This function initializes the LCD by setting appropriate commands.
void LCD_start() {

	// 8-bit, 2-line mode (RS = 0, D[7..0] = 0x38)
	IOWR_ALTERA_AVALON_PIO_DATA(LCD_BASE, 0b0100111000);
	usleep(DELAY);
	IOWR_ALTERA_AVALON_PIO_DATA(LCD_BASE, 0b0000111000);
	usleep(DELAY);

	// Turn on display, cursor hidden (RS = 0, D[7..0] = 0x0C)
	IOWR_ALTERA_AVALON_PIO_DATA(LCD_BASE, 0b0100001100);
	usleep(DELAY);
	IOWR_ALTERA_AVALON_PIO_DATA(LCD_BASE, 0b0000001100);
	usleep(DELAY);

	LCD_clear();
}

// This function writes a single ASCII-based character at the current cursor position,
// then moves the cursor one column to the right.
void LCD_write(unsigned char c) {

	// Merge the ASCII representation of the input character, which has 8 bits, with the rest of the command.
	// We will start sending ASCII data to the screen, so RS is now 1.
	IOWR_ALTERA_AVALON_PIO_DATA(LCD_BASE, (0b1100000000 | c));

	// Since we need to display stuff to the LCD and output PWM signal AT THE SAME TIME, using usleep() will ruin the PWM generation.
	// Therefore, using timestamps instead will keep everything flowing and uninterrupted.

	// Similar to the PWM generation function above, we first use a variable to mark the current point.
	wait = alt_timestamp();

	// We want to wait for 100 microseconds to pass while still keeping the PWM system running.
	while (alt_timestamp() - wait < HUNDRED_MICROS) generate_PWM();

	// Do the same delay technique as above when bringing E down
	IOWR_ALTERA_AVALON_PIO_DATA(LCD_BASE, (0b1000000000 | c));
	wait = alt_timestamp();
	while (alt_timestamp() - wait < HUNDRED_MICROS) generate_PWM();
}

// This function moves the cursor to anywhere we want, within the LCD limit, of course.
void LCD_cursor(unsigned short row, unsigned short col) {

	// Depending on what LCD is being used, we create the corresponding "map" using a two-dimensional array
	// to mark all addresses of each "cell" in the LCD. In this case, we are using a 2-row, 16-column LCD.
	unsigned char data[2][16] = {{0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F},
                                 {0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF}};
	
	// A temporary variable to hold the address of the inputted position.
	unsigned char pick;

	// In C, counting starts from 0. Therefore the top left position will be data[0][0], and bottom right will be data[1][15].
	// If the inputted rows or columns exceeds the LCD limit,
	// this function will stop here, and the main code will continue as if nothing happened.
	if ((row > 1) || (col > 15)) return; else pick = data[row][col];

	// Merge the picked address with the rest of the command.
	IOWR_ALTERA_AVALON_PIO_DATA(LCD_BASE, (0b0100000000 | pick));

	// Again, we need to update both the LCD and the PWM simutaneously.
	wait = alt_timestamp();
	while (alt_timestamp() - wait < HUNDRED_MICROS) generate_PWM();

	// Bring E low from high to confirm the command, whilst keeping PWM generation stable.
	IOWR_ALTERA_AVALON_PIO_DATA(LCD_BASE, (0b0000000000 | pick));
	wait = alt_timestamp();
	while (alt_timestamp() - wait < HUNDRED_MICROS) generate_PWM();
}

// This function prints out a string, character by character, to the LCD by calling LCD_write() repeatedly.
void LCD_print(unsigned char str[], unsigned short size) {
	unsigned short i;
	for (i = 0; i < size; i++) LCD_write(str[i]);
}

// Updates on time and off time for PWM based on duty cycle, which is stored in percentage.
void update_PWM() {

	// 50000 is the number of clock cycles for 1 milisecond, aka. frequency of 1 kHz.
	UP = 50000*DC/100;
	DOWN = 50000-UP;
}

// Initialize everything in the system
void init() {
	DC = 50;                    // Default value is 50%
	update_PWM();               // On time and off time should now be the same (25000).
	PWM_state = 0;              // Bring the PWM signal low by default
	wait_time = DOWN;           // By extension, wait time should also be off time
	alt_timestamp_start();      // Start counting for elapsed clock cycles
	PWM_mark = alt_timestamp(); 
	LCD_mark = alt_timestamp(); // Then set both marks to the latest point

	LCD_start(); // Finally, boot up the LCD
}

// This function is responsible for displaying PWM information on the second row, with a bit of OCD-ish visualization.
void display_PWM() {

	// These two 'if' blocks eliminate leading zeroes,
	// meaning the LCD will display 25% instead of 025%, and 5% instead of 005%, and so on.
	if (DC == 100)
		 fp[12] = '1';
	else fp[12] = ' ';
	if ((((DC/10) % 10) == 0) && (DC/100 == 0))
		 fp[13] = ' ';
	else fp[13] = (char) ((DC/10) % 10) + 0x30;
	     fp[14] = (char) DC % 10  + 0x30;

	// This big block checks for SW1's state to decide whether to print info or erase it.
	if (((IORD_ALTERA_AVALON_PIO_DATA(SWITCH_BASE) >> 1) & 1) == 1) {

		// If SW1 is on, check for SW2's state next,
		if (((IORD_ALTERA_AVALON_PIO_DATA(SWITCH_BASE) >> 2) & 1) == 1) {
			if (DC < 100) {
				fp[11] = ' ';
				fp[12] = '-';
			}
			else fp[11] = '-'; // then either put the minus sign at the right place,
		}
		else fp[11] = ' '; // or clear it.

		// Finally, output the correct information to the second row.
		LCD_cursor(1, 0);
	    LCD_print(fp, sizeof(fp)-1);
	} else {

		// Self-explanatory. Clear the entire second row if SW1 is off.
		LCD_cursor(1, 0);
		LCD_print(empty, sizeof(empty)-1);
	}
}

// This function is responsible for the "Hello World !!" blinking part.
void control_LCD() {
	now = alt_timestamp(); // update elapsed clock cycles in real-time

	// Check for SW0's state
	if ((IORD_ALTERA_AVALON_PIO_DATA(SWITCH_BASE) & 1) == 0) {

		// If SW0 is off, empty the first row, but LCD_state stays at 1 so the text will show as soon as SW0 goes high.
		LCD_state = 1;
		LCD_cursor(0, 0);
		LCD_print(empty, sizeof(empty)-1);
	} else {

		// If SW0 stays on, check whether half a second has passed, to toggle between "Hello World !!" and empty row at 1 Hz.
		if (now - LCD_mark >= HALF) {
	    	if (LCD_state == 0) {
				LCD_cursor(0, 0);
	        	LCD_print(empty, sizeof(empty)-1);
	    	} else {
				LCD_cursor(0, 0);
	    		LCD_print(text, sizeof(text)-1);
	    	}
			LCD_state = !LCD_state;
			LCD_mark = alt_timestamp(); // Save current timestamp right after toggling the state.
		}
	}
}

// All functions above would be useless without this.
int main()
{
	init();

	while (1) {
		generate_PWM();
		display_PWM();
		control_LCD();

		// Check for KEY2 and KEY1 when SW1 is on, aka. when PWM system is active.
		// On DE10-Standard, all four buttons are active low.
		if ((((IORD_ALTERA_AVALON_PIO_DATA(SWITCH_BASE) >> 1) & 1) == 1) && (IORD_ALTERA_AVALON_PIO_DATA(BUTTON_BASE) == 0b1101)) {

			// If KEY1 is pressed, continue running the system like normal while waiting for KEY1 to be released.
			while (IORD_ALTERA_AVALON_PIO_DATA(BUTTON_BASE) == 0b1101) {
				generate_PWM();
				display_PWM();
				control_LCD();
			}

			// On KEY1 release, increase duty cycle by 5, and do nothing when it reaches 100.
			if (DC < 100) DC += 5;
			update_PWM(); // Update on time and off time to match the new duty cycle
		} else
		if ((((IORD_ALTERA_AVALON_PIO_DATA(SWITCH_BASE) >> 1) & 1) == 1) && (IORD_ALTERA_AVALON_PIO_DATA(BUTTON_BASE) == 0b1011)) {
			while (IORD_ALTERA_AVALON_PIO_DATA(BUTTON_BASE) == 0b1011) {
				generate_PWM();
				display_PWM();
				control_LCD();
			}

			// On rising edge of KEY2 (aka KEY2 is pressed then released), decrease duty cycle by 5, minimum is 5.
			if (DC > 5) DC -= 5;
			update_PWM();
		}
	}
	return 0;
}