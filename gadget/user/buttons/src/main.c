#include <stdio.h>
#include <unistd.h>
#include <pigpio.h> 


#define PIN_LED 23
#define PIN_BUTTON1 17
#define PIN_BUTTON2 27
#define PIN_BUTTON3 22

int main(int argc, char **argv){
    (void)argc;
    (void)argv;

    int ret = gpioInitialise();
    if(0 > ret){
        fprintf(stderr, "Failed to initialize pigpio - aborting.\n");
        return ret;
    }

    /* setup led pin */
    gpioSetMode(PIN_LED, PI_OUTPUT);

    /* setup pushbuttons pins */
    gpioSetMode(PIN_BUTTON1, PI_INPUT);
    gpioSetMode(PIN_BUTTON2, PI_INPUT);
    gpioSetMode(PIN_BUTTON3, PI_INPUT);
    gpioSetPullUpDown(PIN_BUTTON1, PI_PUD_UP);
    gpioSetPullUpDown(PIN_BUTTON2, PI_PUD_UP); 
    gpioSetPullUpDown(PIN_BUTTON3, PI_PUD_UP); 

    while(1){
        /* Set led pin output based on button pin input */
        uint8_t led_level = (!gpioRead(PIN_BUTTON1)) || (!gpioRead(PIN_BUTTON2)) || (!gpioRead(PIN_BUTTON3));
        gpioWrite(PIN_LED, led_level);
        usleep(100000);
    }

    gpioTerminate();

    return 0;
}
