#include "algoritmo.h"
#include <iostream>

using namespace std;

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
	_tempoInicio = TempoAtual(); 
	_tempoInicioCpu = TempoCpuAtual();
	cout << "Tempo de Inicio: " << _tempoInicio << "  " << _tempoInicioCpu << endl; 
	cout << "CLOCKS_PER_SEC: " << CLOCKS_PER_SEC << endl;
};

void Algoritmo::Finalizacao() {
	_tempoFinal = TempoAtual();
	_tempoFinalCpu = TempoCpuAtual();
	printf("Tempo de execucao: %3.2f\nCpu Time: %3.2f\nCiclos de Clock atual: %f\nIteracoes: %d\n",
		ElapsedTime(_tempoInicio,_tempoFinal), CPUElapsedTime(_tempoInicioCpu,_tempoFinalCpu), (double)CLOCKS_PER_SEC, _iteracoes);
}

void Algoritmo::Relaxacao(Solucao &sol, float &valor, float InitialCost){}
bool Algoritmo::Heuristica(Solucao &solRel, Solucao &solHeu, float &valor, float InitialCost) { return false; }
bool Algoritmo::Price(Solucao& relaxed) { return false;  }
void Algoritmo::GeraCortes(Solucao &solRel){}
void Algoritmo::SubGradiente(Solucao &sol) {}
void Algoritmo::FixaVariaveis(Solucao &solRel, float valor, float InitialCost){}
bool Algoritmo::TesteParada() { return true; }
