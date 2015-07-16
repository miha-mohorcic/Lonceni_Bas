#include <iostream>
#include <unistd.h>

#include <chrono>
#include <ctime>
#include <ratio>

#include "MPR/MPR121.h"

using namespace std;

int main()
{
    using namespace std::chrono;

    MPR121 senzor = MPR121();
    senzor.initialize();

    if (senzor.testConnection())
    {
        cout << "MPR121 OK!" << endl;
        cout << flush;
    }
    else
    {
        cout << "MPR121 not OK!" << endl;
        cout << flush;
        return 1;
    }

    int num_sensors = 12;
    bool touched;
    bool table[12] = {false};
    high_resolution_clock::time_point times[12];

    while(true)
    {
        int i = 0;
        while (i < num_sensors )
        {
            touched = senzor.getTouchStatus(i);
            if (touched != table[i])
            {
                if (touched)
                {
                    duration<double> timer = duration_cast<duration<double>>(high_resolution_clock::now()- times[i]);
                    times[i] = high_resolution_clock::now();
                    cout << "touched " << i << ": rising " << timer.count() << "\n";
                    table[i] = true;
                }
                else
                {
                    duration<double> timer = duration_cast<duration<double>>(high_resolution_clock::now()- times[i]);
                    times[i] = high_resolution_clock::now();
                    cout << "release " << i << ": falling " << timer.count() << "\n";
                    table[i] = false;
                }

            }
            i++;
        }
        usleep(25000);
    }

    return 0;
}
