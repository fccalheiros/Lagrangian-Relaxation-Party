#ifndef _LAGRANGIAN_RELAXATION_H
#define _LAGRANGIAN_RELAXATION_H

#include "algoritmo.h"

class LagrangianRelaxation : public Algoritmo {

public:

	LagrangianRelaxation(Configuration * config);

	virtual ~LagrangianRelaxation();

	virtual void setLagrangianManager(LagrangianManager *mes);
  
	virtual void Inicializacao();
	virtual void Relaxacao(Solucao& sol, float& valor, float InitialCost);
	virtual bool Heuristica(Solucao &solRel, Solucao &solHeu, float &valor, float InitialCost);
	virtual void GeraCortes(Solucao &solRel);
	virtual bool Price(Solucao& relaxed);
	virtual void SubGradiente(Solucao &sol);
	virtual bool TesteParada();
	virtual Variable* ChooseBranchVariable();
	void Restart();

	void ResetIterations() { _iteracoes = 0;  }

	virtual Algoritmo* getNew() { return new LagrangianRelaxation(_config); }

protected:

	void InicializacoesHeuristica();
	inline virtual bool TemIntercessao(Solucao &solHeu, Variable *var); 
	void InicializaRelaxacao(float &soma);

	Variable* ChooseBranchVariableLowLagrangian();
	Variable* ChooseBranchVariableHighIncumbentCost();

 protected:

	float _ultimoLI;
	float _ultimoLIHeuristica;
	float _alfa;
	float _lambda;
	int _naoMudouLI;

  
	float _somaMultiplicadores;

	bool _ordenou;
	bool _reStart;
  
 protected:

	Variable *_fracionario;
	float _valorFracionario;
  
};




#endif
