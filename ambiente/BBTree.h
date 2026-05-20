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

enum class SearchAlgorithm { DFS, BFS, BEST_BOUND, CUSTOM, NONE };


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

private:

    struct NodePriorityComparator {

        const BBTree* tree = nullptr;

        NodePriorityComparator() = default;

        NodePriorityComparator(BBTree* t)
            : tree(t) {
        }

        bool operator()(int a, int b) const {
            return tree->_nodes[a]._priority > tree->_nodes[b]._priority;
        }
    };

public:

    // ========================================================
    // Constructors / Destructor
    // ========================================================

    BBTree();

    BBTree(LagrangianManager* manager, Solver* solver, Configuration* config);

    ~BBTree();

    // ========================================================
    // Execution
    // ========================================================

    void GO();

	void ExecuteNode(int node);

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

	void PrintBranchAndBoundFinalStats();

protected:

    // ========================================================
    // Tree execution control
    // ========================================================

    short int _currentNode = 0;

    short int _nodesCount = 0;

    short int _nodesExecuted = 0;

    SearchAlgorithm _branchStrategy = SearchAlgorithm::NONE;

    // ========================================================
    // Tree data
    // ========================================================

    std::vector<BBTreeNode> _nodes;

    std::priority_queue<int,std::vector<int>, NodePriorityComparator > _priorityOpenNodes;

    Configuration* _config = nullptr;

protected:

    // ========================================================
    // Branching control
    // ========================================================

    bool MoveToNextOpenNode();

    bool ShouldBranchNode(int node);

    void BranchNode(int node);
    
    void ScheduleChildren(int node);

    Variable* ChooseBranchVariable(int node);

    bool CheckGlobalOptimality();

    void UpdateNodePriority(int node);

    // ========================================================
    // Node operations
    // ========================================================

    void ProcessNodeCompletion(int node);

    // ========================================================
    // Pruning and bound propagation
    // ========================================================

    void PropagateBoundsToRoot(int nodeIndex);

	void UpdateAncestorIncumbent(BBTreeNode* node, BBTreeNode* father, float UB);

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

    inline int createEmptyNode(int father);

    inline void ReleaseNodeRuntime(int node);

    inline void ReleaseNodeRuntime(BBTreeNode* node);

    // ========================================================
    // Output helpers
    // ========================================================

    void PrintNodeStart(int node);

    void PrintNodeEnd(int node);

    void PrintBFS(int node, std::string& output);

    void SetFantasyNumber(int node, int& number);

private: 

    void SetBranchStrategy();

};

#endif