#include <iostream>
#include <unistd.h>
#include <chrono>
#include <ctime>
#include <ratio>
#include <pthread.h>
#include "MPR/MPR121.h"
#include <fstream>

#define MPR121_NUM_INPUTS 12 // number of inputs on MPR121 used
#define MICRO_S_SLEEP 30000 // micro seconds of sleep
bool touched[12] = {false}; // global status of currently active inputs

using namespace std;
using namespace std::chrono;

void *sense_thread(void *threadid)
{
    MPR121 senzor = MPR121(); //initalizing MPR121
    senzor.initialize();

    /*
        if (senzor.testConnection()) //testing MPR connection (checks register)
        {
            cout << "MPR121 OK starting program!" << endl;
        }
        else
        {
            cout << "MPR121 not OK closing program!" << endl;
            pthread_exit(NULL);
        }
    */

    bool temp_touched;
    high_resolution_clock::time_point last_change_time[12];
    double times[12] = {10.0};

    //TODO: Optimize to use "getTouchStatus()" and use bit-wise operations to speed up the process

    while(true) //constantly check input status and remember time from last change
    {
        for(int i=0; i < MPR121_NUM_INPUTS; i++)
        {
            temp_touched = senzor.getTouchStatus(i);
            if (temp_touched != touched[i])
            {
                if (temp_touched)
                {
                    duration<double> timer = duration_cast<duration<double>>(high_resolution_clock::now()- last_change_time[i]);
                    last_change_time[i] = high_resolution_clock::now();
                    times[i] = timer.count();
                    //cout << "temp_touched " << i << ": rising " << times[i] << "\n";
                    touched[i] = true;
                }
                else
                {
                    duration<double> timer = duration_cast<duration<double>>(high_resolution_clock::now()- last_change_time[i]);
                    times[i] = timer.count();
                    last_change_time[i] = high_resolution_clock::now();
                    //cout << "release " << i << ": falling " << times[i] << "\n";
                    touched[i] = false;
                }

            }
        }
        usleep(MICRO_S_SLEEP);
    }
    pthread_exit(NULL); //if for some reason we fall out of loop (graceful exit)
}

void *produce_thread(void *threadid)
{
    //TODO: Thread that produces sound

    pthread_exit(NULL);
}

//provides space for hist_sample last samples
bool **allocateMemory (int hist_sample)
{

    bool **hist = (bool **)malloc(sizeof(double *)*hist_sample);

    for (int i = 0; i < hist_sample; i++)
        hist[i] = (bool *)malloc(sizeof(double)*MPR121_NUM_INPUTS);

    return hist;
}

//return # of detected gesture
int detectGesture(bool **hist, int hist_sample)
{
    double temp_avg = 0.0;

    bool* temp_row = hist[0];
    double avg = 0.0;
    double old_avg = 0.0;
    int number_sum = 0;
    for (int i=0; i<12; i++)
    {
        if(hist[0][i])
        {
            old_avg+=(double)i;
            number_sum++;
        }
    }

    old_avg=(double)(old_avg/number_sum);
    int temp_sum = 0;

    for (int i=1; i < hist_sample; i++)
    {
        number_sum=0;
        temp_sum=0;
        for (int j=0; j<12; j++)
        {
            if(hist[i][j])
            {
                number_sum++;
                temp_sum+=j;
            }
        }
        temp_avg = (double)temp_sum/number_sum;
        avg+=old_avg-temp_avg;
        old_avg=temp_avg;

        hist[i-1]=hist[i];
    }

    //update last row with new data
    hist[hist_sample-1]=temp_row;
    for(int j=0; j<12; j++)
    {
        cout << touched[j];
        hist[hist_sample-1][j]=touched[j];
    }
    cout << "\n";

    if(avg<0.0)
        return 1;
    else if(avg>0.0)
        return 2;
    else
        return 0;
}

bool detectHold(bool **hist, int hist_sample)
{
    bool hold = true;
    for(int j=0; j<MPR121_NUM_INPUTS; j++)
    {
        hold = true;
        for(int i=0; i<hist_sample; i++)
        {
            if(!hist[i][j])
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
    int hist_sample = 10;

    bool **hist = allocateMemory(hist_sample);
    int gesture;
    bool hold;

    while(true)
    {
        gesture = detectGesture(hist, hist_sample); // get gesture #
        hold = detectHold(hist,hist_sample); // detect if user is holding a stick

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
