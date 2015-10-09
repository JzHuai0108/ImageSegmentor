#ifndef  TIMESHOW_H
#define TIMESHOW_H
#include<time.h>
long 	currentTime=0;
void  StartTimer( void )
{

	//set msSystem time to system time
	currentTime = clock();

	//done.
	return;

}
double  ElapsedTime( void )
{

	//return the amount of time elapsed in seconds
	//since the msSystem time was last set...
	return ((double) (clock() - currentTime))/(CLOCKS_PER_SEC);

}
#endif