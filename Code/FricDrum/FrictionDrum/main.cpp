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
#include "include/WavFile.h"

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
////////////////////////////////////////////////////////////////////////////////
//
// Last changed  : $Date: 2015-05-18 17:32:21 +0000 (Mon, 18 May 2015) $
// File revision : $Revision: 4 $
//

void *produce_thread(void *threadid)
{
	//TODO: Thread that produces sound
	//look at soundStrech example


	char const *file_up = "C.wav";
	char const *file_down = "D.wav";
	char const *file_tap = "E.wav";

	//load files in MEM
	WavInFile *fileUp;
	WavInFile *fileDown;
	WavInFile *fileTap;

	int bits1,bits2,bits3;
	int srate1,srate2,srate3;
	int channels1,channels2,channels3;

	fileUp = new WavInFile(file_up);
	bits1 = (int)(fileUp)->getNumBits();
	srate1 = (int)(fileUp)->getSampleRate();
	channels1 = (int)(fileUp)->getNumChannels();


	cout << bits1 << " " << srate1 << " " << channels1 << endl;

	//loop
		//event checking
		//apply filters (pitch)
		//play sample  - Poglej kako proizvesti zvok iz waveforma.

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
