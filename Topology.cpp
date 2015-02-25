/* 
* This file performs the topological changes for the mesh.
* T1 swap - when an edge gets two small and is reoriented.
* T2 swap - when a triangular cell has a small area and is deleted.
* 
* I identify two types of interior edges. There are fully interior 
* edges and half interior edges. A full interior edge will affect four 
* cells when it undergoes the transformation. A half interior edge will 
* affect only three cells whereas an full interior edge will affect 
* four cells.
* 
*/

#include <cstdlib>
#include <vector>
#include <iostream>
#include <cmath> /* The cmath routines use radians, not degrees !*/
#include <algorithm>
#include <cassert>
#include <random>

#include "cell.hpp"
#include "coordinate.hpp"

const double swap_low_bd = 0.1;

using namespace std;

bool full_int, half_int, both;

/***********************************************************************
*                                                                      *
*************** Functions used by the topology functions ***************
*                                                                      *
***********************************************************************/

double len(coordinate a, coordinate b)
{
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

coordinate MidPoint (coordinate A, coordinate B)
{
    coordinate mp( (A.x + B.x)/2, (A.y+B.y)/2 );
    return mp;	
}

double M_Perp(coordinate A, coordinate B)
{
    double slope  = (A.y - B.y)/(A.x - B.x);
    return -1 / slope;
}

/***********************************************************************
*                                                                      *
********************* Topology functions *******************************
*                                                                      *
***********************************************************************/



/* 
* Remember to always try to break the code up into the smallest
* meaningful pieces possible. I am making the T1 swap its own routine, 
* the search for cells ci, etc.
*/


void T1_Swap(cell& c1, cell& c2, cell& c3, cell& c4, coordinate& I, coordinate& Ip1, double delta)
{
    /* The T1 routine performs I T1 swap on 4 cells if the vertices I 
    * and Ip1 get too close together. The technical details of this 
    * routine are layed out with great detail in my thesis paper. */
    
    /* delete I from c1, delete Ip1 from c2. */
    c1.EraseVert(I.index);
    c2.EraseVert(Ip1.index);
    
    /* calculate the midpoint of AB, and the perpendicular slope to AB.*/
    coordinate mp = MidPoint(I, Ip1);
    double m = M_Perp(I, Ip1);

    /* calculate x' and y' */
    double theta = atan(m);
    double dx = abs(delta*cos(theta));
    double dy = abs(delta*sin(theta));
    
    /* Change I and Ip1 in the coordinate lists */
    if((I.x < Ip1.x)&&(I.y == Ip1.y)){
		I.SetXY(mp.x, mp.y - dy); Ip1.SetXY(mp.x, mp.y + dy);
	}
    else if((I.x > Ip1.x)&&(I.y == Ip1.y)){
		I.SetXY(mp.x, mp.y + dy); Ip1.SetXY(mp.x, mp.y - dy);		
	}
    else if((I.x < Ip1.x)&&(I.y < Ip1.y)){
		I.SetXY(mp.x + dx, mp.y - dy); Ip1.SetXY(mp.x - dx, mp.y + dy);
	}
    else if((I.x > Ip1.x)&&(I.y > Ip1.y)){
		I.SetXY(mp.x - dx, mp.y + dy); Ip1.SetXY(mp.x + dx, mp.y - dy);
	}
    else if((I.x < Ip1.x)&&(I.y > Ip1.y)){
		I.SetXY(mp.x - dx, mp.y - dy); Ip1.SetXY(mp.x + dx, mp.y + dy);
	}
    else if((I.x > Ip1.x)&&(I.y < Ip1.y)){
		I.SetXY(mp.x + dx, mp.y + dy); Ip1.SetXY(mp.x - dx, mp.y - dy);
	}
    else if((I.x == Ip1.x)&&(I.y > Ip1.y)){
		I.SetXY(mp.x - dx, mp.y); Ip1.SetXY(mp.x + dx, mp.y);
	}
    else if((I.x == Ip1.x)&&(I.y < Ip1.y)){
		I.SetXY(mp.x + dx, mp.y); Ip1.SetXY(mp.x - dx, mp.y);
	}
    else
		cout << "ERROR IN T1 SWAP!" << endl;
    
    /* Insert Ip1 after I in c4, insert I after Ip1 in c3. */
    c3.InsertVert(Ip1.index, I.index);
    c4.InsertVert(I.index, Ip1.index);
}


void Perform_T1s(vector<cell>& sim_cells, vector<coordinate>& coords, double delta)
{
    /* This function sniffs out the locations of the T1 swaps and
    * performs them on the whole mesh. As of now this is a very
    * 'brute force' algorithm. See Knuth, D. "Premature optimization
    * is the root of all evil. That is my alibi. */
    for(vector<cell>::iterator c = sim_cells.begin(); c != sim_cells.end(); ++c)              //Go through every cell in the mesh
    {
        vector <int> verts = c->GetVertices(); 
        for(int i = 0; i < verts.size(); i++)                // For each cell go through all of the vertices
        {
            int Im1 = verts.at((i-1)%(verts.size()));                                     // These are the indices 
            int I = verts.at(i);
            int Ip1 = verts.at((i+1)%(verts.size()));
            int Ip2 = verts.at((i+2)%(verts.size()));
            coordinate prev = coords.at(Im1);
            coordinate curr = coords.at(I);
            coordinate next = coords.at(Ip1);
            coordinate next_next = coords.at(Ip2);
            
            if((curr.IsInner)&&(next.IsInner)&&(len(curr, next) < delta)) // Then the edge is fully inside and it is critically small.
            {
                /* Find which cells are affected by this edge. */
                /* The four involved cells will be:
                * 1) The current cell with the small edge.
                * 2) The other cell involving both of these two vertices.
                * 3) The cell involving (i-1) and (i)
                * 4) The cell involving (i+1) and (i+2)
                */
                int id1 = c - sim_cells.begin();
                int id2, id3, id4;
                for(vector<cell>::iterator c_search = sim_cells.begin(); c_search != sim_cells.end(); ++c_search)
                {
                    /* We could make a function called contains_both() */
                    if((c_search->GetIndex() != id1)&&(c_search->ContainsVertex(curr.index))&&(c_search->ContainsVertex(next.index)))
                    {
                        id2 = c_search - sim_cells.begin();
                    }

                    if((c_search->GetIndex() != id1)&&(c_search->ContainsVertex(next.index))&&(c_search->ContainsVertex(next_next.index)))
                    {
                        id3 = c_search - sim_cells.begin();
                    }
                    if((c_search->GetIndex() != id1)&&(c_search->ContainsVertex(prev.index))&&(c_search->ContainsVertex(curr.index)))
                    {
                        id4 = c_search - sim_cells.begin();
                    }
                }
                T1_Swap(sim_cells[id1], sim_cells[id2], sim_cells[id3], sim_cells[id4], coords[I], coords[Ip1] , delta); // This is upside down
                cout << "T1 swap occurred!" << endl;
            }
        }
    }
}

void Random_T1s(vector<cell>& sim_cells, vector<coordinate>& coords, int max_swaps, double swap_len, int swap_odds)
{
    /* This function sniffs out the locations of the T1 swaps and
    * performs them on the whole mesh. As of now this is a very
    * 'brute force' algorithm. See Knuth, D. "Premature optimization
    * is the root of all evil. That is my alibi. */
    int num_swaps = 0;
    for(vector<cell>::iterator c = sim_cells.begin(); c != sim_cells.end(); ++c)              //Go through every cell in the mesh
    {
		    std::random_device rd; // obtain a random number from hardware
			std::mt19937 eng(rd()); // seed the generator
			std::uniform_int_distribution<> distr(0, swap_odds); // define the range
			int should_swap = distr(eng);
			if(should_swap == 1)
			{
				vector <int> verts = c->GetVertices(); 
                int Im1 = verts.at(0);                                     // These are the indices 
                int I = verts.at(1);
                int Ip1 = verts.at(2);
                int Ip2 = verts.at(3);
                coordinate prev = coords.at(Im1);
                coordinate curr = coords.at(I);
                coordinate next = coords.at(Ip1);
                coordinate next_next = coords.at(Ip2);
                
                if((curr.IsInner)&&(next.IsInner)&&(len(curr, next) < swap_len)&&(len(curr, next) > swap_low_bd*1.1)) // Then the edge is fully inside and it is critically small.
                {
                                                cout << "should swap!" << endl;

                    /* Find which cells are affected by this edge. */
                    /* The four involved cells will be:
                    * 1) The current cell with the small edge.
                    * 2) The other cell involving both of these two vertices.
                    * 3) The cell involving (i-1) and (i)
                    * 4) The cell involving (i+1) and (i+2)
                    */
                    int id1 = c - sim_cells.begin();
                    int id2, id3, id4;
                    for(vector<cell>::iterator c_search = sim_cells.begin(); c_search != sim_cells.end(); ++c_search)
                    {
                        /* We could make a function called contains_both() */
                        if((c_search->GetIndex() != id1)&&(c_search->ContainsVertex(curr.index))&&(c_search->ContainsVertex(next.index)))
                        {
                            id2 = c_search - sim_cells.begin();
                        }
    
                        if((c_search->GetIndex() != id1)&&(c_search->ContainsVertex(next.index))&&(c_search->ContainsVertex(next_next.index)))
                        {
                            id3 = c_search - sim_cells.begin();
                        }
                        if((c_search->GetIndex() != id1)&&(c_search->ContainsVertex(prev.index))&&(c_search->ContainsVertex(curr.index)))
                        {
                            id4 = c_search - sim_cells.begin();
                        }
                    }
                    T1_Swap(sim_cells[id1], sim_cells[id2], sim_cells[id3], sim_cells[id4], coords[I], coords[Ip1] , swap_low_bd); // This is upside down
                    num_swaps++;
                    max_swaps--;
                }//if(curr)
        } // if(max_swaps)
    }//for(sim_cell iterator)
    cout << num_swaps << " inital random T1 swaps occured!" << endl;
}

void Perform_T2s(vector<cell>& sim_cells, vector<coordinate>& coords, double delta)
{
	/* Loop over all of the cells in the mesh. 
	 * For each cell, check the number of vertices in the cell.
	 * If there are three vertices, check the length of the edges in the cell.
	 * If all of the edges are below delta, then the cell becomes a point.
	 * We will compute the centroid of the cell, delete two of the coordinates
	 * and reset the remaining vertex to the centroid.
	 * Then we look over all of the cells in the mesh and find the cells containing the deleted vertices. 
	 * The vertex is then replaced by the index of the centroid vert.
	 */
	for(vector<cell>::iterator c = sim_cells.begin(); c != sim_cells.end(); c++){
		vector<int> v = c->GetVertices();
		if(v.size() == 3){ // Then c is a triangle
			coordinate *A = &coords.at(v.at(0));
			coordinate B = coords.at(v.at(1));
			coordinate C = coords.at(v.at(2));
			if(len(*A, B) < delta && len(*A, C) < delta && len(B, C) < delta){ // Then the triangle is critically small.
				A->SetXY((A->x + B.x + C.x)/3, (A->y + B.y + C.y)/3); // Update the a coordinates.
				vector <coordinate>::iterator i = find (coords.begin(), coords.end(), B); // Delete the b and c coordinates from the list.
				coords.erase(i);
				i = find (coords.begin(), coords.end(), C); 
				coords.erase(i);
				sim_cells.erase(c); // This only works because the size of the container is going to be larger than one.
				int found = 0;
				for(vector<cell>::iterator other = sim_cells.begin(); other != sim_cells.end(); other++){
					if(other->ContainsVertex(B.index) && other->ContainsVertex(A->index)){
						other->EraseVert(B.index);
						found++;
					}
					if(other->ContainsVertex(C.index) && other->ContainsVertex(A->index)){
						other->EraseVert(C.index);
						found++;
					}
					if(other->ContainsVertex(B.index) && other->ContainsVertex(C.index)){
						other->ReplaceVert(B.index, A->index); // Replace b by a
						other->EraseVert(C.index); // delete c. Problem solved.
						found++;
					}
					if(found == 3) break;
				}
                cout << "T2 Swap Occurred!" << endl;
			}
		}
	}
}

