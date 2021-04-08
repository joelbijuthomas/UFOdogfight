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

typedef enum{
    start,
    game,
    player_dead,
    AI_dead,
}gamestage;

gamestage stage = game;

//global variable
int turn = 0;

//structs
typedef struct UFO{
    int x;
    int y;
    int dx;
    int dy;
} UFO;

typedef struct MISSILE{
    int x;
    int y;
    int dx;
    int dy;
} MISSILE;

//function prototypes

void clear_screen(); 
void draw_line(int x0, int y0, int x1, int y1, short int line_color); 
void plot_pixel(int x, int y, short int line_color); 
void swap(int *x, int *y);
void wait_for_vsync();
void keyboard_input(char *keypressed);
void draw_UFO(UFO *ufo, short int line_color);
void update_location_UFO(UFO *ufo, char PS2Data, MISSILE *missile);
void clear_UFO(UFO *ufo); 
void update_AI_location(UFO *ufo, int counter);
void draw_missile(MISSILE *missile);
void update_missile_location(MISSILE *missile);
int check_hit(UFO *ufo, MISSILE *missile);

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
	
    UFO ufo1 = {100,120,0,0};
    UFO *ufo1_ptr = &ufo1; 
	UFO ufo2 = {100,100,0,0};
    UFO *ufo2_ptr = &ufo2;
    MISSILE missile1 = {0,0,0,0};
    MISSILE missile2;
    MISSILE *missile1_ptr = &missile1;
    MISSILE *missile2_ptr = &missile2; 
    char key_pressed = 0;
    char *key_pressed_ptr = &key_pressed;

	clear_screen();
	
	int counter = 0; 
	
    while (1)
    {
        switch (stage){
            case game:
                counter = counter + 1;
                clear_screen(); 
                //clear_UFO(ufo1_ptr); 
                //clear_UFO(ufo2_ptr); 
                
                keyboard_input(key_pressed_ptr);
                draw_UFO(ufo1_ptr, GREEN);
                draw_UFO(ufo2_ptr, RED);
                update_location_UFO(ufo1_ptr, key_pressed, missile1_ptr);
                
                update_AI_location(ufo2_ptr, counter);
                draw_missile(missile1_ptr);
                update_missile_location(missile1_ptr);


                wait_for_vsync(); // swap front and back buffers on VGA vertical sync
                pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
                if(counter == 7){
                    counter = 0; 
                }
                if(check_hit(ufo2_ptr, missile1_ptr)){
                    stage = AI_dead;
                    break;
                }
                break;

            case AI_dead:
                clear_screen();
                wait_for_vsync();
                pixel_buffer_start = *(pixel_ctrl_ptr + 1);
                break;
            
            default:
                printf("error\n");
        }
    }
}

// code for subroutines (not shown)
void plot_pixel(int x, int y, short int line_color) {
  *(short int * )(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void clear_screen() {
    for (int x = 0; x < RESOLUTION_X; x++) {
        for (int y = 0; y < RESOLUTION_Y; y++) {
            plot_pixel(x,y,0);
        }
    }
}

void clear_UFO(UFO *ufo) {
  for (int x = ufo->x - 20; x < ufo->x + 20; x++) {
    for (int y = ufo->y - 20; y < ufo->y + 20; y++) {
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

void keyboard_input(char *keypressed){
    volatile int *PS2_ptr = (int *) 0xFF200100;
    int Data = *PS2_ptr;
    *keypressed = Data & 0xFF;
    while(Data & 0x8000){
        Data = * PS2_ptr;
    }
}

void draw_UFO(UFO *ufo, short int line_color){
    for(int x_shift = 0; x_shift <10; x_shift++){
	for(int y_shift = 0; y_shift <10; y_shift++){
    plot_pixel(ufo->x + x_shift, ufo->y + y_shift, line_color);
	}
	}
}

void update_location_UFO(UFO *ufo, char PS2Data, MISSILE *missile){
    if(PS2Data == 0x29  && turn){
        missile->x = ufo->x;
        missile->y = ufo->y;
        if(ufo->dx > 0){
            missile->dx = 7;
            missile->dy = 0; 
        }
        else if(ufo->dx < 0){
            missile->dx = -7;
            missile->dy = 0;
        }
        else if(ufo->dy > 0){
            missile->dx = 0;
            missile->dy = 7;
        }
        else if(ufo->dy < 0){
            missile->dx = 0;
            missile->dy = -7;
        }
        else{
            missile->dx = 0;
            missile->dy = 0;
        }
        turn = 0;
    }
    else if(PS2Data == 0x74){
        ufo->dx = 2;
		ufo->dy = 0;
        turn = 1;
    }
    else if(PS2Data == 0x6B){
        ufo->dx = -2;
		ufo->dy = 0;
        turn = 1;
    }
	else if(PS2Data == 0x72){
        ufo->dy = 2;
		ufo->dx = 0;
        turn = 1;
    }
	else if(PS2Data == 0x75){
        ufo->dy = -2;
		ufo->dx = 0;
        turn = 1;
    }
    if(ufo->x < 1  || ufo->x > 314){
        ufo->dx = -ufo->dx;
    }
    else if(ufo->y < 8 || ufo->y > 234){            //These lines have to be changed.
        ufo->dy = -ufo->dy;
    }
    ufo->x += ufo->dx;
    ufo->y += ufo->dy;
    ufo->x += ufo->dx;
	ufo->y += ufo->dy;
}

void update_AI_location(UFO *ufo, int count){
    if((rand()%4)==0){
        ufo->dx = 2;
		ufo->dy = 0; 
    }
    else if(((rand()%4)==1) && (count==7)){
        ufo->dx = -2;
		ufo->dy = 0;  
    }
	else if((rand()%4)==2 && (count==7)){
        ufo->dy = 2;
		ufo->dx = 0; 
    }
	else if((rand()%4)==3 && (count==7)){
        ufo->dy = -2;
		ufo->dx = 0; 
    }
    ufo->x += ufo->dx;
	ufo->y += ufo->dy;
}

void draw_missile(MISSILE *missile){
    for(int i = 0; i<4; i++){
        for(int j = 0; j<4; j++){
            plot_pixel(missile->x + i, missile->y + j, ORANGE);
        }
    }
}

void update_missile_location(MISSILE *missile){
    if(missile->x < 1  || missile->x > 314){
        missile->dx = -missile->dx;
    }
    else if(missile->y < 5 || missile->y > 234){
        missile->dy = -missile->dy;
    }
    missile->x += missile->dx;
    missile->y += missile->dy;
}

int check_hit(UFO *ufo, MISSILE *missile){
    int centreX = missile->x + 5;
    int centreY = missile->y + 5;

    if((centreX > ufo->x) && (centreX < ufo->x + 10) && (centreY > ufo->y) && (centreY < ufo->y + 10)){
        return 1;
    }
    else{
        return 0;
    }
}