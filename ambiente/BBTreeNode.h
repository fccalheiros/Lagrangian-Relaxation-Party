#ifndef BBTREENODE_H
#define BBTREENODE_H

#include "Configuration.h"
// ============================================================
// Forward declarations
// ============================================================

class Variable;
class LagrangianManager;
class Solver;

// ============================================================
// BBTreeNode
// ============================================================
// Represents a single node in the Branch-and-Bound tree.
//
// Responsibilities:
// - Store local bounds
// - Store branching decision
// - Store execution/pruning state
// - Hold manager/algorithm instances associated with the node
// - Keep tree topology information
// ============================================================

enum class NodeState {
    OPEN,
    EXECUTING,
    EXECUTED,
    PRUNED
};

class BBTreeNode {

    friend class BBTree;

public:

    // ========================================================
    // Constructors / Destructor
    // ========================================================

    BBTreeNode() = default;
	
	explicit BBTreeNode(Configuration* config)
	{
        _lowerBound = config->MINUS00;
        _upperBound = config->PLUS00;
	}

    BBTreeNode(const BBTreeNode&) = default;

    ~BBTreeNode() = default;

    // ========================================================
    // State helpers
    // ========================================================

    inline bool HasChild() const {
        return (_leftSon != -1 || _rightSon != -1);
    }

    bool ReachedFinalState() const {
        return _state == NodeState::EXECUTED || _state == NodeState::PRUNED;
	}

protected:

    // ========================================================
    // Tree topology
    // ========================================================

    short int _father = 0;

    short int _leftSon = -1;
    short int _rightSon = -1;

    short int _index = 0;
    short int _fantasyNumber = 0;

	short int _depth = 0;

    // ========================================================
    // Bounds
    // ========================================================

    float _upperBound = 0.0f;
    float _lowerBound = 0.0f;

    float _initialCost = 0.0f;
    float _originalBound = 0.0f;

    // ========================================================
    // Execution state
    // ========================================================

    bool _optimalFound = false;

	NodeState _state = NodeState::OPEN;

    // ========================================================
    // Statistics
    // ========================================================

    double _totalRunTime = 0.0;

    // ========================================================
    // Problem instances
    // ========================================================

    LagrangianManager* _manager = nullptr;
    Solver* _solver = nullptr;

    // ========================================================
    // Branching information
    // ========================================================

    Variable* _branchVariable = nullptr;

    short int _value = 1;
};

#endif
