/*
 * This is the file where the action happens.
 * 
 * I am following the google style guide for naming non-class variables 
 * for naming functions. See:
 * http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Function_Names
 */
 
#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include "cell.hpp"

using namespace std;

int main()
{
	// Create a vector of coordinates. 
	// This process needs to be automated.
	vector<coordinate> coordinate_list;
	coordinate_list.push_back(coordinate(0.0, 0.0));//0
	coordinate_list.push_back(coordinate(2.0, 0.0));//1
	coordinate_list.push_back(coordinate(2.0, 2.0));//2
	coordinate_list.push_back(coordinate(0.0, 2.0));//3
	coordinate_list.push_back(coordinate(4.0, 0.0));//4
	coordinate_list.push_back(coordinate(4.0, 1.0));//5
	coordinate_list.push_back(coordinate(4.0, 2.0));//6
	coordinate_list.push_back(coordinate(2.0, 1.0));//7
	
	// Get  ready to make a mesh using these coordinates.
	// This process also needs to be automated.
	vector<cell> simulation_cells;
	int current_cell_index = 0;
	
	//cell 0
	vector<int> vertex_list;
	vertex_list.push_back(0);
	vertex_list.push_back(1);
	vertex_list.push_back(7);
	vertex_list.push_back(2);
	vertex_list.push_back(3);
	simulation_cells.push_back(cell(current_cell_index, vertex_list));
	current_cell_index++;
	
	//cell 1
	vertex_list.clear();
	vertex_list.push_back(1);
	vertex_list.push_back(4);
	vertex_list.push_back(5);
	vertex_list.push_back(7);
	simulation_cells.push_back(cell(current_cell_index, vertex_list));
	current_cell_index++;
		
	//cell 2
	vertex_list.clear();
	vertex_list.push_back(7);
	vertex_list.push_back(5);
	vertex_list.push_back(6);
	vertex_list.push_back(2);
	simulation_cells.push_back(cell(current_cell_index, vertex_list));
	current_cell_index++;
	
	// With this loop we can figure out which cells the vertex belongs to and integrate accordingly.
	for(int i = 0; i < 8; i++)
	{
		printf("\nVertex %d\n", i);
		for(unsigned int j = 0; j < 3; j++)
		{
			if(simulation_cells[j].ContainsVertex(i) == 1) printf(" %u ", j);
		}
	}
	printf("\n");
	
	ofstream OffFile;
	OffFile.open("NagaiHonda.off");

	cout << "Area of cell 0 " << simulation_cells[0].ComputeArea(coordinate_list) << endl;
	cout << "Area of cell 1 " << simulation_cells[1].ComputeArea(coordinate_list) << endl;
	cout << "Area of cell 2 " << simulation_cells[2].ComputeArea(coordinate_list) << endl;

	cout << "Perimeter of cell 0 " << simulation_cells[0].ComputePerimeter(coordinate_list) << endl;
	cout << "Perimeter of cell 1 " << simulation_cells[1].ComputePerimeter(coordinate_list) << endl;
	cout << "Perimeter of cell 2 " << simulation_cells[2].ComputePerimeter(coordinate_list) << endl;
	
	OffFile << "OFF" << endl;
	OffFile << "8 2 0" << endl;
	for(int i = 0; i < 8; i++)
	{
		OffFile << coordinate_list[i].x << " " << coordinate_list[i].y << " " << 0.00 << endl;
	}
	for(int j = 0; j < 3; j++)
	{
		simulation_cells[j].PrintCell(OffFile);
	}
	OffFile.close();
}

