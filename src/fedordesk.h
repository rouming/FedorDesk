#ifndef FEDORDESK_H
#define FEDORDESK_H

typedef enum led_speed {
	led_speed0 = 0,
	led_speed1 = 1,
	led_speed2 = 2
} led_speed_t;

typedef enum button {
	button0 = 0,
	button1 = 1,
	button2 = 2
} button_t;

#define LEDS_NUM    12
#define BUTTONS_NUM 3

void desk_button_pressed(button_t b);
void desk_timer_100ms_callback();

#endif //FEDORDESK_H
