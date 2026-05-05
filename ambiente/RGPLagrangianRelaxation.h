#ifndef _RGPLagrangianRelaxation_H
#define _RGPLagrangianRelaxation_H

#include "LagrangianRelaxation.h"
#include "RGPVariable.h"


class RGPLagrangianRelaxation : public LagrangianRelaxation {
  
 public:
  
  RGPLagrangianRelaxation(Configuration * config);
  ~RGPLagrangianRelaxation();

  bool _primeira;
  int _vezes1;
  int _vezes2;
  float _maximofator;

  virtual void FixVariables(VariableSet &solRel, float valor, float InitialCost);
  virtual void SolveRelaxation(VariableSet& sol, float& valor, float InitialCost);
  virtual bool ColumnGeneration(VariableSet& relaxed, float& newLowerBound, float InitialCost);

  void Relaxacao2(VariableSet& sol, float& valor, float InitialCost);

  virtual Algoritmo* getNew() { return new RGPLagrangianRelaxation(_config); }
  
protected:

    inline virtual bool TemIntercessao(VariableSet& solHeu, Variable* var);
};


template <class T> struct ComparaArea  {
public:
    ComparaArea() { }

    bool  operator() (Variable* v1, Variable* v2) {
        // Deve verificar se v1 e v2 sao "ativas" 
        float custo1 = v1->_valorLag / ((float)((RGPVariable*)v1)->Area());
        float custo2 = v2->_valorLag / ((float)((RGPVariable*)v2)->Area());

        if (custo1 < custo2) {
            if (custo2 < 0) return false;
            return true;
        }

        if (custo2 < custo1) {
            if (custo1 < 0) return true;
            return false;
        }

        if (v1->_nome < v2->_nome)
            return true;

        return false;
    }

};

#endif
