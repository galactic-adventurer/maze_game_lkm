#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/lcd_button_char_dev"

// Function for Thread 1: Wait for Button Interrupt
void *interrupt_handler(void *arg) {
    int fd = *(int *)arg;
    char buffer[2];

    printf("Thread 1: Waiting for button press...\n");
    read(fd, buffer, sizeof(buffer)); // Blocks until interrupt occurs
    printf("Thread 1: Button %s pressed!\n", buffer);

    return NULL;
}

// Function for Thread 2: Perform Other Tasks
void *other_tasks(void *arg) {
    
    printf("Thread 2: Performing other tasks...\n");
    sleep(15); // Simulate work
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    int fd;

    fd = open(DEVICE_PATH, O_RDONLY); // Open the device
    if (fd < 0) {
        perror("Failed to open buttons device");
        return EXIT_FAILURE;
    }

    // Create Thread 1 to handle button interrupts
    if (pthread_create(&thread1, NULL, interrupt_handler, &fd) != 0) {
        perror("Failed to create Thread 1");
        return EXIT_FAILURE;
    }

    // Create Thread 2 to perform other tasks
    if (pthread_create(&thread2, NULL, other_tasks, NULL) != 0) {
        perror("Failed to create Thread 2");
        return EXIT_FAILURE;
    }

    // Wait for threads to complete (this will likely be infinite)
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    close(fd); // Close device file
    return 0;
}
