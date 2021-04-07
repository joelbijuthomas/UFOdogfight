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

	int arrayx[100]; 
	int arrayy[100]; 
	int x_shift[100]; 
	int y_shift[100]; 
	int x_terms[100] = {-1,0,1}; 
	int y_terms[100] = {-1,0,1}; 
        for(int i=0; i<8; i++){
			arrayx[i] = rand() % 319; 
			arrayy[i] = rand() % 239; 
			x_shift[i] = x_terms[rand()%3];
            y_shift[i] = y_terms[rand()%3];
		}
	
	int y3 = 0; 
	
    while (true)
    {
		clear_screen(); 
        /* Erase any boxes and lines that were drawn in the last iteration */
		
		for(int i=0; i<8; i++){		
			arrayx[i]+=x_shift[i];
            arrayy[i]+=y_shift[i];

			
			for(int j= 1; j<3; j++){
			plot_pixel(arrayx[i], arrayy[i], BLUE);
            plot_pixel(arrayx[i] + j , arrayy[i], BLUE);
            plot_pixel(arrayx[i], arrayy[i] + j, BLUE);
            plot_pixel(arrayx[i] + j, arrayy[i] + j, BLUE);
			}
			
			if(i<7){
			draw_line(arrayx[i], arrayy[i], arrayx[i+1], arrayy[i+1], GREEN);
			}
			if(i==7){
				draw_line(arrayx[i], arrayy[i], arrayx[0], arrayy[0], GREEN);
			}
			
			if (arrayx[i] == 0) {
				x_shift[i] = 1;
			}
            else if (arrayx[i] == 319) {
				x_shift[i] = -1;
			}
            if (arrayy[i] == 0) {
				y_shift[i] = 1;
			}
            else if (arrayy[i] == 239){
				y_shift[i] = -1;
			}

		}
		
		

        // code for drawing the boxes and lines (not shown)
        // code for updating the locations of boxes (not shown)

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
	if(*(short int * )(pixel_buffer_start + (y << 10) + (x << 1)) != 0x0000)
      plot_pixel(x, y, 0x0000);
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


