#ifndef _RGPManager_H
#define _RGPManager_H

#include "LagrangianManager.h"
#include "Points.h"
#include "RGPVariable.h"

class RGPLagrangianRelaxation;

class RGPManager : public LagrangianManager {

 protected:

    Points _instancePoints;

    int _gridSize;
	int _instanceNumber = 0;
  
    int _colunas;
    int _colunas2;
    int _colunas3;
    int _colunas4;

    bool _JaImprimiu;
    float _cutoffForPricing = 0.0f;


 public:

    int _numPoints;

    RGPManager(Configuration *config);
    RGPManager(Configuration* config, Algoritmo *algo, Direction direction = Direction::MINIMIZE);
    RGPManager(RGPManager* m);
    virtual LagrangianManager* CopyAndClean(LagrangianManager* m);

    virtual ~RGPManager();

    virtual void FinalStats();

    virtual void SetVariableForBranch(Variable* v, short int value);
    virtual void Solve(float InitialCost, float KnownBound);
    inline int Area() { return (_gridSize*_gridSize); }; 

    inline virtual string DefaultFilePrefix();
    int getCardinality() { return _numPoints + 1; };
    virtual void PrintSolution(string filename);

  
 protected:
  
    virtual void ReadProblem(char *arq);
    virtual void CreateProblem();
	virtual void PostProblemCreationPriceOut();
    virtual void CustomProcessing();
    bool VerificaTerminalDentro(GridIterator x1, GridIterator y1, GridIterator x2, GridIterator y2);
    bool VerificaTerminalNoCorner(GridIterator x1, GridIterator y1, GridIterator x2, GridIterator y2);
    bool VerificaSteiners(GridIterator x1, GridIterator y1, GridIterator x2, GridIterator y2);
    void CreateVariable(GridIterator x1, GridIterator y1, GridIterator x2, GridIterator y2);
    virtual void InsertVariableIntoConstraint(Variable *var1);
    void ExtractInstanceNumber(const char* filename);
  
    void PostGenerationConstraintsReduction();

    void CalculaSegmentos(RGPVariable *var, int px1, int py1, int px2, int py2);

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
#endif

