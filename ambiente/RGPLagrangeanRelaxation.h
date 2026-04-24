#ifndef _RGPLagrangeanRelaxation_H
#define _RGPLagrangeanRelaxation_H

#include "LagrangeanRelaxation.h"
#include "RGPVariable.h"


class RGPLagrangeanRelaxation : public LagrangeanRelaxation {
  
 public:
  
  RGPLagrangeanRelaxation(Configuration * config);
  ~RGPLagrangeanRelaxation();

  bool _primeira;
  int _vezes1;
  int _vezes2;
  float _maximofator;

  virtual void FixaVariaveis(Solucao &solRel, float valor, float InitialCost);
  virtual void Relaxacao(Solucao& sol, float& valor, float InitialCost);

  void Relaxacao2(Solucao& sol, float& valor, float InitialCost);

  virtual Algoritmo* getNew() { return new RGPLagrangeanRelaxation(_config); }
  
protected:

    inline virtual bool TemIntercessao(Solucao& solHeu, Variable* var);
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
