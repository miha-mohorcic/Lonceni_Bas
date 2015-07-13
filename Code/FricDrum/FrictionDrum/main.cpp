#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "MPR/MPR121.h"
#include "MPR/I2Cdev.h"

using namespace std;


int main()
{
    cout << "Hello MPR121!" << endl;

    MPR121 senzor = MPR121();
    cout << "class" << endl;

    if (senzor.testConnection()){
        cout << "MPR121 OK" << endl;
        cout << flush;
    }
    else {
        cout << "MPR121 not OK" << endl;
        cout << flush;
        return 1;
    }
    cout << "init" << endl;

    senzor.initialize();
    cout << "loop" << endl;

    bool touched;
    while(true)
    {
        touched = senzor.getTouchStatus(0);
        cout << touched << endl;
    }

    return 0;
}
