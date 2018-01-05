#include <iostream>
#include <
#include "sctp.h"

using namespace std;

int main()
{

    try
    {
    SctpConn = new sctp();
    }
    catch(exception& e)
            cout << e.what() << endl;

    return 0;
}
