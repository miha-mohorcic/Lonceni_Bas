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
#include <mcp3004.h>
// touch sensor
#include "mpr121.h" // only register definitions
#include "wiringPiI2C.h" 
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
#define MICRO_S_SLEEP_UPDATE 2000 //sleep for updating MPR
#define MICRO_S_SLEEP_SOUND 1000000 //sleep for sound production
int MPR121_ADDR = 0; //file descriptor for MPR121

//global variables
uint16_t touched = 0; //global bitwise status of currently active inputs
uint16_t joystick_x = 0; //global position of joystick x direction -> between 1-1023
uint16_t joystick_y = 0;

using namespace std;

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
		
		//cout << "X: " <<joystick_x<< " Y: " << joystick_y << " t: " << touched << endl;
		usleep(MICRO_S_SLEEP_UPDATE);
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
	system("gpio load i2c");
	MPR121_ADDR = wiringPiI2CSetup(0x5a); // default address
	if(MPR121_ADDR == -1){
		cout << "MPR121 NOT OK\n" << flush;
		return 1;
	} 
	cout << "starting MPR setup\n";
	// start MPR121 setup
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
	cout << "MPR121 OK\n" << endl;
	return 0;
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
		printf(" Opened File %d.\n",i);
	}
	//make room for 1s silence
	sound_len[0]= sr;
	sound_buf[0]= (float *)calloc(sr, sizeof(float));
	cout << "\nLoading all files OK\n";
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
	
	//everything initialized ( TODO : SDL will need some attention)
	
	//run thread for touch detection
	int re1 = 0;
	pthread_t sense;

	if(pthread_create(&sense, NULL, update_state, (void *)re1)){
		cout << "Error: Could not start sense_thread!\n";
		return 1;
	}
	
	//while sense thread is going -> we produce sounds
	while(pthread_kill(sense,0) == 0 ) 
	{
		cout << "." << flush;
		usleep(MICRO_S_SLEEP_SOUND);
	}
	return 0;
}
