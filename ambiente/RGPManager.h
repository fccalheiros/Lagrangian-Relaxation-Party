#ifndef _RGPManager_H
#define _RGPManager_H

#include "LagrangeanManager.h"
#include "Points.h"
#include "RGPVariable.h"

class RGPLagrangeanRelaxation;

class RGPManager : public LagrangeanManager {

 protected:

  Pontos _instancia;

  int _faixa;
  
  int _colunas;
  int _colunas2;
  int _colunas3;
  int _colunas4;

  bool _JaImprimiu;


 public:


    int _numeroPontos;
    double _uns;

    RGPManager(Configuration *config);
    RGPManager(Configuration* config, Algoritmo *algo, Direction direction = Direction::MINIMIZE);
    RGPManager(RGPManager* m);
    virtual LagrangeanManager* CopyAndClean(LagrangeanManager* m);

    virtual ~RGPManager();

    virtual void FinalStats();

    virtual void SetVariableForBranch(Variable* v, short int value);
    virtual void Solve(float InitialCost, float KnownBound);
    inline int Area() { return (_faixa*_faixa); }; 

    inline virtual string DefaultFilePrefix();

  
 protected:
  
    virtual void ReadProblem(char *arq);
    virtual void CreateProblem();
    virtual void CustomProcessing();
    virtual void PrintSolution();
    bool VerificaTerminalDentro(GridIter x1, GridIter y1, GridIter x2, GridIter y2);
    bool VerificaTerminalNoCorner(GridIter x1, GridIter y1, GridIter x2, GridIter y2);
    bool VerificaSteiners(GridIter x1, GridIter y1, GridIter x2, GridIter y2);
    void CreateVariable(GridIter x1, GridIter y1, GridIter x2, GridIter y2);
    virtual void InsertVariableIntoConstraint(Variable *var1);
  
    void ConstraintsReduction();

    void CalculaSegmentos(RGPVariable *var, int px1, int py1, int px2, int py2);

    void MarcaVariaveis(RGPVariable *var);
  

    int GuilhotinaRecursivo(int x1, int y1, int x2, int y2);
    int Guilhotina();

 private:

  char *novoNome(int x) {
    int tamanho=0;
    int salvaX = x;
    
    while ( x > 0 ) {
      tamanho++;
      x = x / 10;
    }
    if (tamanho == 0) tamanho++;
   
    x = salvaX;
    char *res = new char[tamanho+1];
    res[tamanho] = 0;
    while ( tamanho > 0 ) {
      tamanho--;
      res[tamanho] = '0'+ (x % 10);
      x = x / 10;
    }
    
    return res;
  }
  
};

class CompFixa  {
    public:
        CompFixa() { }
    
    bool  operator() (Variable *v1) {  
      
        return ! v1->_fixa;
      
    }

};
  
template <class T> struct  CompFixa2  {
public:
    CompFixa2() { } 
    bool  operator() (Variable *v1, Variable *v2) {     
        if ( v1->_fixa != v2->_fixa ) 
            return v2->_fixa;
        return ( v1->_nome < v2->_nome );
    }
};


#endif

