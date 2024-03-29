// Final Project Edit 1. 
/* This files provides address values that exist in the system */

#define SDRAM_BASE            0xC0000000
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_CHAR_BASE        0xC9000000

/* Cyclone V FPGA devices */
#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define TIMER_BASE            0xFF202000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030

/* VGA colors */
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
#define ORANGE 0xFC00

#define ABS(x) (((x) > 0) ? (x) : -(x))

/* Screen size. */
#define RESOLUTION_X 320
#define RESOLUTION_Y 240

/* Constants for animation */
#define BOX_LEN 2
#define NUM_BOXES 8

#define FALSE 0
#define TRUE 1

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

void clear_screen(); 
void draw_line(int x0, int y0, int x1, int y1, short int line_color); 
void plot_pixel(int x, int y, short int line_color); 
void swap(int *x, int *y);
void wait_for_vsync(); 

// Begin part3.c code for Lab 7


volatile int pixel_buffer_start; // global variable

int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    // declare other variables(not shown)
    // initialize location and direction of rectangles(not shown)

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
	
	unsigned char pressedKey = 0;
    int delta_up = 3, delta_down = -3, delta_right = 3, delta_left = -3, x_init = 100, y_init = 100; 
    int x_fin = 120, y_fin = 120; 
	
	clear_screen(); 
	
    while (true)
    {

		load_screen(x_init, y_init, x_fin, y_fin); 
		read_keyboard(&pressedKey);
		
		if (pressedKey == 0x29) {
			make_box(x_init, y_init, x_fin, y_fin); 
		   }
		
		if(pressedKey == 0x74){
			x_init = x_init + delta_right;
			x_fin = x_fin + delta_right;
			make_box(x_init, y_init, x_fin, y_fin); 
		}
		
		if(pressedKey == 0x72){ 
			y_init = y_init + delta_up;
			y_fin = y_fin + delta_up;
			make_box(x_init, y_init, x_fin, y_fin); 
		}
		
		if(pressedKey == 0x75){
			y_init = y_init + delta_down;
			y_fin = y_fin + delta_down;
			make_box(x_init, y_init, x_fin, y_fin); 
		}
		
		if(pressedKey == 0x6B){
			x_init = x_init + delta_left;
			x_fin = x_fin + delta_left;
			make_box(x_init, y_init, x_fin, y_fin); 
		}
		
        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
}

// code for subroutines (not shown)
void plot_pixel(int x, int y, short int line_color) {
  *(short int * )(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void clear_screen() {
  for (int x = 0; x < RESOLUTION_X; x++) {
    for (int y = 0; y < RESOLUTION_Y; y++) {
      plot_pixel(x, y, CYAN);
  }
  }
}

void load_screen(int x_init, int y_init, int x_fin, int y_fin) {
  for (int x = x_init-6; x < x_fin+6; x++) {
    for (int y = y_init-6; y < y_fin+6; y++) {
      plot_pixel(x, y, CYAN);
   }
 }
}


void make_box(int x_init, int y_init, int x_fin, int y_fin){
  for (int x = x_init; x < x_fin; x++) {
    for (int y = y_init; y < y_fin; y++) {
      plot_pixel(x, y, RED);
   }
 }
}

void draw_line(int x0, int y0, int x1, int y1, short int line_color){
    bool is_steep = abs(y1-y0) > abs(x1-x0); 
    if(is_steep){
        swap(&x0, &y0); 
        swap(&x1, &y1); 
    }
    if(x0>x1){
        swap(&x0, &x1); 
        swap(&y0, &y1); 
    }
    int deltax = x1 - x0; 
    int deltay = abs(y1-y0); 
    int error = -(deltax/2); 
    int y = y0; 
    int y_step; 
    if(y0<y1){
        y_step = 1; 
    } else{
        y_step = -1; 
    }
    for(int i = x0; i <= x1; i++){
        if(is_steep){
            plot_pixel (y, i, line_color); 
        } else{
            plot_pixel (i, y, line_color); 
        }
        error = error + deltay; 
        if(error>=0){
            y = y+y_step; 
            error = error - deltax; 
        }
    }
}

void swap(int *x, int *y){
    int temp = *x; 
    *x = *y;
    *y = temp; 
}

void wait_for_vsync() {
    volatile int * pixel_ctrl_ptr = (int*)0xFF203020;
    register int status;
    
    *pixel_ctrl_ptr = 1;
    
    status = *(pixel_ctrl_ptr + 3); 
    while ((status & 0x01) != 0) 
    status = *(pixel_ctrl_ptr + 3); 
}

void read_keyboard(unsigned char *pressedKey) {
	volatile int * PS2_ptr = (int *) 0xFF200100;
	int data = *PS2_ptr;
	*pressedKey = data & 0xFF;

	while (data & 0x8000) {
		data = *PS2_ptr;
	}
}



