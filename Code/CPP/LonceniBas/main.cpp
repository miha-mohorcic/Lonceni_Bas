#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "MPR121.h"
#include "I2Cdev.h"

//#include "wiringPi.h"
//#include "wiringPiI2C.h"

using namespace std;

int main()
{
    cout << "Start testing" << endl;

    MPR121 senzor = MPR121(0x5a);

    cout << "got class" <<endl;
    if (senzor.testConnection()){
        cout << "OK" << endl;
    }
    else {
        return 1;
    }

    senzor.initialize();

    bool touched;
    while(true)
    {
        touched = senzor.getTouchStatus(0);
        cout << touched << endl;
    }

    return 0;
}
