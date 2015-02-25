COMP = g++ -std=c++11

all: NagaiHondaModel

NagaiHondaModel: NagaiHondaModel.o NagaiHondaForce.o cell.o Topology.o MoveVerts.o
	nvcc -o NagaiHondaModel  NagaiHondaModel.o NagaiHondaForce.o cell.o Topology.o MoveVerts.o 
	
NagaiHondaModel.o: NagaiHondaModel.cpp cell.hpp NagaiHondaForce.hpp coordinate.hpp printCellProperties.hpp EnergyCalculator.hpp
	$(COMP) -c NagaiHondaModel.cpp

Topology.o: Topology.cpp Topology.hpp
	$(COMP) -c Topology.cpp

NagaiHondaForce.o: NagaiHondaForce.cpp NagaiHondaForce.hpp cell.hpp coordinate.hpp Get_Assoc_Cells.hpp AreaForce.hpp PerAdhForce.hpp
	$(COMP) -c NagaiHondaForce.cpp

cell.o: cell.cpp cell.hpp coordinate.hpp
	$(COMP) -c cell.cpp

MoveVerts.o: MoveVerts.cu coordinate.hpp
	nvcc -c MoveVerts.cu

clean:
	rm *.o Images/*.off Images/*.rgb

