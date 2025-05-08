#include "lcd_lib.h"
#include "mpu_lib.h"
#include "buttons_lib.h"
#include "game_config.h"
#include "mazes.h"
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile sig_atomic_t process = 1;

uint16_t mazes [LEVELS][LCD_HEIGHT][LCD_WIDTH];
uint16_t init_ball_poses [LEVELS][2] = {{11, 144}, {150, 20}, {30, 120}};
bool restart = true;
int new_level = 1;
pthread_mutex_t restart_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t restart_cond = PTHREAD_COND_INITIALIZER;

void wait_for_restart() {
    pthread_mutex_lock(&restart_mutex);
    while (!restart) {
        pthread_cond_wait(&restart_cond, &restart_mutex);
    }
    pthread_mutex_unlock(&restart_mutex);
}

void * button_press_handler(void *arg) {
    while (1)
    {
        int button = get_button_pressed();
        pthread_mutex_lock(&restart_mutex);
        new_level = button-1;
        restart = true;
    	pthread_cond_signal(&restart_cond);
    	pthread_mutex_unlock(&restart_mutex);
    }
    return NULL;
}

void handle_exit(int sig)  {
    mpu_close();
    buttons_close();
    process = 0;
} 

int main(){
    //signal(SIGINT, handle_exit);

    int mpu_ret = mpu_open();
    int but_ret = buttons_open();

    if (mpu_ret || but_ret)
        return -1;

    pthread_t buttons_thread;
    if (pthread_create(&buttons_thread, NULL, button_press_handler, NULL) != 0) {
        printf("Failed to create buttons thread");
        mpu_close();
        buttons_close();
        return -1;
    }

    memcpy(mazes[0], maze0, LCD_HEIGHT*LCD_WIDTH*2);
    memcpy(mazes[1], maze1, LCD_HEIGHT*LCD_WIDTH*2);
    memcpy(mazes[2], maze2, LCD_HEIGHT*LCD_WIDTH*2);

    static uint16_t ball_pose [2];
    static uint16_t new_ball_pose [2];
    static uint16_t int_ball_pose [2];
static uint16_t last_valid_ball_pose [2];
    static uint16_t ball_velocity [2];
    static int16_t accel_values[3];
    static int level;

    while (1)
    {
        pthread_mutex_lock(&restart_mutex);
        if (restart) {
	    printf("restart\n");
            level = new_level;
            restart = false;
        memcpy(ball_pose, init_ball_poses[level], 2 * sizeof(init_ball_poses[level][0]));
	    memset(ball_velocity, 0, 4);
        }
        pthread_mutex_unlock(&restart_mutex);

        get_accel(&accel_values[0]);
        ball_velocity[0] = ball_velocity[0] + -1 * accel_values[0]/5000;
        ball_velocity[1] = ball_velocity[1] + -1 * accel_values[1]/5000;


	new_ball_pose[0] = ball_pose[0] + ball_velocity[0]; //y
        new_ball_pose[1] = ball_pose[1] + ball_velocity[1]; //x

	int steps = ceil(hypot(new_ball_pose[1] - ball_pose[1], new_ball_pose[0] - ball_pose[0]));
	last_valid_ball_pose[0] = ball_pose[0];
	last_valid_ball_pose[1] = ball_pose[1];
        for (int i = 1; i <= steps; i++){
                int_ball_pose[0] = floor(ball_pose[0] + (new_ball_pose[0] - ball_pose[0]) * ((double)i / steps));
		int_ball_pose[1] = floor(ball_pose[1] + (new_ball_pose[1] - ball_pose[1]) * ((double)i / steps));
		for (uint16_t y=int_ball_pose[0]-BALL_RADIUS-1; y<=int_ball_pose[0]+BALL_RADIUS+1; y++) {
     		   for (uint16_t x=int_ball_pose[1]-BALL_RADIUS-1; x<=int_ball_pose[1]+BALL_RADIUS+1; x++) {
            		if ((int_ball_pose[0]-y)*(int_ball_pose[0]-y) + (int_ball_pose[1]-x)*(int_ball_pose[1]-x)<= (BALL_RADIUS+1)*(BALL_RADIUS+1)) {
               			 if (mazes[level][y][x] == 0x0000){
					printf("Border! y=%d  x=%d\n",y, x);
		                        ball_velocity[0] = 0;
                		        ball_velocity[1] = 0;
                        		ball_pose[0] = last_valid_ball_pose[0];
                        		ball_pose[1] = last_valid_ball_pose[1];
                        		goto draw;
				}
			     }
     		   }
    		}
		if (mazes[level][int_ball_pose[0]][int_ball_pose[1]] == 0xef7b){
                              ball_velocity[0] = 0;
                              ball_velocity[1] = 0;
                              ball_pose[0] = int_ball_pose[0];
                              ball_pose[1] = int_ball_pose[1];
                              printf("Trap!\n"); goto lose;
                 }
		
		else if (mazes[level][int_ball_pose[0]][int_ball_pose[1]] == 0xe225){
                              ball_velocity[0] = 0;
                              ball_velocity[1] = 0;
                              ball_pose[0] = int_ball_pose[0];
                              ball_pose[1] = int_ball_pose[1];
                              printf("Finish!\n"); goto win;
                 }


		last_valid_ball_pose[0] = int_ball_pose[0];
        	last_valid_ball_pose[1] = int_ball_pose[1];
            }

		ball_pose[0] = last_valid_ball_pose[0];
                ball_pose[1] = last_valid_ball_pose[1];
    		goto draw;

    win:
	draw_filled_rect(LCD_WIDTH/2-90,  LCD_HEIGHT/2-20, 150, 30, COLOR_GREEN);
        draw_str(LCD_WIDTH/2-55, LCD_HEIGHT/2-15, "You won!", Font_11x18, COLOR_WHITE, COLOR_GREEN);
        lcd_write();
        wait_for_restart();
	continue;
    lose:
        draw_filled_rect(LCD_WIDTH/2-90,  LCD_HEIGHT/2-20, 150, 30, COLOR_RED);
	draw_str(LCD_WIDTH/2-65, LCD_HEIGHT/2-15, "You lost!", Font_11x18, COLOR_WHITE, COLOR_RED);
	lcd_write();
	wait_for_restart();
	continue;

    draw:
       if (ball_pose[0] >= LCD_HEIGHT - BALL_RADIUS) {
            ball_pose[0] = LCD_HEIGHT - BALL_RADIUS - 1;
            ball_velocity[0] = 0;
        }
        else if (ball_pose[0] <= BALL_RADIUS){
            ball_pose[0] = BALL_RADIUS + 1;
            ball_velocity[0] = 0;
        }
        if (ball_pose[1] >= LCD_WIDTH - BALL_RADIUS) {
            ball_pose[1] = LCD_WIDTH - BALL_RADIUS - 1;
            ball_velocity[1] = 0;
        }
        else if (ball_pose[1] <= BALL_RADIUS) {
            ball_pose[1] = BALL_RADIUS + 1;
            ball_velocity[1] = 0;
        }
	draw_full_screen(mazes[level][0]);
        draw_filled_circle(ball_pose[0], ball_pose[1], BALL_RADIUS, COLOR_MAGENTA);
      
        lcd_write();
    }
    pthread_join(buttons_thread, NULL);
    printf("Program exited cleanly\n"); 
    return 0;
}

