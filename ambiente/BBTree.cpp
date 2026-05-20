#include "BBTree.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

// ============================================================
// Constructors / Destructor
// ============================================================

BBTree::BBTree() :
    _priorityOpenNodes(NodePriorityComparator(this))
{
    _nodes.reserve(static_cast<int>(std::pow(2, 5)));
    createEmptyNode(-1);
}

BBTree::BBTree(LagrangianManager* manager, Solver* solver, Configuration* config)
    : _config(config),
      _priorityOpenNodes(NodePriorityComparator(this))
{
    SetBranchStrategy();
    _nodes.reserve(_config->MAX_NODES);
    createEmptyNode(-1);
    _nodes[0]._manager = manager;
    _nodes[0]._solver = solver;
	_priorityOpenNodes.push(0);
}

void BBTree::SetBranchStrategy() {
    std::string upperStrategy = _config->getValue("BRANCHSTRATEGY");
    for (char& c : upperStrategy) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    _branchStrategy = SearchAlgorithm::NONE; // default
    if (upperStrategy == "BFS")        _branchStrategy = SearchAlgorithm::BFS;
    if (upperStrategy == "DFS")        _branchStrategy = SearchAlgorithm::DFS;
    if (upperStrategy == "BEST_BOUND") _branchStrategy = SearchAlgorithm::BEST_BOUND;
    if (upperStrategy == "CUSTOM")     _branchStrategy = SearchAlgorithm::CUSTOM;
    if (upperStrategy == "NONE")       _branchStrategy = SearchAlgorithm::NONE;
}

BBTree::~BBTree()
{
    for (size_t i = 1; i < _nodes.size(); i++) {
        delete _nodes[i]._manager;
		_nodes[i]._manager = nullptr;

        delete _nodes[i]._solver;
        _nodes[i]._solver = nullptr;
    }

    _nodes.clear();
    std::priority_queue<int, std::vector<int>, NodePriorityComparator> empty(NodePriorityComparator(this));
	_priorityOpenNodes.swap(empty);

}

// ============================================================
// Main execution
// ============================================================

void BBTree::GO()
{
    while (true) {

        if ( ! MoveToNextOpenNode() )
			break;

        ExecuteNode(_currentNode);
		
        ProcessNodeCompletion(_currentNode);

        if ( CheckGlobalOptimality() ) {
            break;
		}

        if ( ShouldBranchNode(_currentNode) ) {
			BranchNode(_currentNode);
        }
    }

	PrintBranchAndBoundFinalStats();
}

// ============================================================
// Node execution
// ============================================================

void BBTree::ExecuteNode(int node)
{
    Variable* v = nullptr;
	_nodes[node]._state = NodeState::EXECUTING;

    // ========================================================
    // Create node problem
    // ========================================================

    if (node != 0) {

        int father = getFather(node);
        v = _nodes[father]._branchVariable;
        _nodes[node]._manager = _nodes[father]._manager->CopyAndClean(NULL);
        _nodes[node]._solver = _nodes[father]._solver->getNew();
        _nodes[node]._manager->SetSolver(_nodes[node]._solver);
        _nodes[node]._manager->SetVariableForBranch(v, _nodes[node]._value);
		ReleaseNodeRuntime(father);
    }

    PrintNodeStart(node);

    // ========================================================
    // Solve node
    // ========================================================

    _nodes[node]._manager->Solve(_nodes[node]._initialCost, getUpperBound());

    // ========================================================
    // Store node results
    // ========================================================

	_nodes[node]._state = NodeState::EXECUTED;
    _nodesExecuted++;
    _nodes[node]._lowerBound = _nodes[node]._manager->getLowerBound();
    _nodes[node]._upperBound = _nodes[node]._manager->getUpperBound();
    _nodes[node]._optimalFound = _nodes[node]._manager->OptimalFound();
    _nodes[node]._originalBound = _nodes[node]._manager->getPrimalBound();
    _nodes[node]._totalRunTime = _nodes[node]._manager->TotalRunTime();

    PrintNodeEnd(node);

    // ========================================================
    // Releasing parent node runtime resources
    // ========================================================
    
    ReleaseNodeRuntime(node);  // in case optimal found.
	ReleaseNodeRuntime(getFather(node)); // in case two sons were executed
}

// ============================================================
// Branching control
// ============================================================

