// Basic utilities
#ifndef _BASICUTIL_H__
#define _BASICUTIL_H__
#include <time.h>
#include <string>
#include <algorithm>
#include <vector>

#include <graph.h>
#include "ezi/Table2D.h"
#include "ezi/Image2D.h"
#include "Image.h"

#define outv(A) cout << #A << ": " << (double)(A) << endl; // output a Variable
#define outs(S) cout<<S<<endl; // output a String

#define INFTY 1e+20
#define EPS 1e-20 

#include <sstream>

namespace pch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}



enum Label {NONE=0, OBJ=1, BKG=2}; 
// UNKNOWN is introduced for monotonic parametric maxflow
// In this case, NONE means out of region of interest.
typedef Graph<double,double,double> GraphType;

// count certain element in table
template<typename T>
int countintable(const Table2D<T> & table, T t);

template<typename T>
Table2D<bool> getROI(const Table2D<T> & table, T t);

// labeling.l ---> OBJ
// any other label ---> BKG
Table2D<Label> replacelabeling(const Table2D<Label> labeling, Label l);
// get labeling according to OBJ color
Table2D<Label> getinitlabeling(const Table2D<int> & initimg, int OBJcolor);
Table2D<Label> getinitlabelingFB(const Table2D<RGB> & initimg, RGB OBJcolor, RGB BKGcolor);
// save binary labeling
void savebinarylabeling(const Table2D<RGB> & img, const Table2D<Label> & labeling,string outname,bool BW = false);
// save multi labeling
// get labeling from maxflow instances
bool getlabeling(GraphType * g, Table2D<Label> & m_labeling);
// add smoothness term to the graph
// lambda is the weight of the smoothness term
// ROI is the region of interest
void addsmoothnessterm(GraphType * g, const Image & image, double lambda, 
	const Table2D<bool> & ROI, bool bordersmooth = false);

template<typename T>
int countintable(const Table2D<T> & table, T t)
{
	int table_w = table.getWidth();
	int table_h = table.getHeight();
	int tsize = 0;
	for(int y=0; y<table_h; y++) 
	{
		for(int x=0; x<table_w; x++) 
		{ 
			if(table[x][y]==t) // certain element t
				tsize++;
		}
	}
	return tsize;
}

template<typename T>
Table2D<bool> getROI(const Table2D<T> & table, T t)
{
	int table_w = table.getWidth();
	int table_h = table.getHeight();
	Table2D<bool> ROI(table_w,table_h,false);
	for(int y=0; y<table_h; y++) 
	{
		for(int x=0; x<table_w; x++) 
		{ 
			if(table[x][y]==t) // certain element t
				ROI[x][y]=true;
		}
	}
	return ROI;
}

Table2D<Label> getinitlabeling(const Table2D<int> & initimg, int OBJcolor)
{
	int img_w = initimg.getWidth();
	int img_h = initimg.getHeight();
	Table2D<Label> initlabeling(img_w,img_h);
	for(int i=0;i<img_w;i++)
	{
		for(int j=0;j<img_h;j++)
		{
			if(initimg[i][j]==OBJcolor)
				initlabeling[i][j] = OBJ;
			else
				initlabeling[i][j] = BKG;
		}
	}
	return initlabeling;
}

Table2D<Label> getinitlabeling(const Table2D<int> & initimg, int OBJcolor, int BKGcolor)
{
	int img_w = initimg.getWidth();
	int img_h = initimg.getHeight();
	Table2D<Label> initlabeling(img_w,img_h);
	for(int i=0;i<img_w;i++)
	{
		for(int j=0;j<img_h;j++)
		{
			if(initimg[i][j]==OBJcolor)
				initlabeling[i][j] = OBJ;
			else if(initimg[i][j]==BKGcolor)
				initlabeling[i][j] = BKG;
			else
				initlabeling[i][j] = NONE;
		}
	}
	return initlabeling;
}

int geterrorcount(const Table2D<Label> & labeling, const Table2D<Label> & gt)
{
	int img_w = labeling.getWidth();
	int img_h = labeling.getHeight();
	int errorcount = 0;
	for(int i=0;i<img_w;i++)
	{
		for(int j=0;j<img_h;j++)
		{
			if((gt[i][j]==OBJ)&&(labeling[i][j]!=OBJ))
				errorcount++;
			else if((gt[i][j]==BKG)&&(labeling[i][j]!=BKG))
				errorcount++;
		}
	}
	return errorcount;
}

