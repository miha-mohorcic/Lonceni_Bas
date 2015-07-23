/////////////////////////////////////////////////////
/// Friction drum
/// UL FRI - LGM project
/////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <unistd.h>

#include <pthread.h>
#include "include/MPR121.h"

#include "soundtouch/SoundTouch.h"

#include "SDL2/SDL.h"

#define MPR121_NUM_INPUTS 12    // number of inputs on MPR121 used
#define HIST_INP_SAMPLE 10      // number of samples used for history of touches
#define MICRO_S_SLEEP 35000     // micro seconds of sleep

#define DEBUG false

uint16_t touched = 0; // global bitwise status of currently active inputs

using namespace soundtouch;
using namespace std;

void *sense_thread(void *threadid)
{
    MPR121 senzor = MPR121(); //initalizing MPR121
    senzor.initialize();

    if (senzor.testConnection()) //testing MPR connection (checks register)
    {
        cout << "MPR121 OK starting program!" << endl;
    }
    else
    {
        cout << "MPR121 not OK closing program!" << endl;
        pthread_exit(NULL);
    }

    while(true) //constantly check input status and remember time from last change
    {
        touched = senzor.getTouchStatus();
        //if(DEBUG)
        //{
            //bool t;
            //uint16_t mask=1;
            //for(int i=0; i<MPR121_NUM_INPUTS; i++)
            //{
                //t = ( touched & mask ) > 0;
                //cout << t;
                //mask<<= 1;
            //}
            //cout << "\n";
        //}
        usleep(MICRO_S_SLEEP);
    }
    pthread_exit(NULL); //if for some reason we fall out of loop (graceful exit)
}

static Uint8 *audio_pos;
static Uint32 audio_len;
static int global_current_gesture;

void my_audio_callback(void *userdata, Uint8 *stream, int len){
	
	if(audio_len == 0)
		return;
	len = (len > audio_len ? audio_len : len );
	
	SDL_memcpy (stream, audio_pos, len); 	
	//SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	
	audio_pos += len;
	audio_len -= len;
}


void *produce_thread(void *threadid)
{
	//names of files used for sound
	char const *file_up = "C.wav"; //what to play on gesture up
	char const *file_down = "D.wav"; //what to play on gesture down
	char const *file_tap = "E.wav"; //what to play on top tap 
	
	//initialize SDL_AUDIO
	if(SDL_Init(SDL_INIT_AUDIO) < 0){
		cout << "SDL ISSUES WOOHOO" << endl;
		//TODO gracefully close application
	}
	
	SDL_AudioSpec audio_spec; //spec file used to get audio info
	
	Uint32 sound_len[4]; // silence, up, down, tap in order of array
	Uint8 **sound_buf = (Uint8 **)calloc(4, sizeof(Uint8 *));
	//Uint8 *sound_buf[4];
	
	//loading wav files
	if(	SDL_LoadWAV(file_up,&audio_spec,&sound_buf[1],&sound_len[1]) == NULL ||
	SDL_LoadWAV(file_down,&audio_spec,&sound_buf[2],&sound_len[2]) == NULL ||
	SDL_LoadWAV(file_tap,&audio_spec,&sound_buf[3],&sound_len[3]) == NULL )
	{
		cout << "FILE ISSUES, WOOHOO!";
		//TODO gracefully close application
	}
	
	sound_len[0] = sound_len[1]; //silence sound
	sound_buf[0] = (Uint8 *)calloc(sound_len[1], sizeof(Uint8));
	
	//set spec file
	audio_spec.callback = my_audio_callback;
	audio_spec.userdata = NULL;
	
	//begin playing "silence"
	audio_pos = sound_buf[0];
	audio_len = sound_len[0];

	if(  SDL_OpenAudio(&audio_spec, NULL) <  0)
	{
		cout << "AUDIO ISSUES!\n" << "\"" << SDL_GetError() << "\"" << endl ;
		//TODO gracefully close application
	}
		
	//start playing sound
	SDL_PauseAudio(0);
	
	int prev_gesture = 0;
	int current_gesture = 0;
	//TODO KJE SPREMINJAMO PITCH????
	while(true){
		while(audio_len > 0){
			
			current_gesture = global_current_gesture;
			//check change of events and switch pointers if necceseray	
			//pointer audio_len & audio_pos
			if(prev_gesture != current_gesture)
			{
				SDL_LockAudio();
				
				audio_len = sound_len[current_gesture];
				audio_pos = sound_buf[current_gesture];
				
				SDL_UnlockAudio();
				prev_gesture = current_gesture;
			}
			usleep(MICRO_S_SLEEP /2);
		}
		//cout << "PADU VEN!!!!!!" << endl ;
		// TODO  - poglej zakaj si sel iz prejsnjega while - popravi pointerje in nadaljuj	
		
		SDL_LockAudio();
		
		audio_len = sound_len[0];
		audio_pos = sound_buf[0];
		
		SDL_UnlockAudio();
		
		//SDL_PauseAudio(0);
	}
	
	SDL_CloseAudio();
	SDL_FreeWAV(sound_buf[0]);
	SDL_FreeWAV(sound_buf[1]);
	SDL_FreeWAV(sound_buf[2]);
	SDL_FreeWAV(sound_buf[3]);
    pthread_exit(NULL);
}

