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
#define TOUCH_INPUTS 8
//delays should be big enough
#define MICRO_S_SLEEP_UPDATE 10000 	// delay between MPR updates
#define MICRO_S_SLEEP_SOUND  10000  	// delay for sound production
#define HIST_INP_HOLD_SAMPLE 100
#define HIST_INP_SAMPLE      10
#define NUM_SAMPLES 25				// # pitch shifted samples

//global variables
static bool run_program = true;
static uint16_t touched = 0; //global bitwise status of currently active inputs
static uint16_t joystick_x = 0; //global position of joystick x direction -> between 1-1023
static uint16_t joystick_y = 0;
static uint16_t sjoy_x = 0; //start joystick position
static uint16_t sjoy_y = 0;
static int gesture;

using namespace std;

int MPR121_ADDR = 0; //file descriptor for MPR121
static uint write_MPR(int reg, int data){
	return wiringPiI2CWriteReg8(MPR121_ADDR, (reg & 0xff), (data & 0xff));
}

static uint8_t read_MPR(int reg){
	return wiringPiI2CReadReg8(MPR121_ADDR, (reg & 0xff));
}

static void *input_state(void *threadid){
    for (string line; getline(cin, line);) {
        if(line == "q" || line == "Q"){
			cout << "You wish to close the program. ";
			cout << "This is going BYE BYE ";
			run_program = false;
			break;
		}
		else if(line == "0" )
			gesture = 0;
		else if(line == "1" )
			gesture = 1;
		else if(line == "2" )
			gesture = 2;
		else if(line == "3" )
			gesture = 3;	
		usleep(500000);
    }
	return 0; 
}

