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
#define DELAY 1000
#define LEDS_PER_LIGHT 7
#define LIGHTS 10
#define LEDS (LEDS_PER_LIGHT * LIGHTS)

volatile uint8_t test_mode = 0;

void init_clk()
{
	CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_OSC20M_gc;
	CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLB = 0;
}

void init_usart()
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

void (*test_table[4])(void) = { transmit_test_1, transmit_test_2, transmit_test_3, transmit_test_4};

void generate_test_data()
{
	(*test_table[test_mode & 0x03])();
	test_mode++;
}


void transmit_one_light(uint8_t red, uint8_t green, uint8_t blue)
{
	for (uint16_t i = 0; i < LEDS_PER_LIGHT; i++)
	{
		transmit_single_led(red, green, blue);
	} 
}

volatile uint8_t offset = 0;

void generate_4thOfJuly_pattern()
{
	uint8_t step = offset;
	transmit_led_count();
	
	for (uint8_t i = 0; i < LIGHTS; i++)
	{
		switch(step)
		{
			case 0:
				transmit_one_light(128,0,0); //RED
				break;
			case 1:
				transmit_one_light(48,48,48); //WHITE
				break;
			case 2:
				transmit_one_light(0,0,128); //BLUE
				break;
		}
		
		step++;
		
		if (step > 2)
		{
			step = 0;
		}
	}
	
	offset++;
	
	if (offset > 2)
	{
		offset = 0;
	}
	
}

void generate_Halloween_pattern()
{

	transmit_led_count();
	
	for (uint8_t i = 0; i < LIGHTS; i++)
	{
        if ((i+offset)%2)
        {
            transmit_one_light(128,26,0); //ORANGE
        }
        else
        {
            transmit_one_light(8,64,0); //GREEN
        }
    }
    
    if (offset)
    {
        offset = 0;
    }
    else
    {
        offset = 1;
    }
}



void generate_Christmas_pattern()
{

	transmit_led_count();
	
	for (uint8_t i = 0; i < LIGHTS; i++)
	{
        if ((i+offset)%2)
        {
            transmit_one_light(128,0,0); //RED
        }
        else
        {
            transmit_one_light(0,128,0); //GREEN
        }
    }
    
    if (offset)
    {
        offset = 0;
    }
    else
    {
        offset = 1;
    }
}

void transmit_warm_white()
{
	transmit_single_color(255, 142, 33); //2100K
}

int main(void)
{
	uint8_t test = 1;
	PORTA.DIRSET = PIN1_bm;
	PORTA.OUTCLR = PIN1_bm;
	PORTA.DIRCLR = PIN7_bm;
	PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
	init_clk();
	init_usart();
	

	
	if ((PORTA.IN & PIN7_bm))
	{
		test = 0;
	}
	
    /* Replace with your application code */
    while (1) 
    {
		PORTA.OUTTGL = PIN1_bm;
		if ((PORTA.IN & PIN7_bm))
		{
			generate_4thOfJuly_pattern();
		}
		else
		{
			//generate_test_data();	
			transmit_warm_white();
		}
		
		_delay_ms(DELAY);
		
    }
}

