#ifndef _LAGRANGIAN_RELAXATION_H
#define _LAGRANGIAN_RELAXATION_H

#include "algoritmo.h"

class LagrangianRelaxation : public Algoritmo {

public:

	LagrangianRelaxation(Configuration * config);

	virtual ~LagrangianRelaxation();

	virtual void setLagrangianManager(LagrangianManager *mes);
  
	virtual void Inicializacao();
	virtual void SolveRelaxation(VariableSet& sol, float& valor, float InitialCost);
	virtual bool RunPrimalHeuristic(VariableSet &solRel, VariableSet &solHeu, float &valor, float InitialCost);
	virtual void GenerateCuts(VariableSet &solRel);
	virtual bool Price(VariableSet& relaxed);
	virtual bool ColumnGeneration(VariableSet& relaxed, float &newLowerBound, float InitialCost);
	virtual bool PricingTrigger();
	virtual void UpdateSubgradient(VariableSet &sol);
	virtual bool CheckStopCondition();
	virtual Variable* ChooseBranchVariable();
	void Restart();

	void ResetIterations() { _iteracoes = 0;  }

	virtual Algoritmo* getNew() { return new LagrangianRelaxation(_config); }

protected:

	void InicializacoesHeuristica();
	inline virtual bool TemIntercessao(VariableSet &solHeu, Variable *var); 
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
