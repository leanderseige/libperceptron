/****************************************************************

 example program for

 perceptron.h - functionality to use perceptrons on the AVR chips

 This program builds an artifical neural that learns sine waves.

 (c) 2003 Leander Seige - leander@seige.name

 Released under the terms of the GNU General Public License (GPL)

 This program learns the sine function and outputs its contents
 on PORTB (e.g. connected to LEDs). Tested with an Atmega16 at
 8 Mhz on the STK500 Dev-Board. After ~1h the neural net has a
 good approximation of sin(). The highlighted bit will smoothly
 wander from left to right and back then. In the beginning it will
 flicker around in the middle and later start to show a kind of
 linear shifting.

 Patterns:
 01010101 (alternating) - out of mem., the net could not be build
 00010000 (wandering)   - net shows its understanding of sin()

 ****************************************************************/

#include <avr/io.h>
#include <math.h>
#include "perceptron.h"

int main( void )
{
	unsigned char x,y,z,air;
	double i[1]={0.0}; /* array of input values */
	double o[1]={0.0}; /* array of output values */
	double w[1]={0.0}; /* array of wanted values */
	double sx,sy,nx,ny;
	unsigned char nn[]={1,6,6,1}; /* definition of the net to be build */
	struct n_net *mynet;

	outp(0xff,DDRB);						/* prepare PORTB for Output */

	mynet = n_build_net (4,nn,0.2,0.4,NULL);			/* build the neural net */

	if(!mynet)							/* output error pattern */
	{
		air=0xaa;
		for (;;)
		{
			air=~air;
			outp(air,PORTB);
			for(x=0;x<255;x++)
				for(y=0;y<255;y++)
					for(z=0;z<127;z++);
		}
	}

	x=0x00;
	for (;;) 						/* forever */
	{
		nx=(((double)rand())/((double)RAND_MAX));	/* 0..1 */
		sx=nx*M_PI*2;                      		/* 0..2PI */
		sy=sin(sx);      				/* sin(0..2PI) */
		ny=(sy/2.0)+.5;  				/* the sin() scaled to 0..1 */
		i[0]=nx;					/* set input of nn */
		w[0]=ny;					/* set wanted output for learning */

		n_prop(mynet,i,o);				/* forward propagation (computation) */
		n_backprop(mynet,w);				/* backpropagation (learning) */

		/* output leds */
		i[0]=((double) x)/((double)80);		/* 0..1 for scanning the net */
		n_prop(mynet,i,o);				/* get the output from the net */
		outp(~(0x01<<((int)(o[0]*7.0+.5))), PORTB);	/* output sine pattern, scale
								the 1..0 output to the 8 bits (leds) */
		x++; x%=80;                                    /* keep scanning the net always */
	}
}
