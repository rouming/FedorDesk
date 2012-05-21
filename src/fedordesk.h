#ifndef FEDORDESK_H
#define FEDORDESK_H

#include <avr/io.h>

typedef enum led_speed {
	led_speed0 = 0,
	led_speed1 = 1,
	led_speed2 = 2
} led_speed_t;

typedef enum button {
	button_unknown = 0,
	button0 = 1,
	button1 = 2,
	button2 = 3
} button_t;

typedef void (*hw_fire_leds_t)(const uint16_t* leds, uint8_t y);

typedef struct led_state {
	led_speed_t    speed;
	button_t       last_pressed_b;
	uint32_t       last_pressed_ms;
	uint16_t       leds_matrix[3];
	uint32_t       timer_counter;
	hw_fire_leds_t hw_fire_leds;
} led_state_t;

#define LEDS_NUM    12
#define LAYERS_NUM  3
#define BUTTONS_NUM 3

void desk_init_leds(hw_fire_leds_t cb);
void desk_button_pressed(button_t b);
void desk_timer_100ms_callback();

#endif //FEDORDESK_H
