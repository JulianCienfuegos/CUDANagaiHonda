#include <vector>

int NagaiHondaForce(std::vector<coordinate>& coordinate_list,
		     std::vector<cell>& simulation_cells, double step, double delta,
             std::vector<double>& tx, std::vector<double>& ty);
