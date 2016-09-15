#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <graph.h>
#include <EasyBMP.h>

#include "util.h"

using namespace std;

Table2D<double> knnfiltering(const Table2D<int> & knntable, Table2D<bool> ROI, int KNN_K);
double computeenergy(const Table2D<Label> & labeling, const Image & image, const Table2D<int> & knntable, double aaweight, double smoothnessweight);

int main(int argc, char * argv[])
{
    srand( (unsigned)time( NULL ) );
    double totaltime = 0; // timing
    clock_t start = clock();
    const char * UsageStr = "Usage: main -i imagename [-h on or off (hardconstraints)] [-s smoothnessweight]\n";
    if(argc == 1){ 
        printf("%s",UsageStr);
        exit(-1);
    }
    int opt;
    char * imgname = NULL;
    bool hardconstraintsflag = true;
    double smoothnessweight = 0; // weight of smoothness term
    double aaweight = 10.0; // weight of average association term
    
    while((opt = getopt(argc, argv, "ihs")) != -1){
        switch(opt){
            case 'i':
                imgname = argv[optind];
                break;
            case 'h':
                if(0 == strcmp(argv[optind],"on"))
                    hardconstraintsflag = true;
                else if(0 == strcmp(argv[optind],"off")){
                    hardconstraintsflag = false;
                }
                break;
            case 's':
                smoothnessweight = atof(argv[optind]);
                break;
            default: /* '?' */
                fprintf(stderr, "%s", UsageStr);
                break;
        }
    }            
    
    // read image
    Image image = Image((string("images/")+string(imgname) + string(".bmp")).c_str(),imgname,16,8);
        
    int img_w = image.img_w;
    int img_h = image.img_h;
    
    // read KNN graph
    char knnfile[100] = {0};
    strcat(knnfile,"images/");
    strcat(knnfile,imgname);
    strcat(knnfile,"_knn.bin");
    printf("knn file path:%s\n",knnfile);
    //read knn table
    int KNN_K = 50;
    Table2D<int> temp_knntable; 
    readbinfile(temp_knntable,knnfile, KNN_K,img_w * img_h);
    Table2D<int> knntable = Table2D<int>(img_w*img_h,KNN_K); // index from zero
    for(int i=0;i<KNN_K;i++){
        for(int j=0;j<img_w*img_h;j++){
            knntable[j][i] = temp_knntable[i][j]-1;
        }
    }
    temp_knntable.resize(1,1);
    
    // read box
    Table2D<Label> labeling(img_w,img_h,NONE);
    Table2D<Label> hardconstraints(img_w,img_h,NONE);
    labeling = getinitlabeling(loadImage<RGB>((string("images/")+string(imgname) + string("_box.bmp")).c_str()),0);
    for(int i=0;i<img_w;i++){
	    for(int j=0;j<img_h;j++){
		    if(labeling[i][j]==BKG) hardconstraints[i][j] = BKG;
		    else hardconstraints[i][j] = NONE;
	    }
    }
    if(hardconstraintsflag==false) hardconstraints.reset(NONE);
    
    // iterative kernel cut
    Table2D<double> capsource = Table2D<double>(img_w,img_h,0);
    Table2D<double> capsink = Table2D<double>(img_w,img_h,0);
    double energy = computeenergy(labeling, image, knntable, aaweight, smoothnessweight);
    printf("initial energy :%.3f\n",energy);
	while(1)
	{
		GraphType * g;
		g = new GraphType(/*estimated # of nodes*/ img_w*img_h, /*estimated # of edges*/ 4*img_w*img_h); 
	    g->add_node(img_w*img_h);    // adding nodes
		
		// add smoothness term
		if(smoothnessweight>1e-10)
			addsmoothnessterm(g,image,smoothnessweight,Table2D<bool>(img_w,img_h,true),false);

        // take unary bound for average association (kernel k-means)
	    Table2D<double> w_data_OBJ = knnfiltering(knntable, getROI(labeling,OBJ),KNN_K);
	    double obj_size  = countintable(labeling, OBJ);
	    double obj_sum = w_data_OBJ.sum(getROI(labeling, OBJ));
	    for(int i=0;i<img_w;i++){
		    for(int j=0;j<img_h;j++){
			    capsink[i][j] = (- 2*w_data_OBJ[i][j]/obj_size + obj_sum / obj_size / obj_size)*aaweight ;
		    }
	    }

	    Table2D<double> w_data_BKG = knnfiltering(knntable, getROI(labeling,BKG),KNN_K);
	    double bkg_size = countintable(labeling, BKG);
	    double bkg_sum = w_data_BKG.sum(getROI(labeling, BKG));
	    for(int i=0;i<img_w;i++){
		    for(int j=0;j<img_h;j++){
			    capsource[i][j] = (- 2*w_data_BKG[i][j]/bkg_size + bkg_sum / bkg_size / bkg_size)*aaweight;
		    }
	    }
	    
	    // enforce hard constraints
	    for(int j=0;j<img_h;j++)
	    {
		    for(int i=0;i<img_w;i++)
		    {
			    if(hardconstraints[i][j]==BKG) // hard constraints to background
				    capsink[i][j]=INFTY;
			    else if(hardconstraints[i][j]==OBJ) // hard constraints to foreground
				    capsource[i][j]=INFTY;
				g->add_tweights(i+j*img_w,capsource[i][j],capsink[i][j]);
		    }
	    }
	    
	    // run graph cut
	    double flow = g -> maxflow();
		
		Table2D<Label> newlabeling(img_w,img_h);
		if(!getlabeling(g,newlabeling))
		{
			cout<<"trivial solution!"<<endl;
			delete g;
			break; // trivial solution
		}
		delete g;
		if(newlabeling==labeling)
		{
			cout<<"labeling converged!"<<endl;
			break;
		}
		double newenergy = computeenergy(newlabeling, image, knntable, aaweight, smoothnessweight);
		if(energy - newenergy > 0.01){
		    labeling = newlabeling;
		    energy = newenergy;
		    printf("energy = %.3f\n",energy);
		}
		else{
		    cout<<"converged!"<<endl;
		    break;
		}    
	    
    }
    
    // save result
    savebinarylabeling(image.img, labeling,(string("images/")+string(imgname) +string("_kernelcut.bmp")).c_str());
    
    // ground truth and compute error rate
    Table2D<Label> gt = getinitlabeling(loadImage<RGB>((string("images/")+string(imgname) + string("_groundtruth.bmp")).c_str()),255,0);
    double errorrate = 0;
    if( hardconstraintsflag )
        errorrate = geterrorcount(labeling,gt)/ (double) countintable(hardconstraints,NONE);
    else
        errorrate = geterrorcount(labeling,gt)/ (double)(img_w*img_h);
    printf("error rate: %.3f\n",errorrate);
    cout<<"time for segmentation "<<(double)(clock()-start)/CLOCKS_PER_SEC<<" seconds!"<<endl;
}

