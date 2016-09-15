KernelCut_ICCV15

This is the code for the paper:

	"Secrets of GrabCut and Kernel K-means"
	Meng Tang and Ismail Ben Ayed and Yuri Boykov
	International Conference on Computer Vision (ICCV), Santiago, Chile, December, 2015.

Author: Meng Tang (mtang73@uwo.ca)

1. $ make main
2. compute knn graph in color space, run images/computeknn.m which takes image name as argument
   The obtained knn graph can be visualized in image grid by running images/visualizeconnect, click on pixel to see its neighbors.
3. $ ./main  
   Usage: main -i imagename [-h on or off (hardconstraints)] [-s smoothnessweight]  
   Example usage:  
   $ ./main -i 0_5_5303 -h off -s 0.1 (in this case smoothnessweight is 0.1, hardconstraints are turned off)  
   $ ./main -i 124084 (in this case smoothnessweight is zero, hardconstraints are turned on)  
   $ ./main -i 124084 -s 0.2 (in this case smoothnessweight is 0.2, hardconstraints are turned on)  
   $ ./main -i 14_18_s (in this case smoothnessweight is zero, hardconstraints are turned on)  
   $ ./main -i 130066 -s 0.1 -h off (in this case hardconstraints are turned off)  

   