bool BBTree::MoveToNextOpenNode() 
{
    if (_branchStrategy != SearchAlgorithm::NONE) {
       
        while (!_priorityOpenNodes.empty()) {
            int node = _priorityOpenNodes.top();
            _priorityOpenNodes.pop();
            if (_nodes[node]._state != NodeState::OPEN) {
                continue;
            }
            _currentNode = node;
            return true;
        }
        return false;
	}
	return false;
}

void BBTree::ScheduleChildren(int node) {
    int left = _nodes[node]._leftSon;
    int right = _nodes[node]._rightSon;

	// NONE means no B&B at all, so we do not schedule children.
    if (_branchStrategy != SearchAlgorithm::NONE) {
        if (left >= 0)  _priorityOpenNodes.push(left);
        if (right >= 0) _priorityOpenNodes.push(right);
	}
}

void BBTree::BranchNode(int node)
{
    assert(!_nodes[node].HasChild());

    Variable* v = ChooseBranchVariable(node);
    _nodes[node]._branchVariable = v;

    int left = createEmptyNode(node);
    _nodes[node]._leftSon = left;
    _nodes[left]._value = 1;
    _nodes[left]._initialCost = v->_cost + _nodes[node]._initialCost;

    int right = createEmptyNode(node);
    _nodes[node]._rightSon = right;
    _nodes[right]._value = 0;
    _nodes[right]._initialCost = _nodes[node]._initialCost;

	ScheduleChildren(node);
}

bool BBTree::ShouldBranchNode(int node)
{
    if (_nodes[node]._optimalFound) 
        return false;

    if (_nodes[node]._depth >= _config->MAX_DEPTH)
        return false;

    if (_nodesCount + 2 > _config->MAX_NODES)
        return false;

    return true;
}

Variable* BBTree::ChooseBranchVariable(int node)
{
    return _nodes[node]._solver->ChooseBranchVariable();
}

// ============================================================
// Stop criteria and bound propagation
// ============================================================

void BBTree::ProcessNodeCompletion(int node) {

    if (_nodes[node]._optimalFound)
        pruneSubTree(node);

	PropagateBoundsToRoot(node);

    if ( ! CheckGlobalOptimality() ) {
        reBoundSubtree(0); 
    }

}

void BBTree::PropagateBoundsToRoot(int nodeIndex)
{
    BBTreeNode* node = &(_nodes[nodeIndex]);
    float UB = node->_upperBound;

    BBTreeNode* father = getFather(node);

    while (father != NULL) {

        UpdateAncestorIncumbent(node, father, UB);

        if ((UB - father->_lowerBound) < _config->STOP_GAP) {
            father->_optimalFound = true;
            pruneSubTree(father);
        }

        if (_nodes[father->_leftSon]._optimalFound && _nodes[father->_rightSon]._optimalFound) {
            father->_optimalFound = true;
        }

        node = father;
        father = getFather(father);
    }
}
void BBTree::UpdateAncestorIncumbent(BBTreeNode* node, BBTreeNode* father, float UB) {

    if (UB < father->_upperBound) {
        father->_upperBound = UB;
        father->_manager->StoreIncumbentfromBranchAndBound(node->_manager->_incumbentSolution);
        if (node->_value == 1)
            father->_manager->AddToIncumbentfromBranchAndBound(father->_branchVariable);
    }

}

bool BBTree::CheckGlobalOptimality() {
    return ( getUpperBound() - getLowerBound() ) < _config->STOP_GAP;
}

void BBTree::UpdateNodePriority(int node) {
    if (_branchStrategy == SearchAlgorithm::BFS) {
        _nodes[node]._priority = static_cast<float>(_nodes[node]._depth + _nodesCount / 1E6f);
        return;
    }
    if ( _branchStrategy == SearchAlgorithm::DFS) {
        _nodes[node]._priority = static_cast<float>(-(_nodes[node]._depth + 1/(_nodesCount+1))) ;
        return;
	}
    if ( _branchStrategy == SearchAlgorithm::BEST_BOUND) {
        int father = getFather(node);
        float priority = 0.0f;
        if (father >= 0) {
            priority = _nodes[father]._lowerBound;
        }
        _nodes[node]._priority = priority + 0.001f * static_cast<float>(_nodes[node]._depth);
        return;
	}
    if (_branchStrategy == SearchAlgorithm::CUSTOM) {
        int father = getFather(node);
        if (father >= 0) {
            Variable* v = _nodes[father]._branchVariable;
            short int value = _nodes[node]._value;
            _nodes[node]._priority = _nodes[father]._manager->ComputeChildPriority(v, value);
            return;
        }
    }
    _nodes[node]._priority = 0.0f;
}

