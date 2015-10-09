// Inclusion guard
#ifndef _MAJOR_H_
#define _MAJOR_H_
#define DECLDIR __declspec(dllexport)
extern "C"
{
	// End the inclusion guard
	DECLDIR int mySum(int a,int b,int *c);
	
	DECLDIR void Building(float***rasterData, int w, int h, int planes);
}
#endif
