/////////////////////////////////////////////////////
/// Friction drum
/// UL FRI - LGM project
/////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <unistd.h>
#include <math.h>
#include <pthread.h>

#include "wiringPi.h"
#include "mcp3004.h"

#include "include/MPR121.h"
#include <soundtouch/SoundTouch.h>
#include "SDL2/SDL.h"

#define MPR121_NUM_INPUTS 12    // number of inputs on MPR121 used
#define HIST_INP_SAMPLE 10      // number of samples used for history of touches
#define HIST_INP_HOLD_SAMPLE 20 // number of samples used for hold detection
#define MICRO_S_SLEEP 25000     // micro seconds of sleep between samples

#define ST_BUFFER_SIZE 44100 	// soundtouch buffer size

uint16_t touched = 0; //global bitwise status of currently active inputs
uint16_t joystick_x = 0; //global position of joystick x direction -> between 1-1023
uint16_t joystick_y = 0; //global position of joystick y direction -> between 1-1023

int global_current_gesture; //global number of current gesture (silence, up, down, tap)
Uint8 *audio_pos; //current position in audio buffer
Uint32 audio_len; //length (in bytes) of the audio buffer

using namespace soundtouch;
using namespace std;

//initialize MPR121
MPR121 sense_initialize(){
	cout << " Initializing wiringPi and MCP3008 headers" << endl;
	wiringPiSetup();
	mcp3004Setup(100,0);
	
	cout << " Initializing MPR121" << endl;
    MPR121 sensor = MPR121();
    sensor.initialize();

    if(sensor.testConnection())
    {
        cout << " MPR121 OK,   \n Starting sense thread!" << endl;
    }
    else
    {
        cout << " MPR121 not OK,\nStopping sense thread!" << endl;
        cout << " There was and issue reading correct value from "
			"one of the registers (should be 0x04)" << endl << endl;
		pthread_exit(NULL);
    }
    return sensor;
}

//MPR121 thread updating status of touched pins
void *sense_thread(void *threadid)
{
	MPR121 sensor = sense_initialize();
    while(true)
    {
		//analogRead(100); //read joystick pressed status
		joystick_x = analogRead(101); //read joystick x pitch
		joystick_y = analogRead(102); //read joystick y pitch
        touched = sensor.getTouchStatus(); //read MPR
        
        //cout << "X: " <<joystick_x<< " Y: " << joystick_y << " t: " << touched << endl;
        usleep(MICRO_S_SLEEP);
    }
    cout << " Sense thread dying!" << endl;
    pthread_exit(NULL);
}
	
//SDL audio callback - plays wave found at audio_pos 
void my_audio_callback(void *userdata, Uint8 *stream, int len){
	// only play if there is some data left
	if(audio_len == 0)
		return;
		
	// mix as much data as possible	
	len = (len > (int) audio_len ? (int) audio_len : len);	
	
	SDL_memcpy(stream, audio_pos, len); 	//substitute stream
	//SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME); //mix with existing stream

	audio_pos += len;
	audio_len -= len;
}

//initialize SDL and load wav files
void produce_initialize(Uint32* sound_len, Uint8** sound_buf, SDL_AudioSpec* audio_spec){
	cout << "  Reading and initializing media files." << endl;
	char const *file_up = "C.wav"; //what to play on gesture up
	char const *file_down = "D.wav"; //what to play on gesture down
	char const *file_tap = "E.wav"; //what to play on top tap 

	//initialize SDL_AUDIO
	if(SDL_Init(SDL_INIT_AUDIO) < 0){
		cout << "  SDL NOT INITIALIZED!\n" << SDL_GetError()  << endl;
		pthread_exit(NULL); 
	}
	
	//load wav files
	if(SDL_LoadWAV(file_up,audio_spec,&sound_buf[1],&sound_len[1]) == NULL ||
	SDL_LoadWAV(file_down,audio_spec,&sound_buf[2],&sound_len[2]) == NULL ||
	SDL_LoadWAV(file_tap,audio_spec,&sound_buf[3],&sound_len[3]) == NULL)
	{
		cout << "  ERROR OPENING FILES!\n" << SDL_GetError() << endl;
		pthread_exit(NULL); //exit (main thread handles lack of sound!)
	}

	sound_len[0] = sound_len[1]; //define silence sound (fill with 0's)
	sound_buf[0] = (Uint8 *)calloc(sound_len[1], sizeof(Uint8));

	//set spec file
	(*audio_spec).callback = my_audio_callback;
	(*audio_spec).userdata = NULL;

}

