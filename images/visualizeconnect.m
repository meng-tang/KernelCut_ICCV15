imgname = '124084';
global rgbimg
rgbimg = imread([ imgname '.bmp']);
global H
global W
global C
[H, W, C] = size(rgbimg);
iptsetpref('ImshowBorder','tight');
set(0,'DefaultFigureMenu','none');
global f
f= figure;imshow(rgbimg);
fileID = fopen( [imgname '_knn.bin'],'r');
global knnidx
knnidx = fread(fileID,[H*W, 50],'int32');
fclose(fileID);
set(f,'WindowButtonDownFcn',@mycallback);