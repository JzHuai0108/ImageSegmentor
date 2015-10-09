/*
Copyright (C) 2006 Pedro Felzenszwalb

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

#ifndef SEGMENT_IMAGE
#define SEGMENT_IMAGE
//#include <time.h>
//#include <cstdlib>
//#include "ppmimage.h"
//#include "misc.h"
//#include "filter.h" for convolution
//#include "Region.h"
#include "segment-graph.h"

//4-connected regions
void GraphSeg4(float*dat,int*alpha,int dim,int width,int height,float k,int minsize,int &num_ccs)
{
	// build graph
	edge *edges = new edge[width*height*2];//2 for 4-connected regions
	int num = 0,y=0,x=0,d=0;
	int sernum=0;
	int i=0,pos=0,loc=0;
	for(y = 0; y < height; y++)
	{
		for(x = 0; x < width; x++)
		{	
			if (x < width-1) 
			{
				edges[num].a = sernum;
				edges[num].b = sernum+1;
				pos=sernum*dim;
				loc=pos+dim;
				for(d=0;d<dim;d++)
					edges[num].w+=abs(dat[pos+d]-dat[loc+d]);
				num++;
			}
			if (y < height-1) 
			{
				edges[num].a =sernum;
				edges[num].b =sernum+width;
				pos=sernum*dim;
				loc=pos+dim*width;
				for(d=0;d<dim;d++)
					edges[num].w+=abs(dat[pos+d]-dat[loc+d]);
				num++;
			}
			/*
			if ((x < width-1) && (y < height-1)) 
			{
				edges[num].a =sernum;
				edges[num].b = sernum+width+1;
				edges[num].w = dist(*(smooth_r+sernum),*(smooth_g+sernum),*(smooth_b+sernum),*(smooth_r+sernum+width+1),*(smooth_g+sernum+width+1),*(smooth_b+sernum+width+1));
				num++;
			}
			if ((x < width-1) && (y > 0)) 
			{
				edges[num].a = sernum;
				edges[num].b = sernum-width +1;
				edges[num].w = dist(*(smooth_r+sernum),*(smooth_g+sernum),*(smooth_b+sernum),*(smooth_r+sernum-width+1),*(smooth_g+sernum-width+1),*(smooth_b+sernum-width+1));
				num++;
			}*/
			sernum++;
		}
	}
	// segment
	universe *u = segment_graph(width*height, num, edges, k);
	// It's almost sure that num equals  4*w*h-3*(w+h)+2.It's true.
	// post process small components.
	for (i = 0; i < num; i++)
	{
		int a = u->find(edges[i].a);
		int b = u->find(edges[i].b);
		if ((a != b) && ((u->size(a) < minsize) || (u->size(b) < minsize)))
			u->join(a, b);
	}
	delete [] edges;
	num_ccs = u->num_sets();

	// pick random colors for each component
	/* rgb *colors = new rgb[width*height];
	for (  i = 0; i < width*height; i++)
	colors[i] = random_rgb();*/
	//label pixels
	pos=0;
	for (y = 0; y < height; y++) 
	{
		for (x = 0; x < width; x++) 
		{
			sernum= u->find(pos);	
			alpha[pos]=sernum;			
			pos++;
		}
	}	
	delete u;
}
//8-connected regions
void GraphSeg8(unsigned int*dat,int*alpha,int dim,int width,int height,float k,int minsize,int &num_ccs)
{
	// build graph
	edge *edges = new edge[width*height*4];//2 for 4-connected regions
	int num = 0,y=0,x=0,d=0;
	int sernum=0;
	int i=0,pos=0,loc=0;
	for(y = 0; y < height; y++)
	{
		for(x = 0; x < width; x++)
		{	
			if (x < width-1) 
			{
				edges[num].a = sernum;
				edges[num].b = sernum+1;
				pos=sernum*dim;
				loc=pos+dim;
				for(d=0;d<dim;d++)
					edges[num].w+=abs(dat[pos+d]-dat[loc+d]);
				num++;
			}
			if (y < height-1) 
			{
				edges[num].a =sernum;
				edges[num].b =sernum+width;
				pos=sernum*dim;
				loc=pos+dim*width;
				for(d=0;d<dim;d++)
					edges[num].w+=abs(dat[pos+d]-dat[loc+d]);
				num++;
			}
			
			if ((x < width-1) && (y < height-1)) 
			{
				edges[num].a =sernum;
				edges[num].b = sernum+width+1;
				loc=pos+dim*(width+1);
				for(d=0;d<dim;d++)
					edges[num].w+=abs(dat[pos+d]-dat[loc+d]);
				num++;
			}
			if ((x < width-1) && (y > 0)) 
			{
				edges[num].a = sernum;
				edges[num].b = sernum-width +1;
				loc=pos-dim*(width-1);
				for(d=0;d<dim;d++)
					edges[num].w+=abs(dat[pos+d]-dat[loc+d]);	
				num++;
			}
			sernum++;
		}
	}
	// segment
	universe *u = segment_graph(width*height, num, edges, k);
	// It's almost sure that num equals  4*w*h-3*(w+h)+2.It's true.
	// post process small components.
	for (i = 0; i < num; i++)
	{
		int a = u->find(edges[i].a);
		int b = u->find(edges[i].b);
		if ((a != b) && ((u->size(a) < minsize) || (u->size(b) < minsize)))
			u->join(a, b);
	}
	delete [] edges;
	num_ccs = u->num_sets();

	// pick random colors for each component
	/* rgb *colors = new rgb[width*height];
	for (  i = 0; i < width*height; i++)
	colors[i] = random_rgb();*/
	//label pixels
	pos=0;
	for (y = 0; y < height; y++) 
	{
		for (x = 0; x < width; x++) 
		{
			sernum= u->find(pos);	
			alpha[pos]=sernum;			
			pos++;
		}
	}	
	delete u;
}
#endif