static void *update_state(void *threadid){
	uint8_t lsb, msb;
	while(run_program)
	{
		//analogRead(100); //read joystick pressed status	
		joystick_x = analogRead(101); //read joystick x pitch
		joystick_y = analogRead(102); //read joystick y pitch
				
		lsb = read_MPR(0);
		msb = read_MPR(1);
		touched = ((msb << 8) | lsb) ;
		
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

static int initialize_SDL(){
	if(SDL_Init(SDL_INIT_AUDIO) < 0){
		cout << "SDL NOT INITIALIZED!\n" << SDL_GetError()  << endl;
		return 1;
	}
	cout << "SDL OK\n";
	return 0;
}

static int initialize_touch(){
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

static int initialize_joystick(){
	wiringPiSetup();
	mcp3004Setup(100,0);
	sjoy_x = analogRead(101);
	sjoy_y = analogRead(102);
	cout << "Joystick OK!\n";
	return 0;
}

static int read_files(
					Uint8** sound_tap, Uint32* sound_tap_len, 
					Uint8* sound_sil, Uint32 sound_sil_len,
					Uint8** sound_up, Uint32* sound_up_len,
					Uint8** sound_down, Uint32* sound_down_len,
					SDL_AudioSpec* audio_spec)
	{
	cout << "  Reading and initializing media files." << endl;
	string file_name;
    //up files
    for(int i=0;i<NUM_SAMPLES;i++){
		/* Open the WAV file. */
		file_name = "samples/up"+ to_string(i)+".wav";
		if(SDL_LoadWAV(file_name.c_str(), audio_spec, &sound_up[i], &sound_up_len[i]) == NULL){
			cout << "Error opening file up" << i << endl;
			return 1;
		}
		printf(" Opened up File %d.\n",i);
	}
	
	//down file
	for(int i=0;i<NUM_SAMPLES;i++){
		/* Open the WAV file. */
		file_name = "samples/down"+ to_string(i)+".wav";
		if(SDL_LoadWAV(file_name.c_str(), audio_spec, &sound_down[i], &sound_down_len[i]) == NULL){
			cout << "Error opening file down" << i << endl;
			return 1;
		}
		printf(" Opened down File %d.\n",i);
	}
	
	/* Open the WAV file. */
	file_name = "samples/tap.wav";
	if(SDL_LoadWAV(file_name.c_str(), audio_spec, sound_tap, sound_tap_len) == NULL){
		cout << "Error opening tap file "  << endl;
		return 1;
	}

	cout << "\nLoading all files OK\n";
	return 0;
}

static void update_hist(uint16_t *hist){
	for(int i = 1; i < HIST_INP_HOLD_SAMPLE; i++)
		hist[i-1] = hist[i];
	hist[HIST_INP_HOLD_SAMPLE -1] = touched;
}

static bool detect_hold(){
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

static int detect_gesture(){
	static int hist_gesture[HIST_INP_SAMPLE];
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

	int gest = 0;
	if(avg < 0.0) 
		gest = 1;
	else if(avg > 0.0)
		gest = 2;
	else if (tap) 
		gest = 3;
	
	int hist_gesture_count[4] = {0};
	
	hist_gesture_count[gest] = 1;
	for(int i=1;i<HIST_INP_SAMPLE;i++){//update gesture history while doing it
		hist_gesture[i-1] = hist_gesture[i];
		hist_gesture_count[hist_gesture[i]] =  hist_gesture_count[hist_gesture[i]] +1;
	}
	hist_gesture[HIST_INP_SAMPLE-1] = gest;
	
	if(gest == 3) //tap
		return 3;
	
	//return mode for gestures
	short m=0;
	int mnum = hist_gesture_count[0];
	if(mnum < hist_gesture_count[1]){
		m = 1;
		mnum = hist_gesture_count[1];
	}
	if(mnum < hist_gesture_count[2]){
		return 2;
	}
	
	return m;
}

Uint8 *audio_pos;
Uint32 audio_len;
Uint32 audio_played;

static void SDL_audio_callback(void* udata, Uint8* stream, int len){
	if (audio_len == 0) {
		memset(stream, 0, len);
		return;
	}
	
	len = (len > (int) audio_len ? audio_len : len);
	
	SDL_memcpy(stream, audio_pos,len);
	
	audio_pos += len;
	audio_len -= len;
	audio_played += len;
}

int main(int argc, char** argv){
	//Initialize everything
	Uint32 sound_tap_len;
	Uint8*  sound_tap;
	Uint32 sound_sil_len = 44100;
	Uint8  sound_sil[44100] = {0};
	
	Uint32 sound_up_len[NUM_SAMPLES] = {0};
	Uint8** sound_up = (Uint8 **)calloc(NUM_SAMPLES,sizeof(Uint8 *));
	Uint32 sound_down_len[NUM_SAMPLES] = {0};
	Uint8** sound_down = (Uint8 **)calloc(NUM_SAMPLES,sizeof(Uint8 *));
	
	SDL_AudioSpec audio_spec;
	
	if(read_files(	&sound_tap, &sound_tap_len, 
					sound_sil, sound_sil_len,
					sound_up, sound_up_len,
					sound_down, sound_down_len,
					&audio_spec) != 0)
	{
		cout << "Problem reading files!\n";
		return 1; 
	}
		
	if(initialize_joystick() != 0){
		cout << "Problem initializing joystick\n";
		return 1;
	}
	
	
	if(initialize_touch() != 0){
		cout << "Problem initializing touch sensor\n";
		return 1;
	}	

	
	if(initialize_SDL() != 0){
		cout << "Problem initializing SDL\n";
		return 1;
	}
		
	audio_len = sound_sil_len;
	audio_pos = sound_sil;
	
	audio_spec.userdata = NULL;
	audio_spec.callback = SDL_audio_callback;
	
	if(SDL_OpenAudio(&audio_spec,NULL) < 0){
		cout << "Failed opening audio device\n" << SDL_GetError() <<endl;
		return 1;
	}
	SDL_PauseAudio(0);
	
	cout << "Everything initialized!" << endl;
	for(int i=3;i>0;i--){
		cout << "Starting in "<< i << " second/s" << endl;
		usleep(500000);
	}
	
	//run thread for touch detection
	int re2 = 0;
	pthread_t user_inp;
	if(pthread_create(&user_inp, NULL, input_state, (void *)re2)){
		cout << "Error: Could not start input thread!\n";
		return 1;
	}
	int re1 = 0;
	pthread_t sense;
	if(pthread_create(&sense, NULL, update_state, (void *)re1)){
		cout << "Error: Could not start sense_thread!\n";
		return 1;
	}
	
	cout << "Successfuly created background detection thread" << endl;
	cout << "\n\nSTART PLAYING!!!\n\n" << flush;
	
	bool hold;
	int prev_gesture = 0, posx, posy, pitch, prev_pitch = 0;
	//while sense thread is going -> we produce sounds
	while(run_program && pthread_kill(sense,0) == 0 ) 
	{
		gesture = 0;
		hold = detect_hold(); //TODO works badly
		if(!hold)
			gesture = detect_gesture();
		
		if(DEBUG_SOUND)	{
			cout << "hold: " << hold << " " ;
			cout << "gesture: " << gesture << "\n" << flush;
		}
		
		//calc pitch
		posx = joystick_x-sjoy_x;
		posy = joystick_y-sjoy_y;
		pitch = min(NUM_SAMPLES-1, (int)sqrt((posx*posx) + (posy*posy)) / 25 );
		
		if(gesture == 0){
				SDL_LockAudio();
				audio_len = sound_sil_len;
				audio_pos = sound_sil;	
				audio_played = 0;
				SDL_UnlockAudio();
		}else if((prev_gesture != gesture && audio_len == 0) || (prev_gesture == 0 && gesture != 0)){
			SDL_LockAudio();
			switch(gesture){
				case 1: 
					audio_len = sound_down_len[pitch];
					audio_pos = sound_down[pitch];
					break;
				case 2: 
					audio_len = sound_up_len[pitch];
					audio_pos = sound_up[pitch];
					break;
				default:
					audio_len = sound_tap_len;
					audio_pos = sound_tap;
			}
			audio_played = 0;
			SDL_UnlockAudio();
		}else if(prev_gesture == gesture && pitch != prev_pitch){
			SDL_LockAudio();
			switch(gesture){
				case 1: 
					audio_len = sound_down_len[pitch]-audio_played;
					audio_pos = sound_down[pitch]+audio_played;
					break;
				case 2: 
					audio_len = sound_up_len[pitch]-audio_played;
					audio_pos = sound_up[pitch]+audio_played;
					break;
				default:
					audio_len = sound_tap_len-audio_played;
					audio_pos = sound_tap+audio_played;
			}
			SDL_UnlockAudio();
		}
		
		prev_gesture = gesture;
		usleep(MICRO_S_SLEEP_SOUND);
	}
	
	if(pthread_kill(sense,0) == 0)
		pthread_kill(sense,9);
	if(pthread_kill(user_inp,0) == 0)
		pthread_kill(user_inp,9);
		
	//free space before closing
	SDL_CloseAudio();
	for(int i= 0;i<NUM_SAMPLES;i++){
		free(sound_up[i]);
		free(sound_down[i]);
	}
	free(sound_up);
	free(sound_down);
	
	cout << "Everything died, killing this whole thing!" << endl;
	
	return 0;
}
