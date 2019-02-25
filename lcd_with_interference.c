#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#include "console.h"

void *count_thr(void *arg);

int main (void) {
    pthread_t thread[4];
    int rc;
    unsigned long i;

    console_init();

    for (i = 1; i < 4; i += 1) {
        rc = pthread_create(&thread[i], NULL, count_thr, (void *)i);
        assert(rc == 0);
    }

    while (true) {
        /* skip */
    }
    exit(0);
}

void *count_thr(void *arg) {
    unsigned long id = (long)arg;
    unsigned long counter = 0;

    while (true) {
        counter += 1;
        lcd_write_at(id, 0, "Thread %ld counter is %10ld", id, counter);
    }
}

