#ifndef _LagrangeanManager_H
#define _LagrangeanManager_H


#define _CRT_SECURE_NO_WARNINGS

using namespace std;

#include <stdio.h>
#include <iostream>

#include <vector>
#include <algorithm>
#include "Variable.h"
#include "Constraint.h"
#include "RGPCut.h"
#include "algoritmo.h"
#include "SortThreadPool.h"

#include <Windows.h>
#include <string>
#include <sstream>


typedef vector<Variable*>::iterator VariableIterator;
typedef vector<Constraint*>::iterator ConstraintIterator;
typedef vector<Variable*> VariableSet;

enum class Direction {
    MAXIMIZE,
    MINIMIZE
};

template<class Tp, class _Function> struct VariavelValida : public unary_function<Tp, void>
{
public:

    VariavelValida(_Function f) : _f(f) { };

    void operator() (Tp& var) {
        _f(var);
    };
    _Function _f;
};

/*****************************************************/

template<class Tp, class _Function> struct RestricaoAtiva : public unary_function<Tp, void>
{
public:
    RestricaoAtiva(_Function f) : _f(f) { };

    void operator() (Tp& restricao) {
        _f(restricao);
    };
    _Function _f;
};

/*****************************************************/


class LagrangeanManager {

public:

    vector<Variable*> _variables;
    vector<Constraint*> _constraints;
    vector<Constraint*> _constraintsND;
    vector<Constraint*> _cuts;

    vector<Variable*> _best;
    VariableIterator _end;

    Algoritmo* _algo;

    int _countConstraints;
    int _countConstraintsND;

    int _cutsInserted;
    int _cutsFound;
    int _cutsRemoved;
    int _totalVariaveis;
    int _countFixed;
    int _countFixedPartial;

protected:

    Direction _direction;
    Configuration* _config;
    size_t _max_sort_depth;
    SortThreadPool _pool;

private:

    float _upperBound;
    float _lowerBound;

protected:

    inline void setUpperBound(float UB) { _upperBound = UB; }
    inline void setLowerBound(float LB) { _lowerBound = LB; }
    inline void setBound(float B)       { if (_direction == Direction::MINIMIZE) setUpperBound(B); else setLowerBound(B); }
    inline Direction getDirection()  const  { return _direction; }

    void StoreIncumbent(Solucao &sol);

    void CheckBounds(float valRelaxado, float valHeuristica, vector <Variable*>& solHeu, bool resHeuristica);
    virtual void ReadProblem(char* arq) {};
    virtual void CreateProblem() { };
    virtual void InsertVariableIntoConstraint(Variable* var1) {};
    virtual void CustomProcessing() { };
    virtual void PrintSolution() {};

    void FinalizeProblemCreation();
 

public:

    LagrangeanManager(Configuration *config);
    LagrangeanManager(Configuration* config, Algoritmo *algo, Direction direction = Direction::MINIMIZE, size_t max_sort_depth = 4 );
    LagrangeanManager(LagrangeanManager* m);
    virtual LagrangeanManager* CopyAndClean(LagrangeanManager* m);

    virtual ~LagrangeanManager();

    virtual void Solve(float InitialCost, float KnownBound);

    void FixVariable(VariableIterator var);
    void FixLastVariable();

    void InsertVariable(Variable *var);
    void InsertConstraint(Constraint *restricao);
    void InsertConstraintND(Constraint *restricao);
    void InsertCut(Constraint *corte);

    void RemoveVariable(VariableIterator &it);
    void RemoveConstraint(ConstraintIterator &it);
    void RemoveConstraintND(ConstraintIterator &it);
    void RemoveCut(ConstraintIterator &it);

    void MarkConstraintForDeletion(Variable* var);

    int  ActiveVariables();
    void VariableBounds(VariableIterator &comeco, VariableIterator &fim);
    void ConstraintsBounds(ConstraintIterator &comeco, ConstraintIterator &fim);
    void ConstraintsNDBounds(ConstraintIterator &comeco, ConstraintIterator &fim);
    void CutsBounds(ConstraintIterator &comeco, ConstraintIterator &fim);

    Constraint * getConstraint (int i);
    Constraint * getConstraintND (int i);
    Constraint * getCut (int i);
  
    void FreeMemory();
    void GenerateProblem(char *arq);
 
    inline float getUpperBound() const { return _upperBound;  }
    inline float getLowerBound() const { return _lowerBound;  }
    inline float getBound() const { if (_direction == Direction::MINIMIZE) return getUpperBound(); else return getLowerBound(); }

    inline void SetAlgorithm(Algoritmo* algo) { _algo = algo; }
    inline float TotalRunTime() const { return _algo->TotalRunTime();  }

    void CoveredConstraints(Variable *var, vector <Constraint *> &linhas);

    void ImprimeLP(FILE *saida);
    string PrintLP();
    void PrintLP(string filename);
    string PrintVariableVector(Solucao s);
    void PrintLagrangean();

    void CleanUpProblem();
    void Restart();

    virtual void FinalStats();
    void CleanUp();
    int  Audit();

    virtual void SetVariableForBranch(Variable* v, short int value);

    inline bool OptimalFound() { return ( (getUpperBound() - getLowerBound()) < _config->STOP_GAP); }

    inline virtual string DefaultFilePrefix() { return "LagrangeanManager"; }


    void CheckConstraints(Solucao& sol);

 public:

     float GetMaxLagrangean(VariableSet sol) {
         VariableIterator it = max_element(sol.begin(),sol.end(), CompareLagrangean <Variable*>());
         return (*it)->_valorLag;
     }

