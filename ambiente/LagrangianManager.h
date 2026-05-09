#ifndef _LagrangianManager_H
#define _LagrangianManager_H


#ifndef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif




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
#include "PartitionedVector.h"

// =====================
// Type aliases
// =====================

using VariableVector = PartitionedVector<Variable*, VariablePartitionPolicy>;
using VariableIterator = VariableVector::iterator;

typedef std::vector<Constraint*>::iterator ConstraintIterator;

// =====================
// Enums
// =====================
enum class Direction {
    MAXIMIZE,
    MINIMIZE
};

// =====================
// Helper functor wrapper
// =====================
template<class Tp, class _Function>
struct RestricaoAtiva {
public:
    RestricaoAtiva(_Function f) : _f(f) {}

    void operator()(Tp& restricao) {
        _f(restricao);
    }

    _Function _f;
};

// ============================================================
// LagrangianManager
// ============================================================
class LagrangianManager {

public:
    // ========================================================
    // Core data
    // ========================================================
    
    VariableVector _variables;

    std::vector<Constraint*> _constraints;
    std::vector<Constraint*> _constraintsND;
    std::vector<Constraint*> _cuts;

    std::vector<Variable*> _incumbentSolution;

    Algoritmo* _algo;

    // ========================================================
    // Counters
    // ========================================================
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
    // ========================================================
    // Configuration
    // ========================================================
    Direction _direction;
    Configuration* _config;

    // Sorting control
    size_t _max_sort_depth;
    SortThreadPool _pool;

private:
    // ========================================================
    // Bounds
    // ========================================================
    float _upperBound;
    float _lowerBound;

protected:
    
    // ========================================================
    // Internal logic
    // ========================================================
    void UpdateBounds(float valRelaxado, float valHeuristica,
        std::vector<Variable*>& solHeu, bool resHeuristica);

    void StoreIncumbent(VariableSet& sol);
    void AddToIncumbent(Variable* var);
    void ResetLagrangianCosts();

    // Hooks
    virtual void ReadProblem(char* arq) {}
    virtual void CreateProblem() {}
    virtual void PostProblemCreationPriceOut() {}
    virtual void InsertVariableIntoConstraint(Variable* var1) {}
    virtual void CustomProcessing() {}

    void FinalizeProblemCreation();

public:
    // ========================================================
	// Constructors / Initialization
    // ========================================================
    LagrangianManager(Configuration* config);

    LagrangianManager(Configuration* config,
        Algoritmo* algo,
        Direction direction = Direction::MINIMIZE,
        size_t max_sort_depth = 4);

    LagrangianManager(LagrangianManager* m);

    virtual LagrangianManager* CopyAndClean(LagrangianManager* m);

    virtual ~LagrangianManager();

    inline void SetAlgorithm(Algoritmo* algo) { _algo = algo; }

    // ========================================================
    // Solve
    // ========================================================
    virtual void Solve(float InitialCost, float KnownBound);

    inline bool OptimalFound() {
        return ((getUpperBound() - getLowerBound()) < _config->STOP_GAP);
    }
    // ========================================================
    // Bound helpers
    // ========================================================
    inline void setUpperBound(float UB) { _upperBound = UB; }
    inline void setLowerBound(float LB) { _lowerBound = LB; }

    inline void setPrimalBound(float B) {
        if (_direction == Direction::MINIMIZE) setUpperBound(B);
        else setLowerBound(B);
    }

    inline void setDualBound(float B) {
        if (_direction == Direction::MINIMIZE) setLowerBound(B);
        else setUpperBound(B);
    }

    inline float getUpperBound() const { return _upperBound; }
    inline float getLowerBound() const { return _lowerBound; }

    inline float getPrimalBound() const {
        if (_direction == Direction::MINIMIZE) return getUpperBound();
        else return getLowerBound();
    }

    inline float getDualBound() const {
        if (_direction == Direction::MINIMIZE) return getLowerBound();
        else return getUpperBound();
    }

    inline Direction getDirection() const { return _direction; }

    // ========================================================
    // Variable operations
    // ========================================================
    void FixVariable(VariableIterator var);
    void UnfixVariable(VariableIterator var);

    void PriceOutVariable(VariableIterator var);
    void PriceInVariable(VariableIterator var);

    void MarkVariableForPriceIn(VariableIterator var);
    void MarkVariableForPriceOut(VariableIterator var);

