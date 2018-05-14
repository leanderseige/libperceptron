/****************************************************************

 perceptron.c - functionality to use perceptrons on the AVR chips

 will probably run on any *nix system

 version 0.0.5 (alpha)

 (c) 2003 Leander Seige - leander@seige.name

 Released under the terms of the GNU General Public License (GPL)

 ****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "perceptron.h"

const char *n_version(void) {
  return "perceptron 0.0.5 (alpha)";
}

/* init random generator */
void n_init (void) {
  unsigned int seed = 12143; /* fixme if you want */
  // srand(seed);
  srand(time(NULL));
  seed=rand();
}

/* general random helper function */
long double n_rand (void) {
  long double out;
  out=1.0-2.0*(((long double)rand())/((long double)RAND_MAX));
  return(out);
}

/* default activation function */
long double n_f_sigmoid (long double in) {
  long double out;
  out=1.0/(1.0+exp(-in));
  return(out);
}

struct n_net *n_build_net (long long int layers, long long int neurons[], long double momentum, long double learn, void *activation) {
  long long int x,y,z;
  struct n_net	*o_net;
  struct n_layer	**o_layers,*o_layer;
  struct n_neuron	**o_neurons,*o_neuron;

  if (!neurons) {
    /* fprintf(stderr,"PERCEPTRON.H: No neuron numbers given.\n"); */
    return (NULL);
  }

  if (layers<2) {
    /* fprintf(stderr,"PERCEPTRON.H: Not enough layers given.\n"); */
    return (NULL);
  }

  n_init();

  if (!activation) {
    activation=n_f_sigmoid;
  }

  if ((o_net=calloc(1,sizeof(struct n_net)))==NULL) {
    /* fprintf(stderr,"PERCEPTRON.H: Couldn't calloc() net structure.\n"); */
    return (NULL);
  }

  if ((o_layers=calloc(layers,sizeof(struct n_layer *)))==NULL) {
    /* fprintf(stderr,"PERCEPTRON.H: Couldn't calloc() layer list.\n"); */
    return (NULL);
  } else {
    o_net->mom=momentum;
    o_net->learn=learn;
    o_net->error=0;
    o_net->c_layers=layers;
    o_net->layers=o_layers;
    o_net->act_funct=activation;

      for(x=0;x<layers;x++) {
        if((o_layer=calloc(1,sizeof(struct n_layer)))==NULL) {
          /* fprintf(stderr,"PERCEPTRON.H: Couldn't calloc() layer structure.\n"); */
          return (NULL);
        } else {
          o_net->layers[x]=o_layer;
          o_layer->c_neurons=neurons[x];
          if((o_neurons=calloc(neurons[x],sizeof(struct n_neuron *)))==NULL) {
            /* fprintf(stderr,"PERCEPTRON.H: Couldn't calloc() neuron list.\n"); */
            return (NULL);
          } else {
            o_layer->neurons=o_neurons;
            o_layer->c_neurons=neurons[x];
            for(y=0;y<neurons[x];y++) {
              if((o_neuron=calloc(1,sizeof(struct n_neuron)))==NULL) {
                /* fprintf(stderr,"PERCEPTRON.H: Couldn't calloc() layer structure.\n"); */
                return (NULL);
              } else {
                o_layer->neurons[y]=o_neuron;
                o_neuron->out   =0;
                o_neuron->error =0;
                o_neuron->ts    =n_rand();
			    o_neuron->dts   =n_rand();
			    if(x>0)
			    {
				if((o_neuron->in=calloc(o_net->layers[x-1]->c_neurons,sizeof(long double)))==NULL)
				{
				    /* fprintf(stderr,"PERCEPTRON.H: Couldn't calloc() neurons input weights.\n"); */
    				    return (NULL);
				}
				else
				{
    				    if((o_neuron->din=calloc(o_net->layers[x-1]->c_neurons,sizeof(long double)))==NULL)
				    {
					/* fprintf(stderr,"PERCEPTRON.H: Couldn't calloc() neurons delta input weights.\n"); */
    					return (NULL);
				    }
				    else
				    {
					for(z=0;z<(o_net->layers[x-1]->c_neurons);z++)
					{
					    o_neuron->in[z]=n_rand();
					    o_neuron->din[z]=n_rand();
					}
				    }
				}
			    }
			    else
			    {
				o_neuron->in=NULL;
				o_neuron->din=NULL;
			    }

			}
		    }
		}
	    }
	}
    }

    return(o_net);
}

