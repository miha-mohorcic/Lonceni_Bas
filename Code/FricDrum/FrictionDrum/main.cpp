////////////////////////////
/// Friction Drum V0.2   ///
/// UL FRI - LGM Project ///
////////////////////////////

//includes
// standard IO
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
// threading
#include <pthread.h>
// joystick
#include "wiringPi.h"
#include <mcp3004.h>
// touch sensor
#include "mpr121.h" // only register definitions
#include "wiringPiI2C.h" 
// WAV playing 
#include "SDL2/SDL.h"

//defines
#define DEBUG_SOUND false
#define DEBUG_INPUT false
#define DEBUG_SDL 	false
#define USE_JOYSTICK true
#define USE_MPR121 true
#define TOUCH_INPUTS 8
//delays should be big enough
#define MICRO_S_SLEEP_UPDATE 2000 // delay between MPR updates
#define MICRO_S_SLEEP_SOUND 2000  // delay for sound production
#define HIST_INP_HOLD_SAMPLE 100
#define HIST_INP_SAMPLE 10


//global variables
uint16_t touched = 0; //global bitwise status of currently active inputs
uint16_t joystick_x = 0; //global position of joystick x direction -> between 1-1023
uint16_t joystick_y = 0;
uint16_t sjoy_x = 0; //start joystick position
uint16_t sjoy_y = 0;

using namespace std;

int MPR121_ADDR = 0; //file descriptor for MPR121
uint write_MPR(int reg, int data){
	return wiringPiI2CWriteReg8(MPR121_ADDR, (reg & 0xff), (data & 0xff));
}

uint8_t read_MPR(int reg){
	return wiringPiI2CReadReg8(MPR121_ADDR, (reg & 0xff));
}

