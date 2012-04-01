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
 * http://easyelectronics.ru/avr-uchebnyj-kurs-tajmery.html
 *
 * http://maxembedded.wordpress.com/2011/06/24/avr-timers-timer0-2
 * http://maxembedded.wordpress.com/2011/06/28/avr-timers-timer1
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

void int0_init()
{
	// enable negative edge on INT0
	MCUCR |= (1<<ISC01);
	// enable INT0
	GICR |= (1<<INT0);
}


void int1_init()
{
	// enable negative edge on INT1
	MCUCR |= (1<<ISC11);
	// enable INT1
	GICR |= (1<<INT1);
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
	//TODO: add press rate control
	desk_button_pressed(button0);
}

ISR(INT1_vect)
{
	//TODO: add press rate control
	desk_button_pressed(button1);
}

int main()
{
	// initialize timer
	timer0_init();

	// initialize int0 and int1
	int0_init();
	int1_init();

	// enable global interrupts
	sei();

	// loop forever
	while(1)
		;

	return 0;
}
