#include <vector>
#include <cassert>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <stdio.h>
#include <fstream>
#include "cell.hpp"
#include <iostream>
#include "coordinate.hpp"

void cell::InsertVert(int v1, int v2)
{
    // This function finds v1 in the cell, and inserts v2 AFTER it.
    //std::cout << GetIndex() << std::endl;
    //for(auto i : m_AssociatedVertices)
			//std::cout << i << " ";
		//std::cout << std::endl;
		//std::cout << " v1 : " << v1 << std::endl;
		std::vector<int>::iterator after_this = find(m_AssociatedVertices.begin(), m_AssociatedVertices.end(), v1);
		//std::cout << " after_this: " << *after_this <<std::endl;
		if(after_this != m_AssociatedVertices.end())
			m_AssociatedVertices.insert(next(after_this), v2);
		else if(after_this == m_AssociatedVertices.end())
			m_AssociatedVertices.insert(after_this, v2);
		else
			std::cout << "Nowhere to insert vertex. See InsertVert()" << std::endl;
		//for(auto i : m_AssociatedVertices)
			//std::cout << i << " ";
		//std::cout << std::endl;
}

double cell::ComputePerimeter(std::vector<coordinate> coordinates)
{	
	/*
	 * This function will take in the list of coordinates and 
	 * calculate the perimeter of the cell using them.
	 */
	double perimeter = 0.0;
	unsigned int n = m_AssociatedVertices.size();
	for(unsigned int edge = 0; edge < n - 1; edge++)
	{
		perimeter += std::abs(std::sqrt(std::pow((coordinates[m_AssociatedVertices[edge]].x - coordinates[m_AssociatedVertices[edge + 1]].x),2)
						    + std::pow((coordinates[m_AssociatedVertices[edge]].y - coordinates[m_AssociatedVertices[edge + 1]].y),2)));
	}
	perimeter += std::abs(std::sqrt(std::pow((coordinates[m_AssociatedVertices[n-1]].x - coordinates[m_AssociatedVertices[0]].x),2)
						+ std::pow((coordinates[m_AssociatedVertices[n-1]].y - coordinates[m_AssociatedVertices[0]].y),2)));
	return perimeter;
}

double cell::ComputeArea(std::vector<coordinate> coordinates)
{
	/* 
	 * This function will use the cells list of associated vertices and 
	 * the list of coordinates to calculate the area inside the polygon.
	*/
	double area = 0.0;
	unsigned int n = m_AssociatedVertices.size();
	for(unsigned int i = 0; i < n - 1; i++)
	{
		area += coordinates[m_AssociatedVertices[i]].x*coordinates[m_AssociatedVertices[i+1]].y 
			  - coordinates[m_AssociatedVertices[i+1]].x*coordinates[m_AssociatedVertices[i]].y;
	}
	area += coordinates[m_AssociatedVertices[n-1]].x*coordinates[m_AssociatedVertices[0]].y 
	      - coordinates[m_AssociatedVertices[0]].x*coordinates[m_AssociatedVertices[n-1]].y; 
	return std::abs(0.5*area);
}	

void cell::PrintCell(std::ofstream &OffFile)
{
	/*
	 * This function will print out the coordinates in whatever format I choose.
	 * For now we will use geomview.
	 */
	 OffFile << m_AssociatedVertices.size() << " ";
	 for(unsigned int vert = 0; vert < m_AssociatedVertices.size(); vert++)
	 {
		 OffFile << m_AssociatedVertices[vert] << " ";
	 }
     if ((this->GetGamma() != 1.0) && (this->GetTargetArea() != 1.0))
        OffFile << "\t1 0 0 0" << std::endl;
     else if (this->GetTargetArea() != 1.0)
        OffFile << "\t0 1 0 0" << std::endl;   
     else if (this->GetGamma() != 1.0)
        OffFile << "\t0 0 1 0" << std::endl;
     else
        OffFile << std::endl;
}

int cell::ContainsVertex(int index)
{
	for (unsigned int vert = 0; vert < m_AssociatedVertices.size(); vert++)
	{
		if(index == m_AssociatedVertices[vert]) return 1;
	}
	return 0;
}
