#ifndef _ALGORITMO_H
#define _ALGORITMO_H

#include <vector>

#include "Variable.h"
#include "Time.h"
#include "Configuration.h"

typedef vector<Variable *> VariableSet;

class LagrangianManager;

class Algoritmo {
  
 protected:
  
  LagrangianManager * _manager;
  Configuration * _config;

 public:

  long int _tempoInicio;
  clock_t  _tempoInicioCpu;

  long int _tempoFinal;
  clock_t _tempoFinalCpu;

  int _iteracoes;

  Algoritmo(Configuration * config);
  virtual ~Algoritmo();

  virtual void setLagrangianManager(LagrangianManager * mes);
  virtual void Inicializacao();
  virtual void Finalizacao();


  virtual void SolveRelaxation(VariableSet& sol, float& valor, float InitialCost) {};
  virtual bool RunPrimalHeuristic(VariableSet& solRel, VariableSet& solHeu, float& valor, float InitialCost) { return false; };
  virtual void GenerateCuts(VariableSet& solRel) {} ;
  virtual bool Price(VariableSet& relaxed) { return false; }
  virtual bool ColumnGeneration(VariableSet& relaxed, float& newLowerBound, float InitialCost) { return false; }
  virtual bool PricingTrigger() { return false; }
  virtual void UpdateSubgradient(VariableSet& sol) { };
  virtual void FixVariables(VariableSet& solRel, float valor, float InitialCost) {};
  virtual bool CheckStopCondition() { return true; }
  virtual Variable* ChooseBranchVariable() { return NULL;  }

  virtual Algoritmo* getNew() { return new Algoritmo(_config);  }

  inline float TotalRunTime() const { return ElapsedTime(_tempoInicio, _tempoFinal); }

};

template <class T = Variable*> struct  CompareLagrangian  {
public:

    bool  operator() (Variable* v1, Variable* v2) {

        // Deve verificar se v1 e v2 sao "ativas" 
        float custo1 = v1->_valorLag;
        float custo2 = v2->_valorLag;

        if (custo1 < custo2) return true;

        if (custo1 == custo2)
            if (v1->_nome < v2->_nome) return true;

        return false;
    }

};

template <class T = Variable*> struct  NotCompareLagrangian {
public:

    bool  operator() (Variable* v1, Variable* v2) {
        // Deve verificar se v1 e v2 sao "ativas" 
        float custo1 = v1->_valorLag;
        float custo2 = v2->_valorLag;

        if (custo1 > custo2) return true;

        if (custo1 == custo2)
            if (v1->_nome < v2->_nome) return true;

        return false;
    }

};


template <class T = Variable*> struct  LessCost {
public:

    bool  operator() (Variable* v1, Variable* v2) {

        // Deve verificar se v1 e v2 sao "ativas" 
        float custo1 = v1->_valorFO;
        float custo2 = v2->_valorFO;

        if (custo1 < custo2) return true;

        if (custo1 == custo2)
            if (v1->_nome < v2->_nome) return true;

        return false;
    }

};

template <class T = Variable*> struct  GreaterCost  {
public:

    bool  operator() (Variable* v1, Variable* v2) {
        // Deve verificar se v1 e v2 sao "ativas" 
        float custo1 = v1->_valorFO;
        float custo2 = v2->_valorFO;

        if (custo1 > custo2) return true;

        if (custo1 == custo2)
            if (v1->_nome < v2->_nome) return true;

        return false;
    }

};

template <class T> struct  ComparaCobertura  {
public:
    ComparaCobertura() { }

    bool  operator() (Variable* v1, Variable* v2) {
        // Deve verificar se v1 e v2 sao "ativas" 
        float custo1 = v1->_valorLag / v1->_linhasCobertas;
        float custo2 = v2->_valorLag / v2->_linhasCobertas;

        if (custo1 < custo2) {
            if (custo2 > 0)
                return true;
        }
        else if (custo2 < custo1) {
            if (custo1 < 0)
                return true;
        }
        else if (v1->_nome < v2->_nome)
            return true;

        return false;
    }

};

#endif
