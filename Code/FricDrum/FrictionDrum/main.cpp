#include <iostream>
#include <unistd.h>

#include <chrono>
#include <ctime>
#include <ratio>

#include <pthread.h>

#include "MPR/MPR121.h"

#include <fstream>


bool table[12] = {false};
double times[12] = {10.0};

using namespace std;
using namespace std::chrono;

void *sense_thread(void *threadid){

    MPR121 senzor = MPR121();
    senzor.initialize();

    if (senzor.testConnection())
    {
        cout << "MPR121 OK starting program!" << endl;
        cout << flush;
    }
    else
    {
        cout << "MPR121 not OK closing program!" << endl;
        cout << flush;
        pthread_exit(NULL);
    }

    int num_sensors = 5;
    bool temp_touched;
    high_resolution_clock::time_point last_change_time[12];

    while(true)
    {
        int i = 0;
        while (i < num_sensors )
        {
            temp_touched = senzor.getTouchStatus(i);
            if (temp_touched != table[i])
            {
                if (temp_touched)
                {
                    duration<double> timer = duration_cast<duration<double>>(high_resolution_clock::now()- last_change_time[i]);
                    last_change_time[i] = high_resolution_clock::now();
                    times[i] = timer.count();
                    cout << "temp_touched " << i << ": rising " << times[i] << "\n";
                    table[i] = true;
                }
                else
                {
                    duration<double> timer = duration_cast<duration<double>>(high_resolution_clock::now()- last_change_time[i]);
                    times[i] = timer.count();
                    last_change_time[i] = high_resolution_clock::now();
                    cout << "release " << i << ": falling " << times[i] << "\n";
                    table[i] = false;
                }

            }
            i++;
        }
        usleep(25000);
    }
    pthread_exit(NULL);
}

void *produce_thread(void *threadid){
    pthread_exit(NULL);
}

int main()
{
    //Zazene thread zaznave
    pthread_t sense;
    int start1 = pthread_create(&sense, NULL, sense_thread, (void *)1);
    if(start1){
    cout << "Error: Could not start sense_thread!\n" ;
    }


    //Zazene thread ki producira zvok
    pthread_t produce;

    //Ta thread se ukvarja s prepoznavo gest
    int i = 0;
    int j= 0;

    bool hist[1200][12];

    while (i<1200){
	j=0;
	while (j<12){
	    hist[i][j] = table[j];
   	    j++;
	}
	i++;
        usleep(25000);
    }

    ofstream myfile;
    myfile.open ("example.txt");
    j=0;
    while(j<12){
	i=0;
    	while (i<1200){
	    if (hist[i][j]){
		myfile << "X" ;
	    }
	    else{
		myfile << " " ;
	    }
	    i++;
	}
    myfile << "\n";
    j++;
    }

myfile << "Writing this to a file.\n";
    myfile.close();


    pthread_exit(NULL);
}