    template <class StrictWeakOrdering> void EstOrdemVariaveis(int posicao, StrictWeakOrdering comp) {

        if ( distance(_variables.begin(),_end) > 200)
            nth_element(_variables.begin(), _variables.begin() + posicao, _end, comp);
        else
            sort(_variables.begin(), _end, comp);
    };

    template <class StrictWeakOrdering> void EstOrdemVariaveis2(int posicao, StrictWeakOrdering comp) {
        int p = 4 * posicao;
        if ( distance(_variables.begin(),_end)  > p) {
            nth_element(_variables.begin(), _variables.begin() + p, _end, comp);
            sort(_variables.begin(), _variables.begin() + p, comp);
        }
        else
            sort(_variables.begin(), _end, comp);
    };


    template <class StrictWeakOrdering> void Ordena(StrictWeakOrdering comp) {
        sort(_variables.begin(),_end,comp);
    };

    template <class StrictWeakOrdering> void Ordena2(StrictWeakOrdering comp) {
        VariableIterator inicio = _variables.begin();
        VariableIterator fim    = _end;
        OrdenaRecursivo(comp, inicio, fim, 0);
        //if ( ! is_sorted(inicio, fim ,comp) ) { cout << "OPS !!!" << endl; exit(1); }
    }

    template <class StrictWeakOrdering> void Ordena3(StrictWeakOrdering comp) {
        VariableIterator inicio = _variables.begin();
        VariableIterator fim = _end;
        ThreadMergeSort(_pool, comp, inicio, fim, 0, _max_sort_depth);
        //if ( ! is_sorted(inicio, fim ,comp) ) { cout << "OPS !!!" << endl; exit(1); }
    }

    // parâmetros configuráveis
    int max_sort_size_thread = 500; // mínimo para usar outra thread
    int max_sort_size = 64;  // mínimo para usar std::sort

    template <class StrictWeakOrdering> void OrdenaRecursivo(StrictWeakOrdering comp, VariableIterator inicio, VariableIterator fim, int profundidade) {
        if ( ( profundidade < 15 ) && ( (fim-inicio) > max_sort_size ) ) {
            int meio =  (int)((fim - inicio)/2);
            OrdenaRecursivo(comp, inicio, inicio+meio+1, profundidade+1);
            OrdenaRecursivo(comp, inicio+meio+1, fim, profundidade+1);
            VariableIterator it = lower_bound( inicio, inicio+meio+1, *(inicio+meio+1), comp);
            inplace_merge(it, inicio+meio+1 , fim , comp);
        }
        else sort (inicio, fim, comp);
    };
    
    template <class StrictWeakOrdering>
    void ThreadMergeSort(SortThreadPool& pool, StrictWeakOrdering comp,
        VariableIterator begin, VariableIterator end, int depth, size_t max_depth) {
        auto size = end - begin;

        DWORD tid = GetCurrentThreadId();
        OutputDebugStringA((std::string("Execução na Thread: ") + std::to_string(tid) + " " +  std::to_string(depth) + "\r\n").c_str());
        if (size <= max_sort_size) {
            std::sort(begin, end, comp);
            return;
        }

        VariableIterator mid = begin + size / 2;
        if (size > max_sort_size_thread && depth < max_depth) {
			// first half in new thread
            auto fut = pool.enqueue([this, &pool, comp, begin, mid, depth, max_depth] {
                ThreadMergeSort(pool, comp, begin, mid, depth + 1, max_depth);
                });

            // secont half in current thread
            ThreadMergeSort(pool, comp, mid, end, depth + 1, max_depth);
            fut.get(); // espera thread terminar
        }
        else {
			// threadless execution
            OrdenaRecursivo(comp, begin, mid, depth + 1);
            OrdenaRecursivo(comp, mid, end, depth + 1);
        }

        // Optimized merge works well in a quasi-sorted array
        VariableIterator it = std::lower_bound(begin, mid, *mid, comp);
        std::inplace_merge(it, mid, end, comp);

        OutputDebugStringA((std::string("Fim Thread: ") + std::to_string(tid) + " " + std::to_string(depth) + "\r\n").c_str());

    }

    template <class RandomAccessIterator, class StrictWeakOrdering> void retira_heap(RandomAccessIterator last,StrictWeakOrdering comp) {
        pop_heap(_variables.begin(),last,comp);
    }  

    template <class StrictWeakOrdering> void faz_heap(StrictWeakOrdering comp) {
        make_heap(_variables.begin(),_end,comp);
    }

    template <class StrictWeakOrdering> void ordena_heap(StrictWeakOrdering comp) {
        sort_heap(_variables.begin(),_end,comp);
    }

    template <class _Function>
    _Function para_cada_variavel(_Function __f) {
    VariavelValida<Variable * , _Function> tmp =
        for_each(_variables.begin(),_end,VariavelValida <Variable *,_Function> (__f) );
        return tmp._f;
    };
  
    template <class _Function>
    _Function para_cada_restricao(_Function __f) {
        RestricaoAtiva<Constraint *,_Function> tmp =
            for_each(_constraints.begin(),_constraints.end(),RestricaoAtiva <Constraint* ,_Function> (__f) );
        return tmp._f;
    };

    template <class _Function>
    _Function para_cada_restricaoND(_Function __f) {
        RestricaoAtiva<Constraint *,_Function> tmp =
            for_each(_constraintsND.begin(),_constraintsND.end(),RestricaoAtiva <Constraint* ,_Function> (__f) );
        return tmp._f;
    };

    template <class _Function>
    _Function para_cada_corte(_Function __f) {
        RestricaoAtiva<Constraint *,_Function> tmp =
            for_each(_cuts.begin(),_cuts.end(),RestricaoAtiva <Constraint* ,_Function> (__f) );
        return tmp._f;
    };

};
#endif
