#include <avr/io.h>
#include <avr/interrupt.h>

#include "fedordesk.h"

/*
 * ATMega16
 * 16MHz clock
 * prescaler 256
 * timer interrupt every 4.096 ms
 * every 25 8-bit overflow we get ~100 ms delay
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
 * GR[0..2]     -> PB[4..6]
 *
 */

// initialize timer, interrupt and variable
void timer0_init()
{
	// set up timer with prescaler = 256
	TCCR0 |= (1 << CS02);

	// initialize counter
	TCNT0 = 0;

	// enable overflow interrupt
	TIMSK |= (1 << TOIE1);
}

void external_int_init()
{
	// enable falling edge on INT0, INT1
	MCUCR |= (1<<ISC01) | (1<<ISC11);
	// enable falling edge on INT2
	MCUCSR &= ~(1<<ISC2);

	// enable INT0, INT1, INT2
	GICR |= (1<<INT0) | (1<<INT1) | (1<<INT2);
}

void init_io_ports()
{
	/*
	 * LED[0..11]   -> PA[0..7], PC[0..3]
	 * GR[0..2]     -> PB[4..6]
	 */

	// 0..7 A pins as output
	DDRA |= 0b11111111;
	// 0..3 C pins as output
	DDRC |= 0b00001111;
	// 4..6 B pins as output
	DDRB |= 0b01110000;

	// 0..7 A pins to low
	PORTA &= ~0b11111111;
	// 0..3 C pins to low
	PORTC &= ~0b00001111;
	// 4..6 B pins to high
	PORTB |=  0b01110000;
}

static void hw_fire_leds(const uint16_t* leds, uint8_t y)
{
	// firstly turn off leds

	// 0..7 A pins to low
	PORTA &= ~0b11111111;
	// 0..3 C pins to low
	PORTC &= ~0b00001111;

	// turn on
	if (*leds) {
		// ground to low
		PORTB &= ~(1 << (y + 4));

		// leds to high
		PORTA |= (*leds & 0xff);
		PORTC |= ((*leds >> 8) & 0b1111);
	}
	// turn off
	else
		// ground to high
		PORTB |= (1 << (y + 4));
}

// TIMER0 overflow interrupt service routine
// called whenever TCNT0 overflows
ISR(TIMER0_OVF_vect)
{
	// global variable to count the number of overflows
	static uint8_t s_overflow = 0;

	// keep a track of number of overflows
	s_overflow++;

	// 25 overflows = ~100 ms delay
	if (s_overflow == 25) {
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
