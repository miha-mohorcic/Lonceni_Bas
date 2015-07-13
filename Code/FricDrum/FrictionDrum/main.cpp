#include <iostream>

#include "MPR/MPR121.h"

using namespace std;

int main()
{
    cout << "Starting up!" << endl;

    MPR121 senzor = MPR121();
    senzor.initialize();

    /*
    if (senzor.testConnection()){
        cout << "MPR121 OK!" << endl;
        cout << flush;
    }
    else {
        cout << "MPR121 not OK!" << endl;
        cout << flush;
        return 1;
    }
    */

    cout << "loop" << endl;

    bool touched;
    while(true)
    {
        touched = senzor.getTouchStatus(0);
        cout << touched << endl;
    }

    return 0;
}
