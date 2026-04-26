#ifndef _LagrangianManager_H
#define _LagrangianManager_H


#define _CRT_SECURE_NO_WARNINGS

using namespace std;

#include <stdio.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include <string>
#include <sstream>

#include "Variable.h"
#include "Constraint.h"
#include "algoritmo.h"
#include "SortThreadPool.h"


typedef vector<Variable*>::iterator VariableIterator;
typedef vector<Constraint*>::iterator ConstraintIterator;
typedef vector<Variable*> VariableSet;

enum class Direction {
    MAXIMIZE,
    MINIMIZE
};

template<class Tp, class _Function> struct VariavelValida 
{
public:

    VariavelValida(_Function f) : _f(f) { };

    void operator() (Tp& var) {
        _f(var);
    };
    _Function _f;
};

/*****************************************************/

template<class Tp, class _Function> struct RestricaoAtiva 
{
public:
    RestricaoAtiva(_Function f) : _f(f) { };

    void operator() (Tp& restricao) {
        _f(restricao);
    };
    _Function _f;
};

/*****************************************************/


class LagrangianManager {

public:

    vector<Variable*> _variables;
    vector<Constraint*> _constraints;
    vector<Constraint*> _constraintsND;
    vector<Constraint*> _cuts;

    vector<Variable*> _best;

    Algoritmo* _algo;

    int _countConstraints;
    int _countConstraintsND;

    int _cutsInserted;
    int _cutsFound;
    int _cutsRemoved;
    int _totalVariaveis;
    int _countFixed;
    int _countFixedPartial;
    
    double _nonZeroCount = 0;

protected:

    

    Direction _direction;
    Configuration* _config;
    size_t _max_sort_depth;
    SortThreadPool _pool;

private:

    float _upperBound;
    float _lowerBound;

    VariableIterator _activeVariablesEnd;

protected:

    inline void setUpperBound(float UB) { _upperBound = UB; }
    inline void setLowerBound(float LB) { _lowerBound = LB; }
    inline void setBound(float B)       { if (_direction == Direction::MINIMIZE) setUpperBound(B); 
                                          else setLowerBound(B); }
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

    LagrangianManager(Configuration *config);
    LagrangianManager(Configuration* config, Algoritmo *algo, Direction direction = Direction::MINIMIZE, size_t max_sort_depth = 4 );
    LagrangianManager(LagrangianManager* m);
    virtual LagrangianManager* CopyAndClean(LagrangianManager* m);

    virtual ~LagrangianManager();

    virtual void Solve(float InitialCost, float KnownBound);

    void FixVariable(VariableIterator var);
    void FixLastVariable();

    void InsertVariable(Variable *var);
    void InsertConstraint(Constraint *restricao);
    void InsertConstraintND(Constraint *restricao);
    void InsertCut(Constraint *constraint);

    void RemoveVariable(VariableIterator &it);
    void RemoveConstraint(ConstraintIterator &it);
    void RemoveConstraintND(ConstraintIterator &it);
    void RemoveCut(ConstraintIterator &it);

    void MarkConstraintForDeletion(Variable* var);

    int  ActiveVariables();
    void GetActiveVariableRange(VariableIterator &begin, VariableIterator &end);
    void GetConstraintRange(ConstraintIterator &begin, ConstraintIterator &end);
    void GetNDConstraintRange(ConstraintIterator &begin, ConstraintIterator &end);
    void GetCutsRange(ConstraintIterator &begin, ConstraintIterator &end);

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
    void PrintLagrangian();

    void CleanupDeletedConstraints();
    void CleanUpProblem();
    void Restart();

    virtual void FinalStats();
    void CleanUp();
    int  Audit();

    virtual void SetVariableForBranch(Variable* v, short int value);

    inline bool OptimalFound() { return ( (getUpperBound() - getLowerBound()) < _config->STOP_GAP); }

    inline virtual string DefaultFilePrefix() { return "LagrangianManager"; }

    void CheckConstraints(Solucao& sol);

     float GetMaxLagrangian(VariableSet sol) {
         VariableIterator it = max_element(sol.begin(),sol.end(), CompareLagrangian <Variable*>());
         return (*it)->_valorLag;
     }
 
public:

