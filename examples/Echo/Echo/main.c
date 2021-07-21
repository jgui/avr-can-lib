// coding: utf-8

/*
To run this example use an ATmega88A with MCP2515. CAN bus rate is 500kbs.

After reset the program:
- send a frame with ID 0x123456 (one time)
- then echo any received frame with the same data and an ID equal to received ID+10

MCP2515 is clocked using 16MHz cristal.
ATmega88A clock shall be 8MHz as defined in global.h.
This may be either by internal RC oscillator at 8MHz, or external cristal, or external clock (*). Fuse shall be set accordingly.
(*) MCP2515 is configure (via canconh.h) so CLKOUT frequency is 8MHz (prescaler = 2), so can be used as external clock.

SPI Chip Select pin is configured as PB2 (via canconh.h).

PC3 is used to control a LED for debug purpose. It is not necessary to connect a LED to run the example, just let the pin unconnected.
*/

#include "global.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "can.h"

// -----------------------------------------------------------------------------
/** Set filters and masks.
 *
 * The filters are divided in two groups:
 *
 * Group 0: Filter 0 and 1 with corresponding mask 0.
 * Group 1: Filter 2, 3, 4 and 5 with corresponding mask 1.
 *
 * If a group mask is set to 0, the group will receive all messages.
 *
 * If you want to receive ONLY 11 bit identifiers, set your filters
 * and masks as follows:
 *
 *	uint8_t can_filter[] PROGMEM = {
 *		// Group 0
 *		MCP2515_FILTER(0),				// Filter 0
 *		MCP2515_FILTER(0),				// Filter 1
 *		
 *		// Group 1
 *		MCP2515_FILTER(0),				// Filter 2
 *		MCP2515_FILTER(0),				// Filter 3
 *		MCP2515_FILTER(0),				// Filter 4
 *		MCP2515_FILTER(0),				// Filter 5
 *		
 *		MCP2515_FILTER(0),				// Mask 0 (for group 0)
 *		MCP2515_FILTER(0),				// Mask 1 (for group 1)
 *	};
 *
 *
 * If you want to receive ONLY 29 bit identifiers, set your filters
 * and masks as follows:
 *
 * \code
 *	uint8_t can_filter[] PROGMEM = {
 *		// Group 0
 *		MCP2515_FILTER_EXTENDED(0),		// Filter 0
 *		MCP2515_FILTER_EXTENDED(0),		// Filter 1
 *		
 *		// Group 1
 *		MCP2515_FILTER_EXTENDED(0),		// Filter 2
 *		MCP2515_FILTER_EXTENDED(0),		// Filter 3
 *		MCP2515_FILTER_EXTENDED(0),		// Filter 4
 *		MCP2515_FILTER_EXTENDED(0),		// Filter 5
 *		
 *		MCP2515_FILTER_EXTENDED(0),		// Mask 0 (for group 0)
 *		MCP2515_FILTER_EXTENDED(0),		// Mask 1 (for group 1)
 *	};
 * \endcode
 *
 * If you want to receive both 11 and 29 bit identifiers, set your filters
 * and masks as follows:
 */
const uint8_t can_filter[] PROGMEM = 
{
	// Group 0
	MCP2515_FILTER(0),				// Filter 0
	MCP2515_FILTER(0),				// Filter 1
	
	// Group 1
	MCP2515_FILTER_EXTENDED(0),		// Filter 2
	MCP2515_FILTER_EXTENDED(0),		// Filter 3
	MCP2515_FILTER_EXTENDED(0),		// Filter 4
	MCP2515_FILTER_EXTENDED(0),		// Filter 5
	
	MCP2515_FILTER(0),				// Mask 0 (for group 0)
	MCP2515_FILTER_EXTENDED(0),		// Mask 1 (for group 1)
};
// You can receive 11 bit identifiers with either group 0 or 1.



// LED connected to PC3 for debug:
#define LedOn()  (PORTC |=  (1 << PC3))
#define LedOff() (PORTC &= ~(1 << PC3))
#define LedIsOn()   (PINC & (1 << PC3))
void LedToggle(void) {
	if (LedIsOn()) {
		LedOff();
	} else {
		LedOn();
	}
}



// -----------------------------------------------------------------------------
// Main loop for receiving and sending messages.

int main(void)
{
	
	// Setup ports:

	/*
	// Set pull-ups or outputs high
	PORTB = (1<<PB7)|(1<<PB6)|(1<<PB5)|(1<<PB4)|(1<<PB3)|(1<<PB2)|(0<<PB1)|(1<<PB0);
	PORTC =          (1<<PC6)|(0<<PC5)|(1<<PC4)|(1<<PC3)|(1<<PC2)|(1<<PC1)|(1<<PC0);
	PORTD = (1<<PD7)|(1<<PD6)|(1<<PD5)|(1<<PD4)|(1<<PD3)|(1<<PD2)|(1<<PD1)|(1<<PD0);
	
	// Set directions for port pins
	DDRB  = (0<<PB7)|(0<<PB6)|(0<<PB5)|(0<<PB4)|(0<<PB3)|(0<<PB2)|(1<<PB1)|(0<<PB0);
	DDRC  =          (0<<PC6)|(0<<PC5)|(0<<PC4)|(1<<PC3)|(0<<PC2)|(0<<PC1)|(0<<PC0);
	DDRD  = (0<<PD7)|(0<<PD6)|(0<<PD5)|(0<<PD4)|(0<<PD3)|(0<<PD2)|(0<<PD1)|(0<<PD0);
	*/
	
	DDRC |= (1 << PC3); // set LED pin as output
	
	
	
	// Toggle LED to control good operation
	LedOn();
	_delay_ms(500);
	LedOff();
	_delay_ms(500);
	LedOn();
	_delay_ms(500);
	
	
	// Initialize MCP2515
	can_init(BITRATE_500_KBPS);
	
	
	// Toggle LED to control good operation
	LedOff();
	_delay_ms(500);
	LedOn();
	_delay_ms(500);
	LedOff();
	_delay_ms(500);
	LedOn();
	_delay_ms(500);
	
	
	// Load filters and masks
	can_static_filter(can_filter);
	// Note: if the program gets stuck at this point, this most probably mean that 
	// MCP2515 do not enter into configuration mode as requested. One possible cause
	// is a broken or not mounted or not terminated (120Ohm) transceiver.
	
	
	// Create a test message
	can_t msg;
	
	msg.id = 0x123456;
	msg.flags.rtr = 0;
	msg.flags.extended = 1;
	
	msg.length = 4;
	msg.data[0] = 0xde;
	msg.data[1] = 0xad;
	msg.data[2] = 0xbe;
	msg.data[3] = 0xef;
	
	// Send the message
	can_send_message(&msg);
	
	
	// Clear LED to check is initialization is passed
	LedOff();	

	
	while (1) // main loop
	{
		// Check if a new message has been received
		if (can_check_message())
		{
			can_t msg;
			
			// Try to read the message
			if (can_get_message(&msg))
			{
				// If we received a message, resend it with a different id
				msg.id += 10;
				
				// Send the new message
				can_send_message(&msg);
			}
		}
	}
	
	return 0;
}


