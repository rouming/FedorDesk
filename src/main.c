#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "sounddata.h"
#include "fedordesk.h"

/*
 * ATMega16
 * TIMER1
 *   We use 16-bit TIMER1 in CTC mode, i.e. interrupt fires
 *   when TCNT1 becomes equal to OCR1A
 *   Every N interrupt we will have 10Hz, i.e. 100ms
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
 * PD7          -> out speaker PWM
 *
 */

#define SAMPLE_RATE 8000 // playback rate, hz
#define CB_RATE     10   // callback rate, hz

// leds state
static uint8_t s_leds_layer;
static uint16_t	s_leds_state;
static uint16_t s_leds_mask;

// sampler and leds 16-bit timer
static void timer1_init()
{
	// Set CTC mode (Clear Timer on Compare Match) (p.109)
	// Have to set OCR1A *after*, otherwise it gets reset to 0!
	TCCR1B |= (1 << WGM12);

	// No prescaler (p.110)
	TCCR1B |= (1 << CS01);

	// Set the compare register (OCR1A).
	OCR1A = F_CPU / SAMPLE_RATE;

	// Enable Output Compare Match Interrupt when TCNT1 == OCR1A (p.112)
	TIMSK |= (1 << OCIE1A);
}

// set up Timer 2 to do pulse width modulation on the speaker pin
static void timer2_init()
{
	// Set fast PWM mode  (p.128)
	TCCR2 |= (1 << WGM21) | (1 << WGM20);

	// Do non-inverting PWM on pin OC2 (p.129)
	TCCR2 |= (1 << COM21);

	// No prescaler (p.130)
	TCCR2 |= (1 << CS20);

	// Set initial pulse width to the first sample.
	OCR2 = 0;
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
	 * PD7 -> out speaker PWM
	 */

	// 0..7 A pins as output
	DDRA |= 0b11111111;
	// 0..3 C pins as output
	DDRC |= 0b00001111;
	// 4..6 C pins as output
	DDRC |= 0b01110000;
	// PWM speaker pin as out
	DDRD = (1 << DDD7);

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

static void load_audio_sample()
{
	static uint16_t s_sample = 0;

	// read audio sample byte to PWM compare register
	OCR2 = pgm_read_byte(&s_samples[s_sample++ % sizeof(s_samples)]);

	// avoid overflow
	s_sample = s_sample % sizeof(s_samples);
}

// TIMER1 Output Compare Match Interrupt service routine
// works on SAMPLE_RATE
ISR(TIMER1_COMPA_vect)
{
	// global variable to count the number of overflows
	static uint16_t s_overflow = 0;

	// keep a track of number of overflows
	s_overflow++;

	// play audio
	load_audio_sample();

	// fire leds on every interrupt for power savings
	// i.e. do persistence of vision (pov) with frequent flicking
	fire_leds();

	// call desk callback
	if (s_overflow == SAMPLE_RATE / CB_RATE) {
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

	// init sampler and leds timer
	timer1_init();

	// init PWM speaker timer
	timer2_init();

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