// ============================================================
// Tree navigation
// ============================================================

BBTreeNode* BBTree::getFather(BBTreeNode* node)
{
    int n = node->_father;
    if (n < 0)
        return NULL;
    return &_nodes[n];
}

short int BBTree::getFather(int node)
{
    return _nodes[node]._father;
}

// ============================================================
// Tree maintenance
// ============================================================

int BBTree::createEmptyNode(int father)
{
    BBTreeNode n(_config);
    n._father = father;
    if (father >= 0) 
        n._depth = _nodes[father]._depth + 1;
    n._index = _nodesCount;
    _nodes.push_back(n);
    _nodesCount++;
    UpdateNodePriority(n._index);
	return _nodesCount - 1;
}

void BBTree::pruneSubTree(BBTreeNode* node)
{
    pruneSubTree(node->_index);
}

void BBTree::pruneSubTree(int node)
{
    if (node > 0) {
        if (_nodes[node]._state == NodeState::PRUNED)
            return;
        if (_nodes[node]._state != NodeState::EXECUTED)
		    _nodes[node]._state = NodeState::PRUNED;    
        ReleaseNodeRuntime(node);
        pruneSubTree(_nodes[node]._leftSon);
        pruneSubTree(_nodes[node]._rightSon);
    }
}

void BBTree::reBoundSubtree(int node)
{
    if (node < 0)
        return;
	
    float UB = getUpperBound();
    if ((UB - _nodes[node]._lowerBound) < _config->STOP_GAP) {
        _nodes[node]._optimalFound = true;
        pruneSubTree(node);
    }
    else {
        reBoundSubtree(_nodes[node]._leftSon);
        reBoundSubtree(_nodes[node]._rightSon);
    }
}

void BBTree::ReleaseNodeRuntime(int node)
{
    if (node >= 0) {
        if (_nodes[node]._manager == NULL)
            return;
        if (_nodes[node]._optimalFound) {
			_nodes[node]._manager->ReleaseRuntimeResources();
            return;
		}
        if ( (_nodes[node]._leftSon > 0) && (_nodes[node]._rightSon > 0) ) {
            if (_nodes[_nodes[node]._leftSon]._state != NodeState::OPEN
                && _nodes[_nodes[node]._rightSon]._state != NodeState::OPEN) {
                _nodes[node]._manager->ReleaseRuntimeResources();
            }
        }
    }
}

void BBTree::ReleaseNodeRuntime(BBTreeNode* node)
{
    if (node != NULL)
        ReleaseNodeRuntime(node->_index);
}

// ============================================================
// Bound evaluation
// ============================================================

float BBTree::evaluateLowerBound(int node)
{
    if (node < 0)
        return _config->MINUS00;

    float lbl = evaluateLowerBound(_nodes[node]._leftSon);
    float lbr = evaluateLowerBound(_nodes[node]._rightSon);
    float lb = (lbl < lbr ? lbl : lbr);

    return (lb > _nodes[node]._lowerBound ? lb : _nodes[node]._lowerBound);
}

float BBTree::evaluateUpperBound(int node)
{
    if (node < 0)
        return _config->PLUS00;

    float ubl = evaluateUpperBound(_nodes[node]._leftSon);
    float ubr = evaluateUpperBound(_nodes[node]._rightSon);
    float ub = (ubl > ubr ? ubl : ubr);

    return (ub < _nodes[node]._upperBound ? ub : _nodes[node]._upperBound);
}

// ============================================================
// Output
// ============================================================