extern void n_add_in_neuron(struct n_net *net) {
  struct n_layer	*layer;
  struct n_neuron **neurons;
  struct n_neuron *new_neuron;
  long double *tww, *twd;
  long long int ns, x;

  layer = net->layers[0];
  ns = layer->c_neurons;

  new_neuron=calloc(1,sizeof(struct n_neuron));
  new_neuron->out   =0;
  new_neuron->error =0;
  new_neuron->ts    =n_rand();

  neurons=calloc(ns+1,sizeof(struct n_neuron *));

  memcpy(neurons,layer->neurons,sizeof(struct n_neuron *));
  neurons[ns]=new_neuron;

  ns+=1;

  free(layer->neurons);
  layer->neurons=neurons;

  layer=net->layers[1];
  for(x=0;x<layer->c_neurons;x++) {
    tww=calloc(ns,sizeof(long double));
    twd=calloc(ns,sizeof(long double));
    memcpy(tww,layer->neurons[x]->in,sizeof(long double)*(ns-1));
    memcpy(twd,layer->neurons[x]->din,sizeof(long double)*(ns-1));
    tww[ns-1]=n_rand();
    twd[ns-1]=n_rand();
    free(layer->neurons[x]->in);
    free(layer->neurons[x]->din);
    layer->neurons[x]->in=tww;
    layer->neurons[x]->in=twd;
  }

}


void n_dump_net(struct n_net *net) {
  long long int x,y,z;

  fprintf(stderr,"\n");
  for(x=0;x<(net->c_layers);x++) {
    for(y=0;y<(net->layers[x]->c_neurons);y++) {
      fprintf(stderr,"Layer:%3lld Neuron:%3lld TS:%+.4Lf dTS:%+.4Lf OUT:%+.4Lf\n",x,y,net->layers[x]->neurons[y]->ts,net->layers[x]->neurons[y]->ts,net->layers[x]->neurons[y]->out);
      if(x>0) {
        for(z=0;z<(net->layers[x-1]->c_neurons);z++) {
          fprintf(stderr,"   %+.4Lf",net->layers[x]->neurons[y]->in[z]);
        }
      }
      fprintf(stderr,"\n");
    }
    fprintf(stderr,"\n");
  }
}

void n_prop (struct n_net *net, long double input[], long double output[]) {
  long long int x,y,z;
  long double temp;

  /* set input values into the net */
  for(x=0;x<(net->layers[0]->c_neurons);x++)
    net->layers[0]->neurons[x]->out=input[x];

  /* forward propagate */
  for(x=1;x<(net->c_layers);x++) {
    for(y=0;y<(net->layers[x]->c_neurons);y++) {
      temp=0;
      for(z=0;z<(net->layers[x-1]->c_neurons);z++) {
	       temp+=
          (net->layers[x]->neurons[y]->in[z])*
          (net->layers[x-1]->neurons[z]->out);
      }
      temp-=net->layers[x]->neurons[y]->ts;
      temp=(net->act_funct)(temp);
      net->layers[x]->neurons[y]->out=temp;
    }
  }

    /* set output values */
    for(x=0;x<(net->layers[(net->c_layers)-1]->c_neurons);x++)
    {
	output[x]=net->layers[(net->c_layers)-1]->neurons[x]->out;
    }
}

void n_backprop (struct n_net *net, long double wanted[])
{
    long long int x,y,z;
    long double temp;

    /* compute error values */

    net->error=0;
    for(x=0;x<(net->layers[(net->c_layers)-1]->c_neurons);x++)
    {
	net->layers[(net->c_layers)-1]->neurons[x]->error=
	    (		net->layers[(net->c_layers)-1]->neurons[x]->out)*
	    (1-		net->layers[(net->c_layers)-1]->neurons[x]->out)*
	    (wanted[x]-	net->layers[(net->c_layers)-1]->neurons[x]->out);
	net->error+=0.5*
	    (wanted[x]-	net->layers[(net->c_layers)-1]->neurons[x]->out)*
	    (wanted[x]-	net->layers[(net->c_layers)-1]->neurons[x]->out);
    }

    for(x=((net->c_layers)-2);x>0;x--)
    {
	for(y=0;y<(net->layers[x]->c_neurons);y++)
	{
	    temp=0;
	    for(z=0;z<(net->layers[x+1]->c_neurons);z++)
	    {
		temp+=(net->layers[x+1]->neurons[z]->error)*
			(net->layers[x+1]->neurons[z]->in[y]);
	    }
	    temp*=
		(	(net->layers[x]->neurons[y]->out))*
		(1-	(net->layers[x]->neurons[y]->out));
	    net->layers[x]->neurons[y]->error=temp;
	}
    }

    /* adjust weights */

    for(x=1;x<(net->c_layers);x++)
    {
	for(y=0;y<(net->layers[x]->c_neurons);y++)
	{
	    for(z=0;z<(net->layers[x-1]->c_neurons);z++)
	    {
		net->layers[x]->neurons[y]->in[z]+=
		    (net->layers[x-1]->neurons[z]->out)*
		    (net->layers[x]->neurons[y]->error)*(net->learn)+
		    (net->layers[x]->neurons[y]->din[z])*(net->mom);
		net->layers[x]->neurons[y]->din[z]=
		    (net->layers[x-1]->neurons[z]->out)*
		    (net->layers[x]->neurons[y]->error)*(net->learn);
	    }
	    net->layers[x]->neurons[y]->ts-=
		(net->layers[x]->neurons[y]->error)*(net->learn)+
		(net->layers[x]->neurons[y]->dts)*(net->mom);
	    net->layers[x]->neurons[y]->dts=
		(net->layers[x]->neurons[y]->error)*(net->learn);

	}
    }
}
