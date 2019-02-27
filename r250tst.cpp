// r250test.cpp

#include <iostream.h>

#include "r250.h"
#include "rndlcg.h"
#include <math.h>

//#define TEST1
#define TEST2

int main()
{
	R250 rand( 1 );
//	RandLCG rand( 1 );
	long int val;

#ifdef TEST1
	for (int k = 0; k < 10000; k++)
		val = rand.rani();

	cout << "final value is " << val << '\t';

	cout << "(should be 267771767)\n";
#endif

#ifdef TEST2                 // prints out two columns for a scatter plot
	for (int k = 0; k < 2000; k++)
		cout << rand.rani() << '\t' << rand.rani() << '\n';
#endif
	//generate gauss random number with NORMAL (0,1) DISTRIBUTION
float x1, x2, w, y1, y2;
 for(int i=0;i<1000;++i)
 {
         do {
                 x1 = 2.0 *rand.ranf()- 1.0;
                 x2 = 2.0 * rand.ranf()- 1.0;
                 w = x1 * x1 + x2 * x2;
         } while ( w >= 1.0 );

         w = sqrt( (-2.0 * log( w ) ) / w );
         y1 = x1 * w;
         y2 = x2 * w;
	cout<<y1<<"\n"<<y2<<endl;
	}
	return 0;
	
}