    void CommitPriceIn();
    void CommitPriceOut();

    void TestVariableVector();

    // ========================================================
    // Insert / Remove
    // ========================================================
    void InsertVariable(Variable* var);

    void InsertConstraint(Constraint* restricao);
    void InsertConstraintND(Constraint* restricao);
    void InsertCut(Constraint* constraint);

    void RemoveVariable(VariableIterator& it);
    void RemoveConstraint(ConstraintIterator& it);
    void RemoveConstraintND(ConstraintIterator& it);
    void RemoveCut(ConstraintIterator& it);

    void MarkConstraintForDeletion(Variable* var);

    // ========================================================
    // Ranges
    // ========================================================
    int getActiveVariablesCount()    { return _variables.ActiveCount();    }
    int getZeroFixedVariablesCount() { return _variables.FixedCount();     }
    int getPricedOutVariablesCount() { return _variables.PricedOutCount(); }

    void GetActiveVariablesRange(VariableIterator& begin, VariableIterator& end);
    void GetZeroFixedVariablesRange(VariableIterator& begin, VariableIterator& end);
    void GetPricedOutVariablesRange(VariableIterator& begin, VariableIterator& end);

    void GetConstraintRange(ConstraintIterator& begin, ConstraintIterator& end);
    void GetNDConstraintRange(ConstraintIterator& begin, ConstraintIterator& end);
    void GetCutsRange(ConstraintIterator& begin, ConstraintIterator& end);

    // ========================================================
    // Access
    // ========================================================
    Constraint* getConstraint(int i);
    Constraint* getConstraintND(int i);
    Constraint* getCut(int i);

    // ========================================================
	// B&B utilities
    // ========================================================
    void StoreIncumbentfromBranchAndBound(VariableSet& set) { StoreIncumbent(set); }
    void AddToIncumbentfromBranchAndBound(Variable* var) { AddToIncumbent(var); }
    virtual void SetVariableForBranch(Variable* v, short int value);

    // ========================================================
    // Memory / lifecycle
    // ========================================================
    void FreeMemory();
    void GenerateProblem(char* arq);

    void CleanupDeletedConstraints();
    void CleanUpProblem();
    void CleanUp();
    void Restart();

    // ========================================================
    // Debug / output
    // ========================================================
    void CoveredConstraints(Variable* var, std::vector<Constraint*>& linhas);

    void ImprimeLP(FILE* saida);
    std::string PrintLP();
    void PrintLP(std::string filename);
    std::string PrintVariableVector(VariableSet s);
    void PrintLagrangian();
    virtual void PrintSolution() {};
    virtual void FinalStats();
    inline double TotalRunTime() const { return _algo->TotalRunTime(); }

    inline virtual std::string DefaultFilePrefix() { return "LagrangianManager"; }

    void CheckConstraints(VariableSet& sol);

    float GetMaxLagrangian(VariableSet sol) {
        VariableIterator it = std::max_element(sol.begin(), sol.end(), CompareLagrangian<Variable*>());
        return (*it)->_valorLag;
    }

public:
    // ==================
    // Sorting utilities 
    // ==================
    int max_sort_size_thread = 2500;
    int max_sort_size = 64;

    template <class StrictWeakOrdering>
    void EstOrdemVariaveis(int posicao, StrictWeakOrdering comp) {
        VariableIterator begin, end;
        GetActiveVariablesRange(begin, end);
        if (std::distance(begin, end) > 200)
            std::nth_element(begin, begin + posicao, end, comp);
        else
            std::sort(begin, end, comp);
    }

    template <class StrictWeakOrdering>
    void EstOrdemVariaveis2(int posicao, StrictWeakOrdering comp) {
        int p = 4 * posicao;
        VariableIterator begin, end;
        GetActiveVariablesRange(begin, end);
        if (std::distance(begin, end) > p) {
            std::nth_element(begin, begin + p, end, comp);
            std::sort(begin, begin + p, comp);
        }
        else
            std::sort(begin, end, comp);
    }

    template <class StrictWeakOrdering>
    void Ordena(StrictWeakOrdering comp) {
        VariableIterator begin, end;
        GetActiveVariablesRange(begin, end);
        std::sort(begin, end, comp);
    }