//check current events and play corresponding sound
void *produce_thread(void *threadid)
{
	SDL_AudioSpec audio_spec; //spec file used to get audio info
	Uint32 sound_len[4]; // silence, up, down, tap in order of array
	Uint8 **sound_buf = (Uint8 **)calloc(4, sizeof(Uint8 *));

	produce_initialize(sound_len, sound_buf, &audio_spec);

	//start playing flat wave
	audio_pos = sound_buf[0];
	audio_len = sound_len[0];

	//open the audio device
	if(SDL_OpenAudio(&audio_spec, NULL) <  0)
	{
		cout << "  FAILED OPENING AUDIO DEVICE!\n" << SDL_GetError() << endl ;
		pthread_exit(NULL);
	}
	
	//let the callback function play the audio chunk
	SDL_PauseAudio(0);


	//samples have to have 2 channels and 44100Hz samplerate!
	Uint8 soundtouch_buffer[ST_BUFFER_SIZE];
	Uint32 soundtouch_len = ST_BUFFER_SIZE / 2;
	SoundTouch stouch;
	stouch.setSampleRate((int)44100);
	stouch.setChannels((int)2);
	int nSamples = 0;
	
	//adapt currently playing sample
	int prev_gesture = 0;
	int current_gesture = 0;
	while(true) 
	{
		while(audio_len > 0)
		{
			current_gesture = global_current_gesture;
			
			if(prev_gesture != current_gesture)
			{
				float pitch = sqrt(pow((double)joystick_x, 2.0) + pow((double)joystick_y, 2.0));
				pitch =10.0*(pitch/512.0); //0.0-1.99 -1
				
				//TODO: USE SOUNDTOUCH TO CORRECT PITCH HERE
				stouch.setPitchSemiTones(pitch);
				stouch.putSamples((float*)sound_buf[current_gesture],(uint)sound_len[current_gesture]);
				nSamples = stouch.receiveSamples((float*)soundtouch_buffer,(uint)soundtouch_len);
								
				SDL_LockAudio();
				audio_len = nSamples;
				audio_pos = soundtouch_buffer;
				SDL_UnlockAudio();

				prev_gesture = current_gesture;
			}
			usleep(MICRO_S_SLEEP /2);
		}

		//in case sample ends, continue with flat wave
		SDL_LockAudio();
		audio_len = sound_len[0];
		audio_pos = sound_buf[0];
		SDL_UnlockAudio();

	}

	cout << "  Produce thread dying!" << endl;
	SDL_CloseAudio();
	
	for (int i = 0; i < 4; i++) 
		SDL_FreeWAV(sound_buf[i]);
	
    pthread_exit(NULL);
}

//return number of detected gesture (0: silence, 1: up, 2: down, 3: tap)
int detectGesture()
{
	static double hist_avg_sample[HIST_INP_SAMPLE]; //remember history
    double avg = 0.0;

    //calculate avg vector of previous samples
    for(int i = 1; i < HIST_INP_SAMPLE; i++)
    {
        avg += hist_avg_sample[i-1] - hist_avg_sample[i];
        hist_avg_sample[i-1] = hist_avg_sample[i];
    }

	//add value with new sample
    double new_avg = 0.00;
    uint16_t mask = 1;
    int pins_touched = 0;
	int i = 0;
	
    for(i = 0; i < MPR121_NUM_INPUTS; i++)
    {
        if(touched & mask)
        {
            pins_touched++;
            new_avg += (double)i;
        }
        mask <<= 1;
    }

	new_avg = (double)new_avg / pins_touched;
	avg += hist_avg_sample[HIST_INP_SAMPLE -1] - new_avg;
	hist_avg_sample[HIST_INP_SAMPLE-1] = new_avg;
	
	//detect tap
	bool tap = false;
	mask = 1;
	for (i = 0; i < MPR121_NUM_INPUTS; i++)
	{
		if (i == 0 && touched & mask) {
			tap = true;
		} else if (i != 0 && touched & mask) {
			tap = false;
		}
		mask <<= 1;
	}
	
    if(avg < 0.0) {
        cout << "1" << endl;
        return 1;
    }
    else if(avg > 0.0) {
        cout << "2" << endl;
        return 2;
    }
	else if (tap) {
		cout << "Tap!" << endl;
		return 3;
	}
    else
        return 0;
}

// update history to new value from inputs
void update_hist(uint16_t *hist) 
{
	int i = 1;
    for(i = 1; i < HIST_INP_HOLD_SAMPLE; i++)
        hist[i-1] = hist[i];
    
    hist[HIST_INP_HOLD_SAMPLE -1] = touched;
}

//detect when stick is held - if any pin is pressed >= HIST_INP_HOLD_SAMPLE
bool detectHold() 
{
	static uint16_t hist[HIST_INP_HOLD_SAMPLE] = {0};
    bool hold = true;
    uint16_t mask;
    update_hist(hist);
	int i = 0, j = 0;

    for(j = 0; j < MPR121_NUM_INPUTS; j++)
    {
        hold = true;
        mask = 1 << j;
        for(i = 0; i < HIST_INP_HOLD_SAMPLE; i++)
        {
            if(0 == (hist[i] &&mask))
            {
                hold = false;
                break;
            }
        }
        if(hold)
        {
            break;
        }
    }
    
    return hold;
}

int main()
{
    int re1 = 0, re2 = 0;
    
    //run threads for touch detection and sound generation
    pthread_t sense, produce;
    
    if(pthread_create(&sense, NULL, sense_thread, (void *)re1))
        cout << "Error: Could not start sense_thread!\n";

    if(pthread_create(&produce, NULL, produce_thread, (void *)re2))
		cout << "Error: Could not start produce_thread!\n";

    bool hold = false;
    
    while((pthread_kill(sense,0) == 0 ) && (pthread_kill(produce,0) == 0) )
    {
		hold = detectHold(); // detect if user is holding a stick
		
		if (!hold)
			global_current_gesture = detectGesture(); // get gesture number
		
        usleep(MICRO_S_SLEEP);
    }

	if(pthread_kill(sense,0) != 0 ){
		cout << "Sense thread died! Sending SigKill to produce thread!\nCheck output for what happened!" << endl;
		pthread_kill(produce,9);
	} else if(pthread_kill(produce,0) != 0 )
	{
		cout << "Produce thread died! Sending SigKill to sense thread!\nCheck output for what happened!" << endl;
		pthread_kill(sense,9);
	} else
	{
		cout << "Both threads are alive! Something went wrong. Check output or check for errors elsewhere!" << endl;
		cout << "Killing both threads and exiting!" << endl;
		pthread_kill(sense,9);
		pthread_kill(produce,9);
	}
    pthread_exit(NULL);
}
