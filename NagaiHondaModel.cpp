/*
 * This is the file where the action happens.
 * 
 * I am following the google style guide for naming non-class variables 
 * for naming functions. See:
 * http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Function_Names
 */
#include <sys/time.h>
#include <sstream> 
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <stdio.h>
#include "coordinate.hpp"
#include "cell.hpp"
#include "NagaiHondaForce.hpp"
#include "mesh.hpp"
#include "printCellProperties.hpp"
#include <stdlib.h>
#include "EnergyCalculator.hpp"
#include "Topology.hpp"

int iter_notification = 100;

using namespace std;

// These are the configuration reading functions.
void ReadConfig(int&, int&, int&, double&, int&, double&, int&, int&, double&, int&);
void ReadMeshChanges(vector<cell>&);

// These are our CUDA functions.
void MoveVerts(vector<coordinate>&, vector<double>, vector<double>);

// This is for timing the code.
double clkbegin, clkend;
double t;
double rtclock()
{
  struct timezone Tzp;
  struct timeval Tp;
  int stat;
  stat = gettimeofday (&Tp, &Tzp);
  if (stat != 0) printf("Error return from gettimeofday: %d",stat);
  return(Tp.tv_sec + Tp.tv_usec*1.0e-6);
}

int main()
{	
	/***************** READ CONFIGURATION FILE ************************/
	int n, print_e, make_movie, num_perturb, num_iters, out_freq, max_swaps, swap_odds;
	double step, energy, delta, dt, swap_len; 
	ReadConfig(n, print_e, make_movie, step, num_iters, delta, out_freq, max_swaps, swap_len, swap_odds);
	
	/************************ MAKE THE MESH ***************************/
	vector<coordinate> coords;
	vector<cell> sim_cells;
	hex_mesh(coords, sim_cells, n);
    Random_T1s(sim_cells, coords, max_swaps, swap_len, swap_odds);
    perturb_mesh(coords, sim_cells, n, num_perturb);
    ReadMeshChanges(sim_cells);
    
    /************************* WELCOME ********************************/
	cout << "\n\n****************************************************\n";
	cout << "*         Nagai-Honda Model Simulation             *\n";
	cout << "****************************************************\n\n";
	cout << "The number of cells in this simulation is " << sim_cells.size() << endl;
	cout << "The number of coordinates in this simulation is " << coords.size() << endl;
	cout << "Starting the integration... \n";
	ofstream OffFile;
	ofstream PE; 
	PE.open("energy.dat");
	
	/************************** MAIN LOOP *****************************/
	int proceed;
    clkbegin = rtclock();
	for(int iter = 0; iter < num_iters; iter++)
	{
        if(iter % iter_notification == 0)
        cout << "		Current iteration: "  << iter << endl;
        
		/******************* PRINT OUT AN OFF FILE ********************/
		// This should be a routine
		if(iter % out_freq == 0)
		{
		  ostringstream ss;
		  ss << setw(5) << setfill('0') << iter;
		  string s2(ss.str());
			
			OffFile.open("Images/NagaiHonda"+ s2 +".off");
			OffFile << "{appearance {+edge}\n" <<"OFF" << endl;
			OffFile << coords.size() << " " << sim_cells.size() << " 0" << endl;
			
			for(vector<coordinate>::iterator pos = coords.begin(); 
				pos != coords.end(); ++pos)
			{
				OffFile << pos->x << " " << pos->y << " " << 0.00 << endl;
			}
			for(vector<cell>::iterator vert = sim_cells.begin(); 
				vert != sim_cells.end(); ++vert)
			{
				vert->PrintCell(OffFile);
			}
			OffFile << "}";
			OffFile.close();	
		}
		
		/**************** CALCULATE VERTEX MOVEMENT *******************/
        vector<double> tx;
        vector<double> ty;
		dt = step;
		proceed = 0;
		while (proceed == 0)
		{
			proceed = NagaiHondaForce(coords, sim_cells, dt, delta, tx, ty);
			dt = dt/2;
		}
        /************* MOVE THE VERTICES ON THE GPU *******************/
        MoveVerts(coords, tx, ty);
        
		/**************** PERFORM TOPOLOGICAL CHANGES *****************/
		Perform_T1s(sim_cells, coords, delta);
		Perform_T2s(sim_cells, coords, delta);
        /**************** CALCULATE ENERGY IN THE MESH ****************/
		energy = Energy(sim_cells, coords);
		PE << iter << " " << energy << endl;
		
	}
	clkend = rtclock();
    t = clkend - clkbegin;
	/******************* MAKE THE ENERGY PLOT *************************/
	if(print_e == 1)
	{
		cout << "Plotting the system energy..." << endl;
		system("python plot_energy.py");
	}
	
	/*********************** MAKE THE MOVIE ***************************/
	if(make_movie == 1)
	{
		cout << "Making animation..." << endl;
		system("bash Images/make_movie.sh");
	}
    cout << "The code ran in : " << t << " seconds!" << endl;
	cout << "\n*************Simulation Complete********************\n\n";

}

void ReadConfig(int  &n, int &print_E, int &make_movie, double &step, int &num_iters, double &delta, int &out_freq, int &max_swaps, double &swap_len, int &swap_odds)
{
	string junk; // a variable to store the junk we dont want from the conf file.
	ifstream conf("config.txt");
	conf >> n; // get the info we want
	getline(conf, junk); // throw away the rest of the line.
	conf >> print_E;
	getline(conf, junk);
	conf >> make_movie;
	getline(conf, junk);
	conf >> step;
	getline(conf, junk);
	conf >> num_iters;
	getline(conf, junk);
	conf >> out_freq;
	getline(conf, junk);
	conf >> delta;
	getline(conf, junk);
    conf >> max_swaps;
	getline(conf, junk);
    conf >> swap_len;
	getline(conf, junk);
    conf >> swap_odds;
	getline(conf, junk);
	cout << "Dimension of mesh: " << n << endl;
	cout << "Number of iterations " << num_iters << endl;
	cout << "Maximum step size for the integration " << step << endl;
	cout << "Delta for T1 swap " << delta << endl;
	cout << "An image will be printed every " << out_freq << " iterations." << endl;
    cout << "The maximum number of initial T1 swaps is: " << max_swaps << endl;
}

void ReadMeshChanges(vector<cell>& sim_cells)
{
    string comment;
    int index;
    double parameter;
    int num_gamma, num_area;
    ifstream changes("change_mesh.txt");
    getline(changes, comment);
    changes >> num_gamma >> num_area;
    for(int line = 0; line < num_gamma; line++)
    {
        changes >> index >> parameter;
        sim_cells[index].SetGamma(parameter);
    }
    for(int line = 0; line < num_area; line++)
    {
        changes >> index >> parameter;
        sim_cells[index].SetTargetArea(parameter);
    }
}
