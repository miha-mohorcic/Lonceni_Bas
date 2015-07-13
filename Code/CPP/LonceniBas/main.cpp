#include <iostream>

#include "MPR121.h"

using namespace std;

int main()
{
    cout << "Start testing" << endl;
    //MPR121* senzor = new MPR121();
    MPR121 senzor = MPR121(MPR121_DEFAULT_ADDRESS);
    cout << "1" << endl;

    if (senzor.testConnection()){
        cout << "OK" << endl;
    }

    senzor.initialize();
    cout << "2" << endl;
    bool touched;
    cout << "3" << endl;
    while(true)
    {
        cout << "get data" << endl;
        touched = senzor.getTouchStatus(0);
        cout << "print data" << endl;
        cout << touched << endl;
    }

    return 0;
}
