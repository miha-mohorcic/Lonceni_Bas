////////////////////////////
/// Friction Drum V0.2   ///
/// UL FRI - LGM Project ///
////////////////////////////

//includes
// standard IO
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
// threading
#include <pthread.h>
// joystick
#include "wiringPi.h"
#include "mcp3004.h"
// touch sensor
#include "include/MPR121.h"
// WAV reading
#include <sndfile.h>
// WAV pitching
#include <soundtouch/SoundTouch.h>
// WAV playing 
#include "SDL2/SDL.h"

//defines
#define DEBUG_INPUT true
#define USE_JOYSTICK true
#define USE_MPR121 true
#define TOUCH_INPUTS 8
#define MICRO_S_SLEEP_UPDATE 2000
#define MICRO_S_SLEEP_SOUND 1000000

//global variables
MPR121 sensor;
uint16_t touched = 0; //global bitwise status of currently active inputs
uint16_t joystick_x = 0; //global position of joystick x direction -> between 1-1023
uint16_t joystick_y = 0;

using namespace std;

void *update_state(void *threadid)
{
	while(true)
	{
		//analogRead(100); //read joystick pressed status
		if(USE_JOYSTICK){
			if(DEBUG_INPUT)
				cout << "reading joystick: ";
			joystick_x = analogRead(101); //read joystick x pitch
			joystick_y = analogRead(102); //read joystick y pitch
			if(DEBUG_INPUT)
				cout << "done\n";
		}
		if(USE_MPR121){
			if(DEBUG_INPUT)
				cout << "reading MPR: ";
			touched = sensor.getTouchStatus(); //read MPR
			if(DEBUG_INPUT)
				cout << "done\n";
		}
		
		//cout << "X: " <<joystick_x<< " Y: " << joystick_y << " t: " << touched << endl;
		usleep(MICRO_S_SLEEP_UPDATE);
		if(DEBUG_INPUT){
			int mask = 1;
			for(int i=0;i<TOUCH_INPUTS;i++){
				cout << ((mask & touched) >1);
				mask <<=1;
			}
			cout << "\tjoystick x: " << joystick_x << "  ";
			cout << "\tjoystick y: " << joystick_y << "  ";
			cout << "\n";
		}
	}
	cout << "Sense thread is dead!" << endl;
	pthread_exit(NULL);
}

int initialize_SDL(){
	if(SDL_Init(SDL_INIT_AUDIO) < 0){
		cout << "SDL NOT INITIALIZED!\n" << SDL_GetError()  << endl;
		return 1;
	}
	cout << "SDL OK\n";
	return 0;
}

int initialize_touch(){
	sensor = MPR121();
	sensor.initialize();

	if(sensor.testConnection())
	{
		cout << "MPR121 OK" << endl;
		return 0;
	}
	cout << "MPR121 not OK\n" << endl;
	return 1;
}

int initialize_joystick(){
	wiringPiSetup();
	mcp3004Setup(100,0);
	cout << "Joystick OK!\n";
	return 0;
}

int read_files(Uint32* sound_len, float ** sound_buf){
	SNDFILE *sf;
    SF_INFO info;
    int num_items;
    int f;
    int sr;
    int c;
    char const * file_names[3] = {"C.wav","D.wav","E.wav"};
    for(int i=0;i<3;i++){
		/* Open the WAV file. */
		info.format = 0;
		sf = sf_open(file_names[i],SFM_READ,&info);
		if (sf == NULL){
			printf("Failed to open the file %d.\n",i);
			return 1;
		}
		f = info.frames;
		sr = info.samplerate;
		c = info.channels;
		num_items = f*c;
		/* Allocate space for the data to be read, then read it. */
		sound_buf[i+1] = (float *) malloc(num_items*sizeof(float));
		sf_read_float(sf,sound_buf[i+1],num_items);
		sf_close(sf);
		sound_len[i+1]=(Uint32)num_items;
	}
	//make room for 1s silence
	sound_len[0]= sr;
	sound_buf[0]= (float *)calloc(sr, sizeof(float));
	cout << "Loaded WAV files!\n";
	return 0;
}

int main(int argc, char** argv){
	//Initialize everything
	Uint32 sound_len[4] = {0};
	float** sound_buf = (float **)calloc(4,sizeof(float *));
	
	if(read_files(sound_len, sound_buf) != 0){
		cout << "Problem reading files!\n";
		return 1; 
	}
	
	if(USE_JOYSTICK){
		if(initialize_joystick() != 0){
			cout << "Problem initializing joystick\n";
			return 1;
		}
	}
	
	if(USE_MPR121){
		if(initialize_touch() != 0){
			cout << "Problem initializing touch sensor\n";
			return 1;
		}	
	}
	
	if(initialize_SDL() != 0){
		cout << "Problem initializing SDL\n";
		return 1;
	}
	
	//run thread for touch detection
	int re1 = 0;
	pthread_t sense;

	if(pthread_create(&sense, NULL, update_state, (void *)re1)){
		cout << "Error: Could not start sense_thread!\n";
		return 1;
	}
	
	while(pthread_kill(sense,0) == 0 )
	{
		cout << "." << flush;
		usleep(MICRO_S_SLEEP_SOUND);
	}
	return 0;
}