double hist_avg_sample[HIST_INP_SAMPLE];
//return # of detected gesture
int detectGesture()
{
    double avg = 0.0;
    for(int i=1; i<HIST_INP_SAMPLE; i++)
    {
        avg+=hist_avg_sample[i-1] - hist_avg_sample[i];
        hist_avg_sample[i-1] = hist_avg_sample[i];
    }

    double new_avg = 0.00;
    uint16_t mask=1;
    int pins_touched = 0;

    for (int i=0; i<MPR121_NUM_INPUTS; i++)
    {
        if(touched & mask)
        {
            pins_touched++;
            new_avg+=(double)i;
        }
        mask <<= 1;
    }
    //if(pins_touched > 0)  //avoid nan as result
    if(true)
    {
        new_avg = (double)new_avg/pins_touched;
        avg += hist_avg_sample[HIST_INP_SAMPLE -1] - new_avg;
        hist_avg_sample[HIST_INP_SAMPLE-1] = new_avg;
    }
    else
    {
        hist_avg_sample[HIST_INP_SAMPLE-1] = 0.0;
    }

    //if(DEBUG)
    //{
        //cout << " avg" << avg <<" ";
    //}

    if(avg<0.0)
        return 1;
    else if(avg>0.0)
        return 2;
    else
        return 0;
}

void update_hist(uint16_t *hist) // update history to new value from inputs
{
    for(int i=1; i<HIST_INP_SAMPLE; i++)
    {
        hist[i-1]=hist[i];
    }
    hist[HIST_INP_SAMPLE -1] = touched;
}

bool detectHold(uint16_t *hist) //detect when  stick is held
{
    bool hold = true;
    uint16_t mask;
    int i, j;

    update_hist(hist);

    for(j=0; j<MPR121_NUM_INPUTS; j++)
    {
        hold=true;
        mask = 1 << j;
        for(i=0; i<HIST_INP_SAMPLE; i++)
        {
            if(0 == (hist[i] & mask))
            {
                hold=false;
                break;
            }
        }
        if(hold)
        {
            break;
        }
    }
    //if (DEBUG)
    //{
        //cout << " hold" <<  hold << " ";
    //}
    return hold;
}


int main()
{
    int re1 = 0, re2 = 0;
    //runs threads for touch detection and sound generation
    pthread_t sense, produce;
    if(pthread_create(&sense, NULL, sense_thread, (void *)re1))
    {
        cout << "Error: Could not start sense_thread!\n" ;
    }

    if(pthread_create(&produce, NULL, produce_thread, (void *)re2))
    {
        cout << "Error: Could not start produce_thread!\n" ;
    }

    //Calculates vector of average movement
    uint16_t hist[HIST_INP_SAMPLE] = {0};
    for (int i = 0; i<HIST_INP_SAMPLE; i++)
    {
        hist_avg_sample[i]=0.0;
    }

    uint16_t gesture;
    uint16_t hold;

    while(true)
    {
        gesture = detectGesture(); // get gesture #
        global_current_gesture = gesture; //TODO TOO SIMPLIFIED!!!!
        
        hold = detectHold(hist); // detect if user is holding a stick

        // TODO: get info from joystick and generate suitable  pitch
        // TODO: playSound(gesture, pitch)

        // recognize gesture
        if(gesture == 1 && !hold)
        {
            //TODO: make sound
            cout << " Premik gor\n";
        }
        else if(gesture == 2 && !hold)
        {
            //TODO: make sound
            cout << " Premik dol\n";
        }
        else if(hold)
            cout << "Zaznavam drzanje!\n";


        usleep(MICRO_S_SLEEP);

    }

    pthread_exit(NULL);
}