void *update_state(void *threadid){
	uint8_t lsb, msb;
	while(true)
	{
		//analogRead(100); //read joystick pressed status
		if(USE_JOYSTICK){
			joystick_x = analogRead(101); //read joystick x pitch
			joystick_y = analogRead(102); //read joystick y pitch
		}
		if(USE_MPR121){
			lsb = read_MPR(0);
			msb = read_MPR(1);
			touched = ((msb << 8) | lsb) ;
		}
		
		if(DEBUG_INPUT){
			int mask = 1;
			for(int i=0;i<TOUCH_INPUTS;i++){
				cout << ((mask & touched) > 0);
				mask <<=1;
			}
			cout << "\tjoystick x: " << joystick_x << "  ";
			cout << "\tjoystick y: " << joystick_y << "  ";
			cout << "\n"<< flush;
		}
		
		usleep(MICRO_S_SLEEP_UPDATE);
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
	cout << "starting MPR setup\n";
	system("gpio load i2c");
	MPR121_ADDR = wiringPiI2CSetup(0x5a); // default address
	if(MPR121_ADDR == -1){
		cout << "MPR121 NOT OK\n" << flush;
		return 1;
	} 
	// start MPR121 registers setup
	write_MPR(ELE_CFG, 0x00); 

	// filtering when data is > baseline.
	write_MPR(MHD_R, 0x01);
	write_MPR(NHD_R, 0x01);
	write_MPR(NCL_R, 0x00);
	write_MPR(FDL_R, 0x00);

	// filtering when data is < baseline.
	write_MPR(MHD_F, 0x01);
	write_MPR(NHD_F, 0x01);
	write_MPR(NCL_F, 0xFF);
	write_MPR(FDL_F, 0x02);

	// touch and release thresholds for each electrode
	write_MPR(ELE0_T, TOU_THRESH);
	write_MPR(ELE0_R, REL_THRESH);

	write_MPR(ELE1_T, TOU_THRESH);
	write_MPR(ELE1_R, REL_THRESH);

	write_MPR(ELE2_T, TOU_THRESH);
	write_MPR(ELE2_R, REL_THRESH);

	write_MPR(ELE3_T, TOU_THRESH);
	write_MPR(ELE3_R, REL_THRESH);

	write_MPR(ELE4_T, TOU_THRESH);
	write_MPR(ELE4_R, REL_THRESH);

	write_MPR(ELE5_T, TOU_THRESH);
	write_MPR(ELE5_R, REL_THRESH);

	write_MPR(ELE6_T, TOU_THRESH);
	write_MPR(ELE6_R, REL_THRESH);

	write_MPR(ELE7_T, TOU_THRESH);
	write_MPR(ELE7_R, REL_THRESH);

	write_MPR(ELE8_T, TOU_THRESH);
	write_MPR(ELE8_R, REL_THRESH);

	write_MPR(ELE9_T, TOU_THRESH);
	write_MPR(ELE9_R, REL_THRESH);

	write_MPR(ELE10_T, TOU_THRESH);
	write_MPR(ELE10_R, REL_THRESH);

	write_MPR(ELE11_T, TOU_THRESH);
	write_MPR(ELE11_R, REL_THRESH);

	// Set the Filter Configuration
	write_MPR(FIL_CFG, 0x04);

	// Enable Auto Config and auto Reconfig
	//write_MPR(ATO_CFG0, 0x0B);
	//write_MPR(ATO_CFGU, 0xC9);  // USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V   write_MPR(ATO_CFGL, 0x82);  // LSL = 0.65*USL = 0x82 @3.3V
	//write_MPR(ATO_CFGT, 0xB5);  // Target = 0.9*USL = 0xB5 @3.3V

	//write_MPR(ELE_CFG, 0x0C);  // Enables all 12 Electrodes
	write_MPR(ELE_CFG, 0x08);  // Enables 8 Electrodes
	//write_MPR(ELE_CFG, 0xC8);	//baseline + 8 electrodes
	cout << "MPR121 OK\n";
	return 0;
}

int initialize_joystick(){
	wiringPiSetup();
	mcp3004Setup(100,0);
	sjoy_x = analogRead(101);
	sjoy_y = analogRead(102);
	cout << "Joystick OK!\n";
	return 0;
}

int read_files(Uint32* sound_len, Uint8 ** sound_buf, SDL_AudioSpec* audio_spec){
	cout << "  Reading and initializing media files." << endl;
	char const *file_tap = "E.wav"; 
	char const *file_up = "C.wav";
	char const *file_down = "D.wav"; 
	
	if(SDL_LoadWAV(file_up,audio_spec,&sound_buf[1],&sound_len[1]) == NULL ||
	SDL_LoadWAV(file_down,audio_spec,&sound_buf[2],&sound_len[2]) == NULL ||
	SDL_LoadWAV(file_tap,audio_spec,&sound_buf[3],&sound_len[3]) == NULL)
	{
		cout << "  ERROR OPENING FILES!\n" << SDL_GetError() << endl;
		return 1;
	}

	sound_len[0] = 88200; //define silence sound (fill with 0's)
	sound_buf[0] = (Uint8 *)calloc(88200, sizeof(Uint8));

	cout << "\nLoading all files OK\n";
	return 0;
}

void update_hist(uint16_t *hist){
	for(int i = 1; i < HIST_INP_HOLD_SAMPLE; i++)
		hist[i-1] = hist[i];
	hist[HIST_INP_HOLD_SAMPLE -1] = touched;
}

bool detect_hold(){
	static uint16_t hist[HIST_INP_HOLD_SAMPLE] = {0};
	static bool comp = true;
	static bool hold = true;
	if(comp){
		uint16_t mask;
		update_hist(hist);
		
		for(int j = 0; j < TOUCH_INPUTS; j++){
			hold = true;
			mask = 1 << j;
			for(int i = 0; i < HIST_INP_HOLD_SAMPLE; i++){
				if(0 == (hist[i] &&mask)){
					hold = false;
					break;
				}
			}
			if(hold){
				break;
			}
		}
	}
	comp = !comp;
	return hold;
}

int detect_gesture(){
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

	for(i = 0; i < TOUCH_INPUTS; i++)
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
	for (i = 0; i < TOUCH_INPUTS; i++)
	{
		if (i == 0 && (touched & mask)) {
			tap = true;
			break;
		} else if (i != 0 && (touched & mask)) {
			tap = false;
			break;
		}
		mask <<= 1;
	}

	if(avg < 0.0) {
	//cout << "1" << endl;
	return 1;
	}
	else if(avg > 0.0) {
	//cout << "2" << endl;
		return 2;
	}
	else if (tap) {
	//cout << "Tap!" << endl;
		return 3;
	}
	return 0;
}

Uint8 *audio_pos;
Uint32 audio_len;

void SDL_audio_callback(void* udata, Uint8* stream, int len){
	static int sample_len = 44100; // 10th of a second is max sample length
	if(audio_len==0)
		return;
	
	len=(len > (int)audio_len ? audio_len : len);
	len=(len > sample_len ? sample_len : len);
	SDL_memcpy(stream, audio_pos,len);
	if(DEBUG_SDL){
		cout << len << " " << sample_len << " " << audio_len << "\n" << flush;
	}
}

int main(int argc, char** argv){
	//Initialize everything
	Uint32 sound_len[4] = {0};
	Uint8** sound_buf = (Uint8 **)calloc(4,sizeof(Uint8 *));
	SDL_AudioSpec audio_spec;
	
	if(read_files(sound_len, sound_buf, &audio_spec) != 0){
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
			cout << "Proble-m initializing touch sensor\n";
			return 1;
		}	
	}
	
	if(initialize_SDL() != 0){
		cout << "Problem initializing SDL\n";
		return 1;
	}
	
	audio_pos = sound_buf[0];
	audio_len = sound_len[0];
	audio_spec.callback = SDL_audio_callback;
	audio_spec.userdata = NULL;
	
	if(SDL_OpenAudio(&audio_spec,NULL) < 0){
		cout << "Failed opening audio device\n" << SDL_GetError() <<endl;
		return 1;
	}
	SDL_PauseAudio(0);
	//everything initialized ( TODO : SDL will need some attention)
	
	cout << "Everything initialized!" << endl;
	for(int i=3;i>0;i--){
		cout << "Starting in "<< i << " second/s" << endl;
		usleep(1000000);
	}
	
	//run thread for touch detection
	int re1 = 0;
	pthread_t sense;
	if(pthread_create(&sense, NULL, update_state, (void *)re1)){
		cout << "Error: Could not start sense_thread!\n";
		return 1;
	}
	
	cout << "Successfuly created background detection thread" << endl;
	cout << "\n\nSTART PLAYING!!!\n\n" << flush;
	
	bool hold;
	int prev_gesture = 0, gesture, posx, posy, pitch;
	//while sense thread is going -> we produce sounds
	while(pthread_kill(sense,0) == 0 ) 
	{
		gesture = 0;
		hold = detect_hold(); //TODO works badly
		//if(!hold)
			gesture = detect_gesture();
		
		if(DEBUG_SOUND)	{
			cout << "hold: " << hold << " " ;
			cout << "gesture: " << gesture << "\n" << flush;
		}

		if(gesture == 0){
			SDL_LockAudio();
			audio_pos=sound_buf[0];
			audio_len=sound_len[0];
			SDL_UnlockAudio();
		}
		
		//change sound if needed
		if(prev_gesture != gesture || audio_len < 40000){
			
			//read gesture and pitch
			posx = joystick_x-sjoy_x;
			posy = joystick_y-sjoy_y;
			pitch = (int)sqrt((posx*posx) + (posy*posy));
			
			SDL_LockAudio();
			
			audio_spec.freq = 44100 + (pitch * 5);
			audio_len = sound_len[gesture];
			audio_pos = sound_buf[gesture];
			
			SDL_CloseAudio();
			if(SDL_OpenAudio(&audio_spec,NULL) < 0){
				cout << "Failed opening audio device\n" << SDL_GetError() <<endl;
				return 1;	
			}
			SDL_PauseAudio(0);
			
			SDL_UnlockAudio();
			
		}
		usleep(MICRO_S_SLEEP_SOUND);
	}
	SDL_CloseAudio();
	for(int i= 0;i<4;i++){
		SDL_FreeWAV(sound_buf[i]);
	}
	
	cout << "Thread died, killing this whole thing!" << endl;
	
	return 0;
}
