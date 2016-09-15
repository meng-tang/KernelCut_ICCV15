function [ ] = computeknn( imagename )
%COMPUTEKNN Summary of this function goes here
%   Compute knn graph in 3D color space
%   The graph is used as kernel for kernel k-means (average association)
%   read image
rgbimg = imread([imagename '.bmp']);
[M, N, ~] = size(rgbimg);
%   add gaussian noise to image (Images typically come with quantization artifcat,
%   which makes KNN graph not connected if noise not added)
rgbimg = imnoise(rgbimg,'gaussian',0,0.0002);
% transform to LAB space
labimg = rgb2lab(rgbimg);
%% normalize lab so that each channel has unit std
X = [reshape(labimg(:,:,1),M*N,1) reshape(labimg(:,:,2),M*N,1) ...
    reshape(labimg(:,:,3),M*N,1) ];
x_std = std(X,0,1);
X = X ./ repmat(x_std,M*N,1);
% knn search in color space
K = 400;
tic
[knnidx, ~] = knnsearch(X,X,'K',K);
toc
% perturb the knn in 'image grid', this is a trick to reach out to large
% neighborhood in the color space without knn search of large K
knnidx = int32(knnidx);
knnidx_r = mod(knnidx-1,M);
knnidx_c = (knnidx-1-knnidx_r)/M;
knnidx_r = knnidx_r + 1;
knnidx_c = knnidx_c + 1;
knnidx_r = knnidx_r + int32(randi([-2 2],M*N,K));
knnidx_c = knnidx_c + int32(randi([-2 2],M*N,K));
knnidx_r = max(1,knnidx_r);knnidx_r = min(M,knnidx_r);
knnidx_c = max(1,knnidx_c);knnidx_c = min(N,knnidx_c);
knnidx = (knnidx_c - 1 )*M + knnidx_r -1 +1;
% save knn graph as .bin file, for efficiency, we can sample the knn graph
% we sample 50 neighbors out of 400 neighbors
fileID = fopen( [imagename '_knn.bin'],'w');
fwrite(fileID,knnidx(:,1:8:end),'int32');
fclose(fileID);
disp(['saved into bin files ' imagename '_knn.bin']);
end