    template <class StrictWeakOrdering>
    void Ordena2(StrictWeakOrdering comp) {
        VariableIterator begin, end;
        GetActiveVariablesRange(begin, end);
        OrdenaRecursivo(comp, begin, end, 0);
    }

    template <class StrictWeakOrdering>
    void Ordena3(StrictWeakOrdering comp) {
        VariableIterator begin, end;
        GetActiveVariablesRange(begin, end);
        ThreadMergeSort(_pool, comp, begin, end, 0, _max_sort_depth);
    }

    template <class StrictWeakOrdering>
    void OrdenaRecursivo(StrictWeakOrdering comp, VariableIterator inicio, VariableIterator fim, int profundidade) {
        if ((profundidade < 15) && ((fim - inicio) > max_sort_size)) {
            int meio = (int)((fim - inicio) / 2);
            OrdenaRecursivo(comp, inicio, inicio + meio + 1, profundidade + 1);
            OrdenaRecursivo(comp, inicio + meio + 1, fim, profundidade + 1);

            VariableIterator it = std::lower_bound(inicio, inicio + meio + 1,
                *(inicio + meio + 1), comp);

            std::inplace_merge(it, inicio + meio + 1, fim, comp);
        }
        else {
            std::sort(inicio, fim, comp);
        }
    }

    template <class StrictWeakOrdering>
    void ThreadMergeSort(SortThreadPool& pool,
        StrictWeakOrdering comp,
        VariableIterator begin,
        VariableIterator end,
        int depth,
        size_t max_depth) {

        auto size = end - begin;

        if (size <= max_sort_size) {
            std::sort(begin, end, comp);
            return;
        }

        VariableIterator mid = begin + size / 2;

        if (size > max_sort_size_thread && depth < max_depth) {
            auto fut = pool.enqueue([this, &pool, comp, begin, mid, depth, max_depth] {
                ThreadMergeSort(pool, comp, begin, mid, depth + 1, max_depth);
                });

            ThreadMergeSort(pool, comp, mid, end, depth + 1, max_depth);
            fut.get();
        }
        else {
            OrdenaRecursivo(comp, begin, mid, depth + 1);
            OrdenaRecursivo(comp, mid, end, depth + 1);
        }

        VariableIterator it = std::lower_bound(begin, mid, *mid, comp);
        std::inplace_merge(it, mid, end, comp);
    }

    template <class RandomAccessIterator, class StrictWeakOrdering>
    void pop_heap_t(RandomAccessIterator last, StrictWeakOrdering comp) {
        VariableIterator begin, end;
        GetActiveVariablesRange(begin, end);
        std::pop_heap(begin, last, comp);
    }

    template <class StrictWeakOrdering>
    void make_heap_t(StrictWeakOrdering comp) {
        VariableIterator begin, end;
        GetActiveVariablesRange(begin, end);
        std::make_heap(begin, end, comp);
    }

    template <class StrictWeakOrdering>
    void sort_heap_t(StrictWeakOrdering comp) {
        VariableIterator begin, end;
        GetActiveVariablesRange(begin, end);
        std::sort_heap(begin, end, comp);
    }

    // ========================================================
    // Iterators helpers
    // ========================================================
    template <class F>
    F for_each_variable(F f) {
        VariableIterator begin, end;
        GetActiveVariablesRange(begin, end);
        return for_each(begin, end, f);
    }

    template <class _Function>
    _Function for_each_constraint(_Function __f) {
        ConstraintIterator begin, end;
        GetConstraintRange(begin, end);
        RestricaoAtiva<Constraint*, _Function> tmp =
            for_each(begin, end, RestricaoAtiva<Constraint*, _Function>(__f));
        return tmp._f;
    }

    template <class _Function>
    _Function for_each_non_dualized_constraint(_Function __f) {
        ConstraintIterator begin, end;
        GetNDConstraintRange(begin, end);
        RestricaoAtiva<Constraint*, _Function> tmp =
            for_each(begin, end, RestricaoAtiva<Constraint*, _Function>(__f));
        return tmp._f;
    }

    template <class _Function>
    _Function for_each_cut(_Function __f) {
        ConstraintIterator begin, end;
        GetCutsRange(begin, end);
        RestricaoAtiva<Constraint*, _Function> tmp =
            for_each(begin, end, RestricaoAtiva<Constraint*, _Function>(__f));
        return tmp._f;
    }
};

#endif

