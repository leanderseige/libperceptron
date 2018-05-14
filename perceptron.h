/****************************************************************

 perceptron.h - functionality to use perceptrons on the AVR chips

 will probably run on any *nix system

 version 0.0.5 (alpha)

 (c) 2003 Leander Seige - leander@seige.name

 Released under the terms of the GNU General Public License (GPL)

 ****************************************************************/

#ifndef PERCEPTRON_H
#define PERCEPTRON_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>

struct n_neuron{
    long double              out;            /* output value */
    long double              error;          /* error value */
    long double              ts,dts;         /* threshold */
    long double              *in;            /* list of weights */
    long double              *din;           /* list of delta weights */
};

struct n_layer{
    long long int       c_neurons;      /* number of neurons in this layer */
    struct n_neuron     **neurons;      /* list of neurons */
};

struct n_net{
    long double               mom;            /* momentum */
    long double               learn;          /* learning rate*/
    long double               error;          /* net error */
    long long int             c_layers;       /* number of layers in this net */
    struct n_layer            **layers;       /* list of layers */
    long double               (*act_funct)(long double in);        /* activation function */
};

/* dynamically add input neurons */
extern void n_add_in_neuron(struct n_net *net);

/* initialization, only the random generator for now */
extern void n_init (void);

/* general random helper function */
extern long double n_rand (void);

/* default activation function */
extern long double n_f_sigmoid (long double in);

/*
build up one perceptron

layers		number of layers

neurons		define the number of neurons
		for each layer,
		for instance {2,4,3} would define
		two input neurons, four hidden neurons
		in one hidden layer and three neurons
		in the output layer

momentum	the momentum value

learn		the learning rate

activation	pointer to the activation function, if
		it is NULL, the n_f_sigmoid will be used
*/
extern struct n_net *n_build_net
	(long long int layers, long long int neurons[],
	long double momentum, long double learn, void *activation);

/*
dump the whole net (needs to be modified to print to a serial
line connection or something similar)
*/
extern void n_dump_net(struct n_net *net);

/*
forward propagate the net

input		array of input values

output		array which will contain the resulting output values
		when the function returns
*/
extern void n_prop (struct n_net *net, long double input[], long double output[]);

/*
backward propagate the net

wanted		array of the correct values, must match the number
		of output neurons
*/
extern void n_backprop (struct n_net *net, long double wanted[]);

extern const char *n_version (void);

#endif
