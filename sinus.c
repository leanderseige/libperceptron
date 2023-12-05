/*

    audible sinus audio demo for libperceptron 
    
    (c) 2023 by Leander Seige, leander@seige.name

    Released under the terms of the GNU General Public License (GPL)

    compile with: gcc  sinus.c -o sinus -lSDL2 -lperceptron -L. # -lm on Linux
    make sure necessary libraries are installed incl. their respective devel packages
    tested under Mac OSX and Linux

*/

#include "SDL2/SDL.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "perceptron.h"

long double i[1]={0.0}; /* array of input values */
long double o[1]={0.0}; /* array of output values */
long double w[1]={0.0}; /* array of wanted values */
long double sx,sy,nx,ny;
long long int nn[]={1,14,14,1}; /* definition of the net to be build */
long long int nnn = 4;
struct n_net *mynet;

static SDL_AudioDeviceID audio_device = 0;
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

#if defined(__GNUC__) || defined(__clang__)
static void panic_and_abort(const char *title, const char *text) __attribute__((noreturn));
#endif

static void panic_and_abort(const char *title, const char *text)
{
    fprintf(stderr, "PANIC: %s ... %s\n", title, text);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, text, window);
    SDL_Quit();
    exit(1);
}

static Uint8 *wavbuf = NULL;
static Uint32 wavlen = 0;
static SDL_AudioSpec wavspec;
static SDL_AudioStream *stream = NULL;

float sincyc = 0;

int SWIDTH = 640;
int SHEIGHT = 480;

static float converted_buffer[4096];
FILE *write_ptr;

double cyclecount = 0;

void lerncycle() {
    nx=(((double)rand())/((double)RAND_MAX));	/* 0..1 */

    
    double r = (((double)rand())/((double)RAND_MAX));
    double c = cyclecount/1000000;
    c = c>1.0 ? 1.0 : c;

    if(c<1.0) {
        if(r>0.6) {
            nx = nx/10 + .95;
        } else if(r<0.4) {
            nx = 1 - nx;
            nx = nx/10 + .95;
            nx = 1 - nx;
        }
    }

    // nx = (1+sin(nx*M_PI));

    #ifndef M_PI
    #    define M_PI 3.14159265358979323846
    #endif


    sx=nx*M_PI*12;                      		/* 0..2PI */
    sy=sin(sx);      				/* sin(0..2PI) */
    ny=(sy/2.0)+.5;  				/* the sin() scaled to 0..1 */
    i[0]=nx;					/* set input of nn */
    w[0]=ny;					/* set wanted output for learning */

    n_prop(mynet,i,o);				/* forward propagation (computation) */
    // printf("%.3Lf %.3Lf %.3Lf %.3Lf %.3Lf\n",nx,sx,ny,sy,o[0]);

    n_backprop(mynet,w);				/* backpropagation (learning) */
  
    cyclecount+=1;       
}