    template <class StrictWeakOrdering> void EstOrdemVariaveis(int posicao, StrictWeakOrdering comp) {
        VariableIterator begin, end;
		GetActiveVariableRange(begin, end);
        if ( distance(begin,end) > 200)
            nth_element(begin, begin + posicao, end, comp);
        else
            sort(begin, end, comp);
    };

    template <class StrictWeakOrdering> void EstOrdemVariaveis2(int posicao, StrictWeakOrdering comp) {
        int p = 4 * posicao;
        VariableIterator begin, end;
        GetActiveVariableRange(begin, end);
        if ( distance(begin,end)  > p) {
            nth_element(begin, begin + p, end, comp);
            sort(begin, begin + p, comp);
        }
        else
            sort(begin, end, comp);
    };


    template <class StrictWeakOrdering> void Ordena(StrictWeakOrdering comp) {
        VariableIterator begin, end;
        GetActiveVariableRange(begin, end);
        sort(begin, end, comp);
    };

    template <class StrictWeakOrdering> void Ordena2(StrictWeakOrdering comp) {
        VariableIterator begin, end;
        GetActiveVariableRange(begin, end); 
        OrdenaRecursivo(comp, begin, end, 0);
        //if ( ! is_sorted(begin, end ,comp) ) { cout << "OPS !!!" << endl; exit(1); }
    }

    template <class StrictWeakOrdering> void Ordena3(StrictWeakOrdering comp) {
        VariableIterator begin, end;
        GetActiveVariableRange(begin, end); 
        ThreadMergeSort(_pool, comp, begin, end, 0, _max_sort_depth);
        //if ( ! is_sorted(begin, end ,comp) ) { cout << "OPS !!!" << endl; exit(1); }
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
        // OutputDebugStringA((std::string("Execução na Thread: ") + std::to_string(tid) + " " +  std::to_string(depth) + "\r\n").c_str());
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

        //OutputDebugStringA((std::string("Fim Thread: ") + std::to_string(tid) + " " + std::to_string(depth) + "\r\n").c_str());

    }

    template <class RandomAccessIterator, class StrictWeakOrdering> void pop_heap_t(RandomAccessIterator last,StrictWeakOrdering comp) {
        VariableIterator begin, end;
        GetActiveVariableRange(begin, end);
        pop_heap(begin, last, comp);
    }  

    template <class StrictWeakOrdering> void make_heap_t(StrictWeakOrdering comp) {
        VariableIterator begin, end;
        GetActiveVariableRange(begin, end);     
        make_heap(begin, end, comp);
    }

    template <class StrictWeakOrdering> void sort_heap_t(StrictWeakOrdering comp) {
        VariableIterator begin, end;
        GetActiveVariableRange(begin, end);
        sort_heap(begin, end, comp);
    }

    template <class _Function>
    _Function for_each_variable(_Function __f) {
        VariableIterator begin, end;
        GetActiveVariableRange(begin, end);
        VariavelValida<Variable * , _Function> tmp =
            for_each(begin, end, VariavelValida <Variable *,_Function> (__f) );
            return tmp._f;
    };
  
    template <class _Function>
    _Function for_each_constraint(_Function __f) {
		ConstraintIterator begin, end;
		GetConstraintRange(begin, end);
        RestricaoAtiva<Constraint *,_Function> tmp =
            for_each(begin, end, RestricaoAtiva <Constraint* ,_Function> (__f) );
        return tmp._f;
    };

    template <class _Function>
    _Function for_each_non_dualized_constraint(_Function __f) {
        ConstraintIterator begin, end;
		GetNDConstraintRange(begin, end);
        RestricaoAtiva<Constraint *,_Function> tmp =
            for_each(begin, end, RestricaoAtiva <Constraint* ,_Function> (__f) );
        return tmp._f;
    };

    template <class _Function>
    _Function for_each_cut(_Function __f) {
        ConstraintIterator begin, end;
        GetCutsRange(begin, end); 
        RestricaoAtiva<Constraint *,_Function> tmp =
            for_each(begin, end, RestricaoAtiva <Constraint* ,_Function> (__f) );
        return tmp._f;
    };

};
#endif
