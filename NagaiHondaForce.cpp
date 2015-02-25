/* This is the meat and potatoes of the code. This algorithm will 
* move each vertex and store its new location in a temporary location
* before copying the temporary array to the old array.
*/

/* The following vector holds the temporary locations of the coordinates.
* I couldnt make this code work until I made a coordinate() default constructor.
* This is an important lesson learned.
*/

#include "cell.hpp"
#include "coordinate.hpp"
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include "Get_Assoc_Cells.hpp"
#include "AreaForce.hpp"
#include "PerAdhForce.hpp"

using std::cout;
using std::endl;
using std::abs;
using std::sqrt;

int NagaiHondaForce(std::vector<coordinate>& coordinate_list,
	 std::vector<cell>& simulation_cells, double step, double delta,
     std::vector<double>& tx, std::vector<double>& ty)
{	
    tx.clear();
    ty.clear();
	/* Calculate the new vertex positions*/
	double dist = 0;
	for(std::vector<coordinate>::iterator it = coordinate_list.begin();
		it != coordinate_list.end(); ++it)
	{
		if(it->IsInner)
		{
            //cout << "vert " << it->index << "is inner\n";
			std::vector<int> assoc_cells = GetCells(it->index, simulation_cells);
            //cout << "start" << endl;

			std::vector<double> area_force = AreaForce(it->index, simulation_cells, assoc_cells, coordinate_list);
			std::vector<double> per_adh_force = PerAdhForce(it->index, simulation_cells, assoc_cells, coordinate_list);
			tx.push_back(step * (area_force.at(0) + per_adh_force.at(0)));
			ty.push_back(step * (area_force.at(1) + per_adh_force.at(1)));

			/* Verify that no vertex has moved too much */
			dist = sqrt(pow(it->t_x, 2) + pow(it->t_y, 2));
			if(dist >  0.5*delta)
				return 0;
		}
        else // the vertex is not an inner vertex. We will set its displacement to 0.
        {
            tx.push_back(0);
            ty.push_back(0);
        }
	}
    return 1;
}

