#include <avr/io.h>
#include <avr/interrupt.h>

#include "fedordesk.h"

/*
 * ATMega16
 * 1MHz clock
 * TIMER0
 *   We use TIMER0 in CTC mode, i.e. interrupt fires
 *   when TCNT0 becomes equal to OCR0 i.e. 125 (1MHz/8Khz).
 *   Every 800 interrupt we will have 10Hz, i.e. 100ms
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
static uint8_t s_leds_layer;
static uint16_t	s_leds_state;
static uint16_t s_leds_mask;

// initialize timer, interrupt and variable
static void timer0_init()
{
	// Set CTC mode (Clear Timer on Compare Match) (p.83)
	// Have to set OCR0 *after*, otherwise it gets reset to 0!
	TCCR0 |= (1 << WGM01);

	// No prescaler (p.85)
	TCCR0 |= (1 << CS00);

	// Set the compare register (OCR0).
	OCR0 = F_CPU / SAMPLE_RATE;    // 1MHz / 8000Hz = 125

	// Enable Output Compare Match Interrupt when TCNT0 == OCR0 (p.85)
	TIMSK |= (1 << OCIE0);
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
	// 6 max fired leds at a time
	// 20mA * 6 = 120mA max
	s_leds_mask = 0b111111;
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

	// toggle mask
	s_leds_mask ^= 0b111111111111;

	// current fired leds
	uint16_t leds = s_leds_state & s_leds_mask;

	// turn on led
	PORTA |= (leds & 0xff);
	PORTC |= ((leds >> 8) & 0b1111);

	// turn on ground
	PORTC |= (1 << ((s_leds_layer % LAYERS_NUM) + 4));
}

// TIMER0 Output Compare Match Interrupt service routine
ISR(TIMER0_COMP_vect)
{
	// global variable to count the number of overflows
	static uint16_t s_overflow = 0;

	// keep a track of number of overflows
	s_overflow++;

	// fire leds on every interrupt for power savings
	// i.e. do persistence of vision (pov) with frequent flicking
	fire_leds();

	// 800 overflows = 100 ms delay
	if (s_overflow == 800) {
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
