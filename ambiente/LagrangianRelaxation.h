#ifndef _LAGRANGIAN_RELAXATION_H
#define _LAGRANGIAN_RELAXATION_H

#include "algoritmo.h"

class LagrangianRelaxation : public Algoritmo {

public:

	LagrangianRelaxation(Configuration * config);

	virtual ~LagrangianRelaxation();

	virtual void setLagrangianManager(LagrangianManager *mes);
  
	virtual void Inicializacao();
	virtual void SolveRelaxation(Solucao& sol, float& valor, float InitialCost);
	virtual bool RunPrimalHeuristic(Solucao &solRel, Solucao &solHeu, float &valor, float InitialCost);
	virtual void GenerateCuts(Solucao &solRel);
	virtual bool Price(Solucao& relaxed);
	virtual bool ColumnGeneration(Solucao& relaxed, float &newLowerBound, float InitialCost);
	virtual bool PricingTrigger();
	virtual void UpdateSubgradient(Solucao &sol);
	virtual bool CheckStopCondition();
	virtual Variable* ChooseBranchVariable();
	void Restart();

	void ResetIterations() { _iteracoes = 0;  }

	virtual Algoritmo* getNew() { return new LagrangianRelaxation(_config); }

protected:

	void InicializacoesHeuristica();
	inline virtual bool TemIntercessao(Solucao &solHeu, Variable *var); 
	void ComputeLagrangianCosts(float & multiplierSum);
	void ComputeReducedCosts(bool onlyActiveVariables);

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
