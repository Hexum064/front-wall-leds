/*
 * t412-LED-test-driver.c
 *
 * Created: 2023-02-16 18:00:08
 * Author : Branden
 */ 

#define F_CPU 20000000U
#include <avr/io.h>
#include <util/delay.h>

#define USART0_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)
#define DELAY 3000
#define LEDS 100

volatile uint8_t test_mode = 0;

void clk_init()
{
	CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_OSC20M_gc;
	CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLB = 0;
}

void usart_init()
{
	PORTA.DIRSET = PIN6_bm;
	USART0_CTRLA = 0;
	USART0_CTRLB = USART_TXEN_bm;
	USART0_CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_SBMODE_1BIT_gc | USART_CHSIZE_8BIT_gc;
	USART0_BAUD = USART0_BAUD_RATE(115200);
}

void transmit_single_led(uint8_t r, uint8_t g, uint8_t b)
{
	while (!(USART0_STATUS & USART_DREIF_bm)){};
	USART0_TXDATAL = r;
	while (!(USART0_STATUS & USART_DREIF_bm)){};
	USART0_TXDATAL = g;
	while (!(USART0_STATUS & USART_DREIF_bm)){};
	USART0_TXDATAL = b;
}

void transmit_led_count()
{
	//high byte first
	while (!(USART0_STATUS & USART_DREIF_bm)){};
	USART0_TXDATAL = LEDS >> 8;
	
	//then low byte
	while (!(USART0_STATUS & USART_DREIF_bm)){};
	USART0_TXDATAL = LEDS;		
}

void transmit_single_color(uint8_t r, uint8_t g, uint8_t b)
{
	transmit_led_count();
	for (uint16_t led_index = 0; led_index < LEDS; led_index++)
	{
		transmit_single_led(r, g, b);
	}	
}

//transmit all red
void transmit_test_1()
{
	transmit_single_color(255, 0, 0);	
}

//transmit all green
void transmit_test_2()
{
	transmit_single_color(0, 255, 0);
}

//transmit all blue
void transmit_test_3()
{
	transmit_single_color(0, 0, 255);
}

//transmit repeating red, green, blue 
void transmit_test_4()
{
	
	transmit_led_count();
	
	for (uint16_t led_index = 0; led_index < LEDS; led_index++)
	{
		if (led_index % 3 == 0)
		{
			transmit_single_led(255, 0, 0);
		}
		else if (led_index % 3 == 1)
		{
			transmit_single_led(0, 255, 0);
		}
		else
		{
			transmit_single_led(0, 0, 255);
		}
	}	
	
}

void transmit_normal_color()
{
	transmit_single_color(255, 142, 33); //2100K
}

void (*test_table[4])(void) = { transmit_test_1, transmit_test_2, transmit_test_3, transmit_test_4};

void generate_test_data()
{
	if (PORTA.IN & PIN1_bm)
	{		
		(*test_table[test_mode & 0x03])();
		test_mode++;
	}
	else 
	{
		transmit_normal_color();
	}
}

int main(void)
{
	PORTA.DIRSET = PIN7_bm;
	PORTA.OUTCLR = PIN7_bm;
	PORTA.DIRCLR = PIN1_bm;
	PORTA.PIN1CTRL = PORT_PULLUPEN_bm;
	clk_init();
	usart_init();
	
	
	
    /* Replace with your application code */
    while (1) 
    {
		PORTA.OUTTGL = PIN7_bm;
		generate_test_data();
		_delay_ms(DELAY);
		
    }
}

