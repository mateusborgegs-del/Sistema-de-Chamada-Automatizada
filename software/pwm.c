#include <wiringPi.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define PIN_OUT 0   // out
#define PIN_BTN 1   // in


#define FREQ 100.0          
#define PERIOD_US 10000     

const float duty_steps[] = {0.0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875, 1.0};
#define NUM_DUTY_STEPS (sizeof(duty_steps)/sizeof(duty_steps[0]))

int duty_index = 0;
pthread_mutex_t lock;

void* wave_thread(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        float duty = duty_steps[duty_index];
        pthread_mutex_unlock(&lock);

        int on_time = (int)(PERIOD_US * duty);
        int off_time = PERIOD_US - on_time;

        if (on_time > 0)
            digitalWrite(PIN_OUT, HIGH), delayMicroseconds(on_time);
        if (off_time > 0)
            digitalWrite(PIN_OUT, LOW), delayMicroseconds(off_time);
    }
    return NULL;
}

void* button_thread(void* arg) {
    int last_state = HIGH;
    while (1) {
        int state = digitalRead(PIN_BTN);
        if (state == LOW && last_state == HIGH) { 
            pthread_mutex_lock(&lock);
            duty_index = (duty_index + 1) % NUM_DUTY_STEPS;
            printf("Duty cycle alterado para %.1f%%\n", duty_steps[duty_index] * 100);
            pthread_mutex_unlock(&lock);
            delay(200); // debounce (200 ms)
        }
        last_state = state;
        delay(10);
    }
    return NULL;
}

int main(void) {
    wiringPiSetup();

    pinMode(PIN_OUT, OUTPUT);
    pinMode(PIN_BTN, INPUT);
    pullUpDnControl(PIN_BTN, PUD_UP);  

    pthread_t tid_wave, tid_button;
    pthread_mutex_init(&lock, NULL);

    pthread_create(&tid_wave, NULL, wave_thread, NULL);
    pthread_create(&tid_button, NULL, button_thread, NULL);


    pthread_join(tid_wave, NULL);
    pthread_join(tid_button, NULL);

    return 0;
}