int main(int argc, char **argv)
{

    mynet = n_build_net (nnn,nn,0.2,0.4,NULL);
    SDL_Rect neuron;
    neuron.x=0;
    neuron.y=0;
    neuron.w=10;
    neuron.h=10;

    SDL_AudioSpec desired;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1) {
        panic_and_abort("SDL_Init failed", SDL_GetError());
    }

    // Get information about the current display mode
    SDL_DisplayMode currentDisplayMode;
    if (SDL_GetCurrentDisplayMode(0, &currentDisplayMode) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_GetCurrentDisplayMode failed: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // Set the window size to the current screen resolution
    SWIDTH = currentDisplayMode.w;
    SHEIGHT = currentDisplayMode.h;
    int YSINUS = SHEIGHT/2;
    int YNEURONS = SHEIGHT*2/3;

    window = SDL_CreateWindow("Neural SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SWIDTH, SHEIGHT, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!window) {
        panic_and_abort("SDL_CreateWindow failed", SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        panic_and_abort("SDL_CreateRenderer failed", SDL_GetError());
    }

    SDL_zero(desired);
    desired.freq = 48000;
    desired.format = AUDIO_F32;
    desired.channels = 1;
    desired.samples = 4096;
    desired.callback = NULL;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &desired, NULL, 0);
    if (audio_device == 0) {
        panic_and_abort("Couldn't audio device!", SDL_GetError());
    }
    SDL_PauseAudioDevice(audio_device, 0);

    SDL_bool paused = SDL_TRUE;

    write_ptr = fopen("outsound.raw","wb"); 

    SDL_bool keep_going = SDL_TRUE;

    while (keep_going) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    keep_going = SDL_FALSE;
                    break;

                case SDL_KEYDOWN:
                    switch( e.key.keysym.sym ){
                        case SDLK_SPACE:
                            cyclecount=0;
                            mynet = n_build_net (nnn,nn,0.2,0.4,NULL);
                            // SDL_PauseAudioDevice(audio_device, paused);
                            break;
                        case SDLK_ESCAPE:
                            keep_going = SDL_FALSE;
                            break;
                        default:
                            break;
                    }
            }
        }

        /* for(int x=0;x<1;x++) {
            lerncycle();
        } */

        if (SDL_GetQueuedAudioSize(audio_device) < 8192) {
            // const int bytes_remaining = SDL_AudioStreamAvailable(stream);
            // if (bytes_remaining > 0) {
                // const int new_bytes = SDL_min(bytes_remaining, 32 * 4096);
                // static Uint8 converted_buffer[32 * 4096];
                for(int x=0;x<2048;x++) {

                    /*
                    sincyc=sincyc+.02;
                    if(sincyc>2*M_PI) {
                        sincyc=sincyc-2*M_PI;
                    }
                    converted_buffer[x]=sin(sincyc);
                    */

                    lerncycle();

                    i[0]=((long double)x)/2048.0;
                    n_prop(mynet,i,o);
                    converted_buffer[x]=2*(o[0]-.5);
                    converted_buffer[x+2048]=converted_buffer[x];
                }
                
                // SDL_AudioStreamGet(stream, converted_buffer, new_bytes);  // !!! FIXME: error checking
                SDL_QueueAudio(audio_device, converted_buffer, 4096*4);
                fwrite(converted_buffer,4096*4,1,write_ptr);
            // }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 128, 0, 255, 255);
        for(int x=1;x<SWIDTH;x++) {
            SDL_RenderDrawLine(renderer,
                x-1,
                YSINUS/2+(YSINUS/4)*sin(((double)(x-1)/(double)(SWIDTH)*M_PI*12)),
                x,
                YSINUS/2+(YSINUS/4)*sin(((double)(x)/(double)(SWIDTH)*M_PI*12))
            );
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for(int x=1;x<2048;x++) {
            SDL_RenderDrawLine(renderer, floor(((x-1)*SWIDTH)/2048), YSINUS/2+converted_buffer[x-1]*YSINUS/4, ceil(((x-1)*SWIDTH)/2048), YSINUS/2+converted_buffer[x]*YSINUS/4);
        }


        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        long long int nx,ny,nz;
        long long int maxn=8; // better calc youself!! FIXME
        long long int rdy=40;
        long long int rdx=240;
        long long int nss=20;
        neuron.w=neuron.h=nss;

        fprintf(stderr,"\n");
        for(nx=0;nx<(mynet->c_layers);nx++) {
            for(ny=0;ny<(mynet->layers[nx]->c_neurons);ny++) {
            // fprintf(stderr,"Layer:%3lld Neuron:%3lld TS:%+.4Lf dTS:%+.4Lf OUT:%+.4Lf\n",nx,ny,mynet->layers[nx]->neurons[ny]->ts,mynet->layers[nx]->neurons[ny]->ts,mynet->layers[nx]->neurons[ny]->out);
            neuron.x = nx*rdx + SWIDTH/2 - ((mynet->c_layers-1)*rdx)/2;
            neuron.y = YNEURONS -  ((mynet->layers[nx]->c_neurons)*rdy)/2 + ny*rdy;
            int e = 255*mynet->layers[nx]->neurons[ny]->error;
            if(nx>0) {
                for(nz=0;nz<(mynet->layers[nx-1]->c_neurons);nz++) {
                    // fprintf(stderr,"   %+.4Lf",mynet->layers[nx]->neurons[ny]->in[nz]);
                    int c = 255*mynet->layers[nx]->neurons[ny]->in[nz];
                    SDL_SetRenderDrawColor(renderer, 255-c, c, 255, 255);
                    SDL_RenderDrawLine(renderer,
                        neuron.x+nss/2,
                        neuron.y+nss/2,
                        (nx-1)*rdx+nss/2 + SWIDTH/2-((mynet->c_layers-1)*rdx)/2,
                        YNEURONS -  ((mynet->layers[nx-1]->c_neurons)*rdy)/2 + nz*rdy + nss/2);
                }
            }
            SDL_SetRenderDrawColor(renderer, 255, 255-e, 255-e, 255);
            SDL_RenderFillRect(renderer,&neuron);
            // fprintf(stderr,"\n");
            }
            // fprintf(stderr,"\n");
        }

        SDL_RenderPresent(renderer);
        printf("cyclecount: %d\n",(int)cyclecount);
    }

    SDL_CloseAudioDevice(audio_device);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

