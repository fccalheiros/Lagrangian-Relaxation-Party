#ifndef _LagrangianManager_H
#define _LagrangianManager_H

#include <stdio.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include <string>
#include <sstream>
#include <algorithm>

#include "Variable.h"
#include "Constraint.h"
#include "Solver.h"
#include "ParallelSort.h"
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

    Solver* _solver;

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
    std::shared_ptr<ParallelSorter> _sorter;

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
    LagrangianManager(Configuration* config, std::shared_ptr<ParallelSorter> sorter);

    LagrangianManager(Configuration* config,
        Solver* solver,
        std::shared_ptr<ParallelSorter> sorter,
        Direction direction = Direction::MINIMIZE,
        size_t max_sort_depth = 4
    );

    LagrangianManager(LagrangianManager* m);

    virtual LagrangianManager* CopyAndClean(LagrangianManager* m);

    virtual ~LagrangianManager();

    inline void SetSolver(Solver* solver) { _solver = solver; }

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
    int getActiveVariablesCount() const { return _variables.ActiveCount(); }
    int getZeroFixedVariablesCount() const { return _variables.FixedCount();     }
    int getPricedOutVariablesCount() const { return _variables.PricedOutCount(); }

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
	virtual float ComputeChildPriority(Variable* v, short int value) const { 
		float priority = value == 0 ?  (std::max)(- v->_lagrangianCost, 0.0f) : (std::max)(v->_lagrangianCost, 0.0f);
        return ( getLowerBound() + priority ); 
    }

    // ========================================================
    // Memory / lifecycle
    // ========================================================
    void ReleaseRuntimeResources();
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
    inline double TotalRunTime() const { return _solver->TotalRunTime(); }

    inline virtual std::string DefaultFilePrefix() { return "LagrangianManager"; }

    void CheckConstraints(VariableSet& sol);

    float GetMaxLagrangian(VariableSet sol) {
        VariableIterator it = std::max_element(sol.begin(), sol.end(), CompareLagrangian<Variable*>());
        return (*it)->_lagrangianCost;
    }

public:
    // ==================
    // Sorting utilities 
    // ==================

    template <class StrictWeakOrdering>
    void NthElementCustom(int rank, StrictWeakOrdering comp) {
        VariableIterator begin, end;
        GetActiveVariablesRange(begin, end);
        if (std::distance(begin, end) > rank) {
            std::nth_element(begin, begin + rank, end, comp);
            std::sort(begin, begin + rank, comp);
        }
        else
            std::sort(begin, end, comp);
    }

    template <class StrictWeakOrdering>
    void SortActiveVariables(StrictWeakOrdering comp, SortMode mode = SortMode::SEQUENTIAL_MERGE) {
        VariableIterator begin, end;
        GetActiveVariablesRange(begin, end);
        _sorter->Sort(begin, end, comp, mode);
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