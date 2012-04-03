#include <string.h>

#include "fedordesk.h"

static led_state_t s_leds_state;
static uint32_t s_timer_counter;

static void desk_clear_leds()
{
	memset(&s_leds_state, 0, sizeof(s_leds_state));
}

static void desk_fire_leds(const led_state_t* led_state)
{
	// fire led
	for (uint8_t y = 0; y < BUTTONS_NUM; ++y)
		led_state->hw_fire_leds(&led_state->leds_matrix[y], y);
}

void desk_init_leds(hw_fire_leds_t cb)
{
	s_timer_counter = 0;
	desk_clear_leds();
	s_leds_state.hw_fire_leds = cb;
}

void desk_button_pressed(button_t b)
{
	// pins jitter? ignore sequential presses in 500ms
	if (s_timer_counter - s_leds_state.last_pressed_ms < 5)
		return;

	// increase speed
	if (s_leds_state.last_pressed_b == b) {
		if (s_leds_state.speed == led_speed2) {
			// turn off leds
			desk_clear_leds();
			return;
		}
		else {
			++s_leds_state.speed;
		}
	}
	// change mode
	else
		// drop leds state
		memset(&s_leds_state.leds_matrix, 0, sizeof(s_leds_state.leds_matrix));

	// init button
	s_leds_state.last_pressed_b = b;
	s_leds_state.last_pressed_ms = s_timer_counter;
}

void desk_timer_100ms_callback()
{
	++s_timer_counter;

	uint8_t delay;
	switch (s_leds_state.speed) {
	case led_speed0:
		delay = 2; // 200 ms
		break;
	case led_speed1:
		delay = 4; // 400 ms
		break;
	case led_speed2:
		delay = 8; // 800 ms
		break;
	default:
		// unknown state
		return;
	}

	// check delay
	if (s_timer_counter % delay != 0)
		return;

	switch (s_leds_state.last_pressed_b) {
	case button0:
		if (s_leds_state.leds_matrix[0] == 0)
			s_leds_state.leds_matrix[0] = (1 << (LEDS_NUM-1));
		else
			s_leds_state.leds_matrix[0] >>= 1;
		break;
	case button1:
		if (s_leds_state.leds_matrix[1] == 0)
			s_leds_state.leds_matrix[1] = 0b100100100100;
		else if (s_leds_state.leds_matrix[1] == 0b1001001001)
			s_leds_state.leds_matrix[1] = 0;
		else
			s_leds_state.leds_matrix[0] >>= 1;
		break;
	case button2:
		if (s_leds_state.leds_matrix[0] == 0 &&
			s_leds_state.leds_matrix[1] == 0 &&
			s_leds_state.leds_matrix[2] == 0)
			s_leds_state.leds_matrix[2] = 0b111111111111;
		else if (s_leds_state.leds_matrix[0] == 0 &&
				 s_leds_state.leds_matrix[1] == 0 &&
				 s_leds_state.leds_matrix[2] != 0) {
			s_leds_state.leds_matrix[1] = 0b111111111111;
			s_leds_state.leds_matrix[2] = 0;
		}
		else if (s_leds_state.leds_matrix[0] == 0 &&
				 s_leds_state.leds_matrix[1] != 0 &&
				 s_leds_state.leds_matrix[2] == 0) {
			s_leds_state.leds_matrix[0] = 0b111111111111;
			s_leds_state.leds_matrix[1] = 0;
		}
		else if (s_leds_state.leds_matrix[0] != 0 &&
				 s_leds_state.leds_matrix[1] == 0 &&
				 s_leds_state.leds_matrix[2] == 0) {
			s_leds_state.leds_matrix[0] = 0;
		}
		break;
	default:
		// unknown state
		return;
	}

	// fire leds
	desk_fire_leds(&s_leds_state);
}
