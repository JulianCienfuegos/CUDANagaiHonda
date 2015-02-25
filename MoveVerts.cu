#include <vector>
#include "coordinate.hpp"
#include <stdio.h>
#include <cuda.h>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <iostream>

void MoveVerts(std::vector<coordinate>& coord, std::vector<double> tx, std::vector<double> ty)
{
    // Get all of the x and y verts from the coordinate list
    // store them in two x and y vectors.
//std::cout <<"Going to the GPU!" << std::endl;
    std::vector<double> x;
    std::vector<double> y;
    for(std::vector<coordinate>::iterator c = coord.begin();
        c != coord.end(); c++)
    {   
        x.push_back(c->x);
        y.push_back(c->y);
    }
    
    // Get all of the data on the GPU
    thrust::device_vector<double> X  (x.begin(), x.end());
    thrust::device_vector<double> Y  (y.begin(), y.end());
    thrust::device_vector<double> TX (tx.begin(), tx.end());
    thrust::device_vector<double> TY (ty.begin(), ty.end());

    // Add TX to X and store it in X
    thrust::transform(X.begin(), X.end(), 
                     TX.begin(),
                     X.begin(), 
                     thrust::plus<double>());
    
    // Copy back to x
    thrust::copy(X.begin(), X.end(), x.begin());
    
    // Repeat for y.
    thrust::transform(Y.begin(), Y.end(), 
                     TY.begin(),
                     Y.begin(), 
                     thrust::plus<double>());
                     
    thrust::copy(Y.begin(), Y.end(), Y.begin());
    
    // Now put all of the data back into the coordinates.
    for(int i = 0; i < coord.size(); i++)
        coord[i].SetXY(x[i], y[i]);
//    std::cout << "Done on the GPU" << std::endl;
}

