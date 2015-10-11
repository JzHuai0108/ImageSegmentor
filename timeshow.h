/**
* This file is part of ImageSegmentor.
*
* Copyright (C) 2012 Jianzhu Huai <huai dot 3 at osu dot edu> (The Ohio State University)
*
* ImageSegmentor is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ImageSegmentor is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ImageSegmentor. If not, see <http://www.gnu.org/licenses/>.
*/

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