double computeenergy(const Table2D<Label> & labeling, const Image & image, const Table2D<int> & knntable, double aaweight, double smoothnessweight)
{
    int img_w = image.img_w;
    int img_h = image.img_h;
    int KNN_K = knntable.getHeight();
	int obj_size = countintable(labeling,OBJ);
	int bkg_size = countintable(labeling,BKG);
	int obj_quad_sum = 0, bkg_quad_sum = 0;
	Label l_p,l_q;
	for(int i=0;i<img_w;i++){
	    for(int j=0;j<img_h;j++){
			int p_idx = j+i*img_h;
			l_p = labeling[i][j];
		    for(int k=0;k<KNN_K;k++){
			    int q_idx = (knntable)[p_idx][k];
				if(labeling[q_idx/img_h][q_idx%img_h]==l_p){
				    if(l_p==OBJ) obj_quad_sum ++;
					else if(l_p == BKG) bkg_quad_sum ++;
				}
			}
		}
	}
	double AA = (double)obj_quad_sum*2 / (double)(obj_size+1e-10) + (double)bkg_quad_sum*2 / (double)(bkg_size+1e-10);
	if(smoothnessweight <1e-10)
		return -AA*aaweight;
	double smoothenergy = 0;
	// number of neighboring pairs of pixels
	int numNeighbor = image.pointpairs.size();
	// n-link - smoothness term
	int node_id1 =0, node_id2 =0;
	for(int i=0;i<numNeighbor;i++)
	{
		PointPair pp = image.pointpairs[i];
		node_id1 = pp.p1.x+pp.p1.y*img_w;
		node_id2 = pp.p2.x+pp.p2.y*img_w;
		if((labeling[pp.p1]!=NONE)&&(labeling[pp.p2]!=NONE))
		{
			if(labeling[pp.p1]!=labeling[pp.p2])
			{
				double v = image.smoothnesscosts[i];
				smoothenergy += v;
			}
		}
	}
	return -AA*aaweight + smoothenergy * smoothnessweight;
}

Table2D<double> knnfiltering(const Table2D<int> & knntable, Table2D<bool> ROI, int KNN_K){
	int img_w = ROI.getWidth();
	int img_h = ROI.getHeight();
	Table2D<double> returnv(img_w,img_h,0);
	for(int i=0;i<img_w;i++){
	    for(int j=0;j<img_h;j++){
			int p_idx = j+i*img_h;
		    for(int k=0;k<KNN_K;k++){
			    int q_idx = knntable[p_idx][k];
				if(ROI[q_idx/img_h][q_idx%img_h]){
				    returnv[i][j] += 1.0;
				}
			}
		}
	}
	for(int i=0;i<img_w;i++){
	    for(int j=0;j<img_h;j++){
			int q_idx = j+i*img_h;
			if(ROI[i][j]){
				for(int k=0;k<KNN_K;k++){
					int p_idx = knntable[q_idx][k];
					returnv[p_idx/img_h][p_idx%img_h] += 1.0;
				}
			}
		}
	}
	return returnv;
}
