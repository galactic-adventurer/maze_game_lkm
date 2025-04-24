#include "lcd_lib.h"
#include "mpu_lib.h"
#include "buttons_lib.h"
#include "game_config.h"
#include "mazes.h"
#include <stdatomic.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>

volatile sig_atomic_t process = 1;

uint16_t mazes [LEVELS][LCD_HEIGHT][LCD_WIDTH];
uint16_t init_ball_poses [LEVELS][2] = {{16, 3}, {10, 10}, {10, 10}};
atomic_bool restart = ATOMIC_VAR_INIT(true);
atomic_int new_level = ATOMIC_VAR_INIT(0);


void * button_press_handler(void *arg) {
    while (process) 
    {
        int button = get_button_pressed();
        atomic_store(&new_level, button-1);
        atomic_store(&restart, true);
    }
    return NULL;
}

void handle_exit(int sig)  {
    mpu_close();
    buttons_close();
    process = 0;
} 

int main(){
    signal(SIGINT, handle_exit);

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

    memcpy(mazes[0], maze0, LCD_HEIGHT*LCD_HEIGHT*2);
    //memcpy(mazes[1], maze1, LCD_HEIGHT*LCD_HEIGHT*2);
    //memcpy(mazes[2], maze2, LCD_HEIGHT*LCD_HEIGHT*2);

    static uint16_t ball_pose [2];
    static int16_t accel_values[3];
    static int level;

    while (process) 
    {
        if (atomic_load(&restart)) {
            level = atomic_load(&new_level);
            memcpy(ball_pose, init_ball_poses[level], 2 * sizeof(init_ball_poses[level][0]));
            atomic_store(&restart, false);
        }

        get_accel(accel_values);
        
      //decide movement (considering borders)
      //check if not in trap - restart game or check win
        draw_full_screen(mazes[level][0]);
        draw_filled_circle(ball_pose[0], ball_pose[1], BALL_RADIUS, COLOR_MAGENTA);
      
        lcd_write();
    }
    pthread_join(buttons_thread, NULL);
    printf("Program exited cleanly\n"); 
    return 0;
}

