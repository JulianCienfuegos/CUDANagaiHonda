/*
 * This is the Perimeter and Adhesion Force Calculator
 * This computes the contribution to the force which comes
 * from the circumference of the cell and the adhesion between neighboring cells.
 */


#include <iostream>
#include <vector>
#include <algorithm>
#include "cell.hpp"
#include "coordinate.hpp"
#include <math.h>
#include "parameters.hpp"

double ReciprocalDistance(coordinate a, coordinate b)
{
	// This function calculates the reciprocal distance needed in the calculation of the gradients.
	return 1/sqrt(pow((a.x - b.x),2)+pow((a.y - b.y),2));
}

std::vector<double> PerAdhForce(int current_vertex, std::vector<cell> sim_cells, \
	std::vector<int> Associated_Cells, std::vector<coordinate> coordinates)
{
	std::vector<double> force_vec{0, 0};
	for(std::vector<int>::iterator it = Associated_Cells.begin(); it != Associated_Cells.end(); ++it)
	{
		int idx = *it; 
	    cell my_cell = sim_cells.at(idx);
	    double target_perim = my_cell.GetTargetPerimeter();
	    double perim = my_cell.ComputePerimeter(coordinates);
	    
	    // Get the vertices surrounding the current one.
	    std::vector<int> my_verts = my_cell.GetVertices(); // vertices in the cell.
        
	    // Find the local index of the vertex in this vector and get the surrounding vertex indices.
	    int l_idx = std::find(my_verts.begin(), my_verts.end(), current_vertex) - my_verts.begin();
	    int next_vert = my_verts.at((l_idx + 1) % my_verts.size());  // No issue with modulo division here because the l_idx will be positive.
	    int prev_vert;// Modulo division gives negative 1 if the l_idx is 0.
	    if(l_idx == 0)
	    {
	      prev_vert = my_verts.back();
	    }
	    else
	    {
	      prev_vert = my_verts.at(l_idx - 1);
	    }
	    // get the coordinates of the previous and following coordinates.
	    coordinate prev_coord = coordinates.at(prev_vert);
	    coordinate curr_coord = coordinates.at(current_vertex);
	    coordinate next_coord = coordinates.at(next_vert);
	    
        //Find cells sharing these edges. Then get their gamma value.
        double gamma0 = my_cell.GetGamma();
        double gamma1 = 0, gamma2 = 0;
        /*
        for(auto c : sim_cells) // C++14 style for loop
        {
            if ((c.ContainsVertex(curr_coord.index))&&(c.ContainsVertex(prev_coord.index))&&(c.GetIndex() != my_cell.GetIndex()))
                gamma1 = c.GetGamma();                
            
            if ((c.ContainsVertex(curr_coord.index))&&(c.ContainsVertex(next_coord.index))&&(c.GetIndex() != my_cell.GetIndex()))
                gamma2 = c.GetGamma();                

        }
        */
        
	    //Calculate gradient 1 (See Fletcher Paper Implementing Vertex Dynamics . . . In a Consistent Computational Framework.
	    // d_{kj} is the distance between the jth vertex of cell k and the next vertex in anticlockwise direction.
	    std::vector<double> grad1 {0, 0};
	    double rd1 = ReciprocalDistance(prev_coord, curr_coord);
	    grad1.at(0) = rd1*(curr_coord.x - prev_coord.x);
	    grad1.at(1) = rd1*(curr_coord.y - prev_coord.y);

	    
	    //Calculate gradient 2
	    std::vector<double> grad2 {0, 0};
	    double rd2 = ReciprocalDistance(next_coord, curr_coord);
	    grad2.at(0) = rd2*(curr_coord.x - next_coord.x); 
	    grad2.at(1) = rd2*(curr_coord.y - next_coord.y);
        
	    // Sum the forces for the current cell which come from the perimeter.
		force_vec.at(0) -= 2*beta*(perim - target_perim)*(grad1.at(0) + grad2.at(0)); 
		force_vec.at(1) -= 2*beta*(perim - target_perim)*(grad1.at(1) + grad2.at(1));
		
		// Now add the contributions from the adhesion.
		force_vec.at(0) -= (gamma0 + gamma1)*(grad1.at(0))/2;
        force_vec.at(0) -= (gamma0 + gamma2)*(grad2.at(0))/2; 
		force_vec.at(1) -= (gamma0 + gamma1)*(grad1.at(1))/2;
        force_vec.at(1) -= (gamma0 + gamma2)*(grad2.at(1))/2;
	}
	return force_vec;
}
