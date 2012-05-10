#include <avr/io.h>
#include <avr/interrupt.h>

#include "fedordesk.h"

/*
 * ATMega16
 * 1MHz clock
 * timer 8-bit overflow interrupt every 256 us
 * every 400 8-bit overflow we get ~102.4 ms delay
 * i.e.: 1000ms / (1000'000Hz / 256) * 400 ~ 102.4ms
 * for power saving we never fire all leds, we try to flicker them
 * i.e. we flicker every overflow interrupt, i.e. with ~3906.25Hz
 *
 *
 * Timers:
 *     http://easyelectronics.ru/avr-uchebnyj-kurs-tajmery.html
 *
 *     http://maxembedded.wordpress.com/2011/06/24/avr-timers-timer0-2
 *     http://maxembedded.wordpress.com/2011/06/28/avr-timers-timer1
 *     http://maxembedded.wordpress.com/2011/06/28/avr-timers-timer2
 *
 * Audio:
 *     http://avrpcm.blogspot.com/2010/11/playing-8-bit-pcm-using-any-avr.html
 *     http://www.arduino.cc/playground/Code/PCMAudio
 *
 * BUTTON[0..2] -> PD2, PD3, PB2
 * LED[0..11]   -> PA[0..7], PC[0..3]
 * GR[0..2]     -> PC[4..6]
 *
 */

// leds state
static uint8_t s_leds_index;
static uint8_t s_leds_layer;
static uint16_t	s_leds_state;

// 20mA * 6 = 120mA max
#define MAX_FIRED_LEDS 6

// initialize timer, interrupt and variable
static void timer0_init()
{
	// No any prescaler
	TCCR0 |= (1 << CS00);

	// initialize counter
	TCNT0 = 0;

	// enable overflow interrupt
	TIMSK |= (1 << TOIE0);
}

static void external_int_init()
{
	// enable falling edge on INT0, INT1
	MCUCR |= (1<<ISC01) | (1<<ISC11);
	// enable falling edge on INT2
	MCUCSR &= ~(1<<ISC2);

	// enable INT0, INT1, INT2
	GICR |= (1<<INT0) | (1<<INT1) | (1<<INT2);
}

static void init_io_ports()
{
	/*
	 * LED[0..11]   -> PA[0..7], PC[0..3]
	 * GR[0..2]     -> PC[4..6]
	 */

	// 0..7 A pins as output
	DDRA |= 0b11111111;
	// 0..3 C pins as output
	DDRC |= 0b00001111;
	// 4..6 C pins as output
	DDRC |= 0b01110000;

	// 0..7 A pins to low
	PORTA &= ~0b11111111;
	// 0..3 C pins to low
	PORTC &= ~0b00001111;
	// 4..6 C pins to low
	PORTC &= ~0b01110000;
}

static void hw_fire_leds(const uint16_t* leds, uint8_t y)
{
	// save leds state
	s_leds_layer = y;
	s_leds_state = *leds;
	s_leds_index = 0;
}

static void fire_leds()
{
	// firstly turn off leds

	// 0..7 A pins to low
	PORTA &= ~0b11111111;
	// 0..3 C pins to low
	PORTC &= ~0b00001111;
	// 4..6 C pins to low
	PORTC &= ~0b01110000;

	// nothing to do
	if (!s_leds_state)
		return;

	// get copy
	uint16_t leds_state = s_leds_state;
	for (uint8_t i = 0; i < MAX_FIRED_LEDS && leds_state; ++i) {
		uint16_t led = 0;
		// get set bit in leds state
		while (!led)
			led = leds_state & (1 << (s_leds_index++ % LEDS_NUM));

		// avoid of char overflow
		s_leds_index %= LEDS_NUM;

		// drop found bit
		leds_state &= ~led;

		// turn on led
		PORTA |= (led & 0xff);
		PORTC |= ((led >> 8) & 0b1111);
	}

	// turn on ground
	PORTC |= (1 << ((s_leds_layer % LAYERS_NUM) + 4));
}

// TIMER0 overflow interrupt service routine
// called whenever TCNT0 overflows
ISR(TIMER0_OVF_vect)
{
	// global variable to count the number of overflows
	static uint16_t s_overflow = 0;

	// keep a track of number of overflows
	s_overflow++;

	// fire leds on every interrupt for power savings
	// i.e. do persistence of vision (pov) with frequent flicking
	fire_leds();

	// 400 overflows = ~102 ms delay
	if (s_overflow == 400) {
		desk_timer_100ms_callback();

		// reset overflow counter
		s_overflow = 0;
	}
}

ISR(INT0_vect)
{
	desk_button_pressed(button0);
}

ISR(INT1_vect)
{
	desk_button_pressed(button1);
}

ISR(INT2_vect)
{
	desk_button_pressed(button2);
}

int main()
{
	// init io ports
	init_io_ports();
	// init timer
	timer0_init();

	// init external interrupts
	external_int_init();

	// init leds
	desk_init_leds(&hw_fire_leds);

	// enable global interrupts
	sei();

	// loop forever
	while(1)
		;

	return 0;
}