void BBTree::PrintNodeStart(int node) {
	
	if (node != 0) {
        Variable* v = _nodes[getFather(node)]._branchVariable;
        std::cout << std::endl << std::endl;
        std::cout << "---------------------------------------------------------------" << std::endl;
        std::cout << "   Starting new node: " << node << std::endl;
        std::cout << "   Father node:       " << _nodes[node]._father << std::endl;
        std::cout << "   Branch Variable:   " << v->_name << " Cost: " << v->_cost << std::endl;
        std::cout << "   Branch Value :     " << _nodes[node]._value << std::endl;
        std::cout << "   Initial Cost:      " << _nodes[node]._initialCost << std::endl;
        std::cout << "---------------------------------------------------------------" << std::endl;
    }
    else {
        std::cout << std::endl;
        std::cout << "---------------------------------------------------------------" << std::endl;
        std::cout << "   Starting root node" << std::endl;
        std::cout << "---------------------------------------------------------------" << std::endl;
    }

}

void BBTree::PrintNodeEnd(int node) {
    
    std::cout << "---------------------------------------------------------------" << std::endl;
    std::cout << "   End node execution: " << node << std::endl;

    if (node != 0) {
        Variable* v = _nodes[getFather(node)]._branchVariable;
        std::cout << "   Father node: " << _nodes[node]._father << std::endl;
        std::cout << "   Branch Variable: " << v->_name << std::endl
                  << "   Cost: " << v->_cost << std::endl
                  << "   Branch Value : " << _nodes[node]._value << std::endl;
    }
    std::cout << "   Optimality Proved?  " << _nodes[node]._optimalFound << std::endl;
    std::cout << "   Upper Bound: " << _nodes[node]._upperBound
              << " --- Lower Bound: " << _nodes[node]._lowerBound << std::endl;
    std::cout << "---------------------------------------------------------------" << std::endl;
}

void BBTree::PrintBranchAndBoundFinalStats() {

    std::cout << Print();
    std::cout << std::endl << std::endl;
    std::cout << "Number of Nodes: " << _nodesExecuted << std::endl;

    if (_nodes[0]._optimalFound) {
        std::cout << "Branch End - Optimality proved : " << _nodes[0]._upperBound << std::endl;
    }
    else {
        std::cout << "Branch End - Optimality not proved - Best Bound: " << _nodes[0]._upperBound << std::endl;
        std::cout << "Lower Bound: " << evaluateLowerBound(0) << std::endl;
    }

}

void BBTree::Print(std::string filename)
{
    std::ofstream file;
    file.open(filename);
    file << Print();
    file.close();
}

std::string BBTree::Print()
{
    int number = 1;
    std::string str = "";
    SetFantasyNumber(0, number);
    PrintBFS(0, str);
    return str;
}

void BBTree::PrintBFS(int node, std::string& output)
{
    if (node < 0)
        return;

    if ( _nodes[node]._state != NodeState::EXECUTED)
        return;

    std::stringstream work;

    work << output;
    work << "Fantasy Number: " << _nodes[node]._fantasyNumber << std::endl;
    work << "Index:"           << _nodes[node]._index         << std::endl;

    if (_nodes[node]._branchVariable != NULL) {
        work << "Branch Variable: " << _nodes[node]._branchVariable->_name << std::endl;
		work << "Value:" 		    << _nodes[node]._branchVariable->_cost << std::endl;
    }
    else {
        work << "Branch Variable : NULL"  << std::endl;
        work << "Value: 0"                << std::endl;
    }

    work << "UB: "  << _nodes[node]._originalBound  << std::endl;
    work << "LB: "  << _nodes[node]._lowerBound     << std::endl;

    work << "Optimal found: " << _nodes[node]._optimalFound                    << std::endl;
    work << "Executed: "      << (_nodes[node]._state == NodeState::EXECUTED)  << std::endl;
    work << "Pruned: "        << (_nodes[node]._state == NodeState::PRUNED)    << std::endl;
    work << "Run Time: "      << _nodes[node]._totalRunTime                    << std::endl;
    work << "------------------------------------------------------" << std::endl;

    output = work.str();

    PrintBFS(_nodes[node]._leftSon, output);
    PrintBFS(_nodes[node]._rightSon, output);
}

void BBTree::SetFantasyNumber(int node, int& number)
{
    if (node < 0) {
        number--;
        return;
    }
    if (_nodes[node]._state != NodeState::EXECUTED) {
        number--;
        return;
    }

    int left = _nodes[node]._leftSon;
    int right = _nodes[node]._rightSon;
    SetFantasyNumber(left, number);
    number++;
    _nodes[node]._fantasyNumber = number;
    number++;
    SetFantasyNumber(right, number);
}
