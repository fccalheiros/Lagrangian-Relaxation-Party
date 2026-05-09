#include "algoritmo.h"
#include <iostream>



Algoritmo::Algoritmo(Configuration * config) :
	_manager(0),
	_tempoInicio(0),
	_tempoInicioCpu(0),
	_tempoFinal(0),
    _tempoFinalCpu(0),
	_iteracoes(0),
	_config(config)
{}

Algoritmo::~Algoritmo() {}

void Algoritmo::setLagrangianManager(LagrangianManager *mes){
	_manager = mes;
}

void Algoritmo::Inicializacao() { 
	_timer.Reset();
};

void Algoritmo::Finalizacao() {
	printf("Execution time : %3.2f s\n"
		   "CPU time       : %3.2f s\n"
		    "Iterations     : %d\n",
		    _timer.ElapsedSeconds(),
			_timer.CpuElapsedSeconds(),
		    _iteracoes
	);
}

