#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#include "console.h"

enum {
	FLASH_MIN_DELAY     = 100000,
	FLASH_INITIAL_DELAY = 1000000,
	FLASH_MAX_DELAY     = 3000000,
	FLASH_DELAY_STEP    = 100000
};

static bool flashing[2] = {false, false};
static int  flashing_delay[2] = {FLASH_INITIAL_DELAY, FLASH_INITIAL_DELAY};
static bool led_status_changed[2] = {false, false};
static sem_t lcd_sem;

void *led_toggle_thr(void *arg);
void *keyboard_thr(void *arg);
void inc_delay(int i);
void dec_delay(int i);

int main (void) {
    pthread_t thread[3];
    int rc;
    unsigned long i;

    console_init();
    
    rc = sem_init(&lcd_sem, 0, 1);
    assert(rc == 0);

    for (i = 0; i < 2; i += 1) {
        rc = pthread_create(&thread[i], NULL, led_toggle_thr, (void *)i);
        assert(rc == 0);
    }
    rc = pthread_create(&thread[2], NULL, keyboard_thr, NULL);
    assert(rc == 0);

    while (true) {
        /* skip */
    }
    exit(0);
}

void *led_toggle_thr(void *arg) {
    unsigned long id = (long)arg;
    int rc;

    while (true) {
        rc = sem_wait(&lcd_sem);
        assert(rc == 0);
        if (flashing[id]) {
            led_toggle((leds_t)id);
        }
        if (led_status_changed[id]) {
            lcd_write_at(id ,0, "(LED%d) F:%s D:%07d", id, 
                    (flashing[id] ? "ON " : "OFF"), 
                     flashing_delay[id]);
            led_status_changed[id] = false;
        }
        rc = sem_post(&lcd_sem);
        assert(rc == 0);
        usleep(flashing_delay[id]);
    }
}

void *keyboard_thr(void * arg) {
    while (true) {
        switch (key_pressed()) {
            case KEY_LEFT: {
                flashing[0] = !flashing[0];
                led_status_changed[0] = true;
                break;
            }
            case KEY_RIGHT: { 
                flashing[1] = !flashing[1];
                led_status_changed[1] = true;
                break;
            }
            case KEY_UP: { 
                dec_delay(0);
                led_status_changed[0] = true;
                break;
            }
            case KEY_DOWN: {
                inc_delay(0);
                led_status_changed[0] = true;
                break;
            }
            case KEY_PPAGE: {
                dec_delay(1);
                led_status_changed[1] = true;
                break;
            }
            case KEY_NPAGE: {
                inc_delay(1);
                led_status_changed[1] = true;
                break;
            }
            default: {
                break;
            }
        }
        usleep(100000);
    }
}

void inc_delay(int i) {
    if (flashing_delay[i] + FLASH_DELAY_STEP > FLASH_MAX_DELAY) {
        flashing_delay[i] = FLASH_MAX_DELAY;
    }
    else {
        flashing_delay[i] += FLASH_DELAY_STEP;
    }
}

void dec_delay(int i) {
    if (flashing_delay[i] - FLASH_DELAY_STEP < FLASH_MIN_DELAY) {
        flashing_delay[i] = FLASH_MIN_DELAY;
    }
    else {
        flashing_delay[i] -= FLASH_DELAY_STEP;
    }
}

