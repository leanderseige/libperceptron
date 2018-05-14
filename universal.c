/****************************************************************

 example program for

 perceptron.h - functionality to use perceptrons on the avr chips

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

#include <math.h>
#include "perceptron.h"
#include <unistd.h>

unsigned char map[5] = " -+*M";

unsigned char obits[8][17];
unsigned char ibits[8][17] = {
    "    MMMMMMMM    \0",
    "  MM        MM  \0",
    " M   M    M   M \0",
    "M              M\0",
    "M  MM      MM  M\0",
    " M   MMMMMM   M \0",
    "  MM        MM  \0",
    "    MMMMMMMM    \0"
};

long double mind[8][17];
long double lern[8][17];


int main( void ) {
  int x,y;
  long int c,r=0;
	long double i[2]={0.0,0.0}; /* array of input values */
	long double o[1]={0.0}; /* array of output values */
	long double w[1]={0.0}; /* array of wanted values */
  long double q=0.0;
	int sx,sy;
	long long int nn[]={2,48,48,1}; /* definition of the net to be build */
	struct n_net *mynet;

    for(x=0;x<16;x++) {
        for(y=0;y<8;y++) {
            if(ibits[y][x]=='M')
                lern[y][x]=1.0;
            else
                lern[y][x]=0.0;
            mind[y][y]=(((long double)rand())/((long double)RAND_MAX));
        }
    }

	mynet = n_build_net (4,nn,0.2,0.4,NULL);			/* build the neural net */

	if(!mynet)							/* output error pattern */
	{
		printf("major malfunction\n");
		exit(0);
	}

    printf("\n\n\n");

	for (;;) 						/* forever */
	{
        for(q=0.0,c=0;c<510;c++) {
		    i[0]=(((long double)rand())/((long double)RAND_MAX));	/* 0..1 */
		    i[1]=(((long double)rand())/((long double)RAND_MAX));	/* 0..1 */
		    sx=round(15*i[0]);					/* set input of nn */
	    	sy=round(7*i[1]);					/* set input of nn */
            w[0]=lern[sy][sx];		/* set wanted output for learning */

		    n_prop(mynet,i,o);				/* forward propagation (computation) */
            mind[sy][sx]=o[0];
            n_backprop(mynet,w);	        /* backpropagation (learning) */

            q+=fabsl(w[0]-o[0]);
		}

        r+=c;

		/* output */

        printf("-----image-----------state of mind-\n\n");

		for(y=0;y<8;y++) {
            for(x=0;x<16;x++) {
                obits[y][x]=map[(int)round(mind[y][x]*4.0)];
            }
			printf("%s   %s\n",ibits[y],obits[y]);
		}
        printf("\nlrn cycles : %ld",r);
        printf("\nerror rate : %.2Lf\n\n",q);

        // usleep(40000);
	}
}
