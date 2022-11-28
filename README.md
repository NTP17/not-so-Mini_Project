# Not-so-mini Project
One of my achievements in university that I am proud of. Special thanks to [my teammate](https://www.facebook.com/100054322087193/) for helping me write a report for this project.

## Project Summary

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

Notes:
- "MINI_PROJECT_FINAL.c" is the main code for the Nios II Eclipse Software Build Tool, i.e. its content should be copied to "hello_world.c" if "Hello World" is chosen when creating new project with BSP template.
- "firmware.qsys" can be imported directly to Quartus's Platform Designer.
- "LCD_and_PWM.v" is the top-level entity. Some files from the Platform Designer must be included to the project first for successful compilation.

Refer to the files and pictures below for clearer details.

[LAB1.pdf](https://github.com/NTP17/not-so-Mini_Project/files/10100229/LAB1.pdf)

![Untitled](https://user-images.githubusercontent.com/108677525/204188974-8cd633dc-ca7b-4114-8619-edef426402ce.png)

![Untitled ](https://user-images.githubusercontent.com/108677525/204188983-6351548b-756d-4e9d-ba7f-f2ee8aa2bef8.png)

![Untitled  ](https://user-images.githubusercontent.com/108677525/204188989-07c907ec-b960-41b3-ab73-bb32ef7b6eaf.png)

## Videos showcasing the final result

[LCD1602 and driver-based Motor](https://drive.google.com/file/d/121-UZ29gHzr8yJedm-8hDNA1izHyYURZ/view?usp=share_link)

[Showing PWM on an oscilloscope](https://drive.google.com/file/d/12877B6pZOsTfwTk7toCMlTWXxNrooujn/view?usp=share_link)