Table2D<Label> getinitlabelingFB(const Table2D<RGB> & initimg, RGB OBJcolor, RGB BKGcolor)
{
	int img_w = initimg.getWidth();
	int img_h = initimg.getHeight();
	Table2D<Label> initlabeling(img_w,img_h);
	for(int i=0;i<img_w;i++)
	{
		for(int j=0;j<img_h;j++)
		{
			if(initimg[i][j]==OBJcolor)
				initlabeling[i][j] = OBJ;
			else if(initimg[i][j]==BKGcolor)
				initlabeling[i][j] = BKG;
			else
				initlabeling[i][j] = NONE;
		}
	}
	return initlabeling;
}

void savebinarylabeling(const Table2D<RGB> & img, const Table2D<Label> & labeling,string outname,bool BW)
{
	int img_w = labeling.getWidth();
	int img_h = labeling.getHeight();
	Table2D<RGB> tmp(img_w,img_h);
	//BW = true;
	for(int i=0;i<img_w;i++)
	{
		for(int j=0;j<img_h;j++)
		{
			if(labeling[i][j]==OBJ)
			{
				if(BW) tmp[i][j] = black;
				else tmp[i][j] = img[i][j];
			}
			else
				tmp[i][j] = white;
		}
	}
	if(saveImage(tmp, outname.c_str()))
	    cout<<"saved into: "<<outname<<endl;
}


bool getlabeling(GraphType * g, Table2D<Label> & m_labeling)
{
	int img_w = m_labeling.getWidth();
	int img_h = m_labeling.getHeight();
	int n=0;
	m_labeling.reset(NONE);
	int sumobj =0, sumbkg = 0;
	for (int y=0; y<img_h; y++) 
	{
		for (int x=0; x<img_w; x++) 
		{ 
			n = x+y*img_w;
			if(g->what_segment(n) == GraphType::SOURCE)
			{
				m_labeling[x][y]=OBJ;
				sumobj++;
			}
			else if(g->what_segment(n) == GraphType::SINK)
			{
				m_labeling[x][y]=BKG;
				sumbkg++;
			}
			//else
				//exit(-1);
		}
	}
	if((sumobj==0)||(sumbkg==0))
		return false;
	else
		return true;
}

// add smoothness term to the graph
// lambda is the weight of the smoothness term
// ROI is the region of interest
void addsmoothnessterm(GraphType * g, const Image & image, double lambda,
	const Table2D<bool> & ROI, bool bordersmooth)
{
	int img_w = image.img_w;
	int img_h = image.img_h;
	// number of neighboring pairs of pixels
	int numNeighbor = image.pointpairs.size();
	// n-link - smoothness term
	int node_id1 =0, node_id2 =0;
	
	for(int i=0;i<numNeighbor;i++)
	{
		PointPair pp = image.pointpairs[i];
		node_id1 = pp.p1.x+pp.p1.y*img_w;
		node_id2 = pp.p2.x+pp.p2.y*img_w;
		// if the two points are inside region of interest.
		if(ROI[pp.p1]&&ROI[pp.p2])
		{
			//double v = fn(dI(image.img[pp.p1],image.img[pp.p2]),lambda,image.sigma_square)/(pp.p1-pp.p2).norm();   
			double v = lambda*image.smoothnesscosts[i];
			g->add_edge(node_id1,node_id2,v,v);
		}
	}
	if(bordersmooth)
	{
		for(int i=0;i<image.img_w;i++)
		{
			for(int j=0;j<image.img_h;j++)
				if((i==0)||(i==img_w-1)||(j==0)||(j==img_h-1)) // border
					g->add_tweights(i+j*img_w,0,lambda);
		}
	}
}
template <class T>
bool readbinfile(Table2D<T> & table, const char * filename, int w, int h){
	table.resize(w,h);
	//printf("read bin file %s\n",filename);
	FILE * pFile = fopen ( filename , "rb" );
    if (pFile==NULL) { fputs ("File error",stderr); exit (1);}
	T * buffer = new T[h];
	for(int i=0;i<w;i++){
		fread(buffer,sizeof(T),h,pFile);
		for(int j=0;j<h;j++){
			table[i][j] = buffer[j];
		}
	}
    fclose (pFile);
	delete [] buffer;
	//printf("bin closed\n");
	return true;
}

#endif
