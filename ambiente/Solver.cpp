#include "Solver.h"
#include <iostream>



Solver::Solver(Configuration * config) :
	_manager(0),
	_tempoInicio(0),
	_tempoInicioCpu(0),
	_tempoFinal(0),
    _tempoFinalCpu(0),
	_iteracoes(0),
	_config(config)
{}

Solver::~Solver() {}

void Solver::setLagrangianManager(LagrangianManager *mes){
	_manager = mes;
}

void Solver::Inicializacao() { 
	_timer.Reset();
};

void Solver::Finalizacao() {
	printf("Execution time : %3.2f s\n"
		   "CPU time       : %3.2f s\n"
		    "Iterations     : %d\n",
		    _timer.ElapsedSeconds(),
			_timer.CpuElapsedSeconds(),
		    _iteracoes
	);
}

