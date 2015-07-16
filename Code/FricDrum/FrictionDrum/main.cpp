#include <iostream>
#include <unistd.h>

#include <chrono>
#include <ctime>
#include <ratio>

#include <pthread.h>

#include "MPR/MPR121.h"

#include <fstream>


bool touched[12] = {false};
double times[12] = {10.0};

using namespace std;
using namespace std::chrono;

void *sense_thread(void *threadid)
{

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

    int num_sensors = 12;
    bool temp_touched;
    high_resolution_clock::time_point last_change_time[12];

    while(true)
    {
        int i = 0;
        while (i < num_sensors )
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
            i++;
        }
        usleep(25000);
    }
    pthread_exit(NULL);
}

void *produce_thread(void *threadid)
{
    pthread_exit(NULL);
}

int main()
{
    int re1, re2 =0;
    //Zazene thread zaznave
    pthread_t sense;
    int start1 = pthread_create(&sense, NULL, sense_thread, (void *)re1);
    if(start1)
    {
        cout << "Error: Could not start sense_thread!\n" ;
    }

    //Zazene thread ki producira zvok
    pthread_t produce;
    int start2 = pthread_create(&produce, NULL, produce_thread, (void *)re2);
    if(start2)
    {
        cout << "Error: Could not start produce_thread!\n" ;
    }

    // dobi "vector" premika (avg)
    int hist_sample = 10;
    bool **hist;
    hist = (bool **)malloc(sizeof(double *)*hist_sample);
    for(int i=0;i<hist_sample;i++){
        hist[i] = (bool *)malloc(sizeof(double)*12);
    }
    int temp_sum, number_sum = 0;
    double temp_avg = 0.0;
    bool *temp_row;

    while(true)
    {
        temp_row = hist[0];
        double avg = 0.0;
        double old_avg = 0.0;
        for (int i=0; i<12;i++)
        {
            if(hist[0][i]){
                old_avg+=(double)i;
                number_sum++;
                //cout << "1";
            }else{
                //cout << "0";
                }
        }
        //cout << "  " << 0 << "\n";
        old_avg=(double)(old_avg/number_sum);
        for (int i=1;i < hist_sample;i++)
        {
            number_sum=0;
            temp_sum=0;
            for (int j=0; j<12;j++)
            {
                if(hist[i][j])
                {
                    number_sum++;
                    temp_sum+=j;
                    //cout << "1";
                }else{
                    //cout << "0";
                }
            }
            //cout << "  " <<  i << "\n";

            temp_avg = (double)temp_sum/number_sum;
            avg+=old_avg-temp_avg;
            old_avg=temp_avg;

            hist[i-1]=hist[i];
        }

        // also move the last col
        hist[hist_sample-1]=temp_row;

        int j=0;
        while(j<12)
        {
            //cout << touched[j];
            hist[hist_sample-1][j]=touched[j];
            j++;
        }
        //cout << "\n\n";
        if(avg < 0.0)
        {
            cout << avg << " Premik gor\n";
        }
        else if(avg > 0.0)
        {
            cout << avg << " Premik dol\n";
        }
        else
        {
            //cout << avg << " Ni spremembe\n";
        }

        usleep(25000);
    }

    /*
        //Ta thread se ukvarja s prepoznavo gest
        int i = 0;
        int j= 0;
        bool hist[1200][12];
        cout << "Start of recording!";
        while (i<1200){
    	j=0;
    	while (j<12){
    	    hist[i][j] = touched[j];
       	    j++;
    	}
    	i++;
    	if(i%40 == 0){
    		cout <<  i/40 ;
    	}
            usleep(25000);
        }
        cout << "End of recording!";
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
        myfile.close();
    */

    pthread_exit(NULL);
}
