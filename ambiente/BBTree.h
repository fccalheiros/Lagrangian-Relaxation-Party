// ============================================================
// BBTree.h
// ============================================================

#ifndef BBTREE_H
#define BBTREE_H

// ============================================================
// Includes
// ============================================================

#include <vector>
#include <string>

#include "BBTreeNode.h"
#include "Variable.h"
#include "Solver.h"
#include "Configuration.h"
#include "LagrangianManager.h"

// ============================================================
// Search strategy
// ============================================================

enum class SearchAlgorithm { DFS, BFS, NONE };

// ============================================================
// BBTree
// ============================================================
// Branch-and-Bound tree controller.
//
// Responsibilities:
// - Create and manage tree topology
// - Execute nodes
// - Propagate bounds
// - Prune subtrees
// - Coordinate branching decisions
// - Maintain global tree state
// ============================================================

class BBTree {

public:

    // ========================================================
    // Constructors / Destructor
    // ========================================================

    BBTree();

    BBTree(LagrangianManager* manager, Solver* solver, Configuration* config);

    BBTree(LagrangianManager* manager, Solver* solver, SearchAlgorithm sa, Configuration* config);

    ~BBTree();

    // ========================================================
    // Tree construction
    // ========================================================

    void populateTreeDFS(int father, int depth);

    void populateTreeBFS(int father, int depth);

    // ========================================================
    // Execution
    // ========================================================

    void GO();

	void ExecuteNode(int node);

    // ========================================================
    // Branching control
    // ========================================================

	bool MoveToNextOpenNode();

    bool ShouldBranchNode(int node);

    void BranchNode(int node);

    // ========================================================
    // Bounds
    // ========================================================

    inline float getUpperBound() {
        return _nodes[0]._upperBound;
    }

    inline float getLowerBound() {
        return _nodes[0]._lowerBound;
    }

    float evaluateLowerBound(int node);

    float evaluateUpperBound(int node);

    // ========================================================
    // Output
    // ========================================================

    std::string Print();

    void Print(std::string filename);

    void PrintNodeStart(int node);

    void PrintNodeEnd(int node);

	void PrintBranchAndBoundFinalStats();

protected:

    // ========================================================
    // Tree execution control
    // ========================================================

    short int _currentNode = 0;

    short int _nodesCount = 0;

    short int _nodesExecuted = 0;

    // ========================================================
    // Tree data
    // ========================================================

    std::vector<BBTreeNode> _nodes;

    Configuration* _config = nullptr;

protected:

    // ========================================================
    // Branch-and-bound logic
    // ========================================================

    bool StopTest();

    Variable* ChooseBranchVariable();

    // ========================================================
    // Pruning and bound propagation
    // ========================================================

    inline void pruneSubTree(BBTreeNode* node);

    void pruneSubTree(int node);

    void reBoundSubtree(int node);

    // ========================================================
    // Tree navigation
    // ========================================================

    inline short int getFather(int node);

    inline BBTreeNode* getFather(BBTreeNode* node);

    // ========================================================
    // Node lifecycle
    // ========================================================

    inline void createEmptyNode(int father);

    inline void cleanUpNode(int node);

    inline void cleanUpNode(BBTreeNode* node);

    // ========================================================
    // Output helpers
    // ========================================================

    void PrintBFS(int node, std::string& output);

    void SetFantasyNumber(int node, int& number);
};

#endif