#include "BBTree.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

// ============================================================
// Constructors / Destructor
// ============================================================

BBTree::BBTree()
{
    _nodes.reserve(static_cast<int>(std::pow(2, 5)));
    createEmptyNode(-1);
}

BBTree::BBTree(LagrangianManager* manager, Solver* solver, Configuration* config)
    : _config(config)
{
    _nodes.reserve(static_cast<int>(std::pow(2, _config->MAX_DEPTH + 1)));
    createEmptyNode(-1);
    _nodes[_current]._manager = manager;
    _nodes[_current]._solver = solver;
}
BBTree::BBTree(LagrangianManager* manager, Solver* solver, SearchAlgorithm sa, Configuration* config)
    : BBTree(manager, solver, config)
{
    if (sa == SearchAlgorithm::BFS)
        populateTreeBFS(0, _config->MAX_DEPTH);
    else if (sa == SearchAlgorithm::DFS)
        populateTreeDFS(0, _config->MAX_DEPTH);
}

BBTree::~BBTree()
{
    _nodes.clear();
}

// ============================================================
// Tree construction
// ============================================================

void BBTree::populateTreeDFS(int node, int depth)
{
    if (depth == 0)
        return;

    createEmptyNode(node);
    _nodes[node]._leftSon = _nodesCount - 1;
    populateTreeDFS(_nodes[node]._leftSon, depth - 1);

    createEmptyNode(node);
    _nodes[node]._rightSon = _nodesCount - 1;
    populateTreeDFS(_nodes[node]._rightSon, depth - 1);
}

void BBTree::populateTreeBFS(int node, int depth)
{
    int pointer = 0;
    int lastNode;

    for (int i = 0; i < depth; i++) {
        lastNode = _nodesCount;
        while (pointer < lastNode) {
			
            createEmptyNode(pointer);
            _nodes[pointer]._leftSon = _nodesCount - 1;

            createEmptyNode(pointer);
            _nodes[pointer]._rightSon = _nodesCount - 1;

            pointer++;
        }
    }
}

// ============================================================
// Main execution
// ============================================================

void BBTree::GO()
{
    Variable* v;

    while (!StopTest()) {

        ExecuteNextNode();

        if (_current == _nodesCount)
            break;

        if (!_nodes[_current]._optimalFound && _nodes[_current].HasChild()) {
            v = ChooseBranchVariable();
            setCurrentNodeBranchVariable(v);
            setLeftSonValue(1);
            setRightSonValue(0);
        }
    }

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

// ============================================================
// Node execution
// ============================================================

void BBTree::ExecuteNextNode()
{
    Variable* v = nullptr;

    while (_nodes[_current]._executed || _nodes[_current]._pruned) {
        _current++;
        if (_current == _nodesCount)
            return;
    }

    // ========================================================
    // Create node problem
    // ========================================================

    if (_current != 0) {

        int father = getFather(_current);
		
        v = _nodes[father]._branchVariable;
        _nodes[_current]._manager = _nodes[father]._manager->CopyAndClean(NULL);
        _nodes[_current]._solver    = _nodes[father]._solver->getNew();
		_nodes[_current]._manager->SetSolver(_nodes[_current]._solver);
		_nodes[_current]._manager->SetVariableForBranch(v, _nodes[_current]._value);

        std::cout << std::endl << std::endl;
        std::cout << "---------------------------------------------------------------" << std::endl;
        std::cout << "   Starting new node: " << _current << std::endl;
        std::cout << "   Father node:       " << _nodes[_current]._father << std::endl;
        std::cout << "   Branch Variable:   " << v->_nome << " Cost: " << v->_valorFO
                  << "   Branch Value :     " << _nodes[_current]._value
				  << "   Initial Cost:      " << _nodes[_current]._initialCost << std::endl;
        std::cout << "---------------------------------------------------------------" << std::endl;
    }
    else {

        std::cout << std::endl;
        std::cout << "---------------------------------------------------------------" << std::endl;
        std::cout << "   Starting root node" << std::endl;
        std::cout << "---------------------------------------------------------------" << std::endl;
    }

    // ========================================================
    // Solve node
    // ========================================================

    _nodes[_current]._manager->Solve(_nodes[_current]._initialCost, getUpperBound());

    // ========================================================
    // Store node results
    // ========================================================

    _nodes[_current]._executed = true;
    _nodesExecuted++;
    _nodes[_current]._lowerBound    = _nodes[_current]._manager->getLowerBound();
    _nodes[_current]._upperBound    = _nodes[_current]._manager->getUpperBound();
    _nodes[_current]._optimalFound  = _nodes[_current]._manager->OptimalFound();
    _nodes[_current]._originalBound = _nodes[_current]._manager->getPrimalBound();
    _nodes[_current]._totalRunTime  = _nodes[_current]._manager->TotalRunTime();

    // ========================================================
    // Cleanup parent node
    // ========================================================

    int node = getFather(_current);

    if (node >= 0) {
        if (_nodes[_nodes[node]._leftSon]._executed && _nodes[_nodes[node]._rightSon]._executed) {
            cleanUpNode(node);
        }
    }

    // ========================================================
    // Final node output
    // ========================================================

    std::cout << "---------------------------------------------------------------" << std::endl;
    std::cout << "   End node execution: " << _current << std::endl;

    if (_current != 0) {
        std::cout << "   Father node: "      << _nodes[_current]._father  << std::endl;
        std::cout << "   Branch Variable: "  << v->_nome 				  << std::endl
				  << "   Cost: "             << v->_valorFO 			  << std::endl
                  << "   Branch Value : "    << _nodes[_current]._value   << std::endl;
    }
    std::cout << "   Optimality Proved?  "   << _nodes[_current]._optimalFound  << std::endl;
    std::cout << "   Upper Bound: "          << _nodes[_current]._upperBound
              << " --- Lower Bound: "        << _nodes[_current]._lowerBound    << std::endl;
    std::cout << "---------------------------------------------------------------" << std::endl;
}

// ============================================================
// Branching helpers
// ============================================================

Variable* BBTree::ChooseBranchVariable()
{
    return _nodes[_current]._solver->ChooseBranchVariable();
}

void BBTree::setCurrentNodeBranchVariable(Variable* v)
{
    _nodes[_current]._branchVariable = v;
}

void BBTree::setLeftSonValue(short int value)
{
    if (_nodes[_current]._leftSon > 0) {
        _nodes[_nodes[_current]._leftSon]._value = value;
        _nodes[_nodes[_current]._leftSon]._initialCost =
            _nodes[_current]._branchVariable->_valorFO * value +
            _nodes[_current]._initialCost;
    }
}

void BBTree::setRightSonValue(short int value)
{
    if (_nodes[_current]._rightSon > 0) {
        _nodes[_nodes[_current]._rightSon]._value = value;
        _nodes[_nodes[_current]._rightSon]._initialCost =
            _nodes[_current]._branchVariable->_valorFO * value +
            _nodes[_current]._initialCost;
    }
}

// ============================================================
// Stop criteria and bound propagation
// ============================================================

bool BBTree::StopTest()
{
    float UB;
    float LB;

    BBTreeNode* node = &(_nodes[_current]);
    BBTreeNode* lastNode;

    if (node->_optimalFound)
        pruneSubTree(node);

    UB = node->_upperBound;
    LB = node->_lowerBound;

    if (LB > UB)
        LB = UB - 0.0001f;

    lastNode = node;
    node = getFather(node);

    while (node != NULL) {

        if (UB < node->_upperBound) {

            node->_upperBound = UB;
            node->_manager->StoreIncumbentfromBranchAndBound(lastNode->_manager->_incumbentSolution);

            if (lastNode->_value == 1)
                node->_manager->AddToIncumbentfromBranchAndBound(node->_branchVariable);
            if ((UB - node->_lowerBound) < _config->STOP_GAP) {
                node->_optimalFound = true;
                pruneSubTree(node);
            }
        }

        if (_nodes[node->_leftSon]._optimalFound && _nodes[node->_rightSon]._optimalFound) {
            node->_optimalFound = true;
        }
        lastNode = node;
        node = getFather(node);
    }

    if ((_nodes[0]._upperBound - _nodes[0]._lowerBound) < _config->STOP_GAP) {
        return true;
    }
    else {
        reBoundSubtree(0);
    }

    return false;
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

void BBTree::createEmptyNode(int father)
{
    BBTreeNode n(_config);
    n._father = father;
    n._index = _nodesCount;
    _nodes.push_back(n);
    _nodesCount++;
}

void BBTree::pruneSubTree(BBTreeNode* node)
{
    pruneSubTree(node->_index);
}

void BBTree::pruneSubTree(int node)
{
    if (node > 0) {
        if (_nodes[node]._pruned)
            return;
        _nodes[node]._pruned = true;
        cleanUpNode(node);
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

void BBTree::cleanUpNode(int node)
{
    if (node >= 0) {
        if (_nodes[node]._manager == NULL)
            return;
        _nodes[node]._manager->FreeMemory();
    }
}

void BBTree::cleanUpNode(BBTreeNode* node)
{
    if (node != NULL)
        cleanUpNode(node->_index);
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

    if (!_nodes[node]._executed)
        return;

    std::stringstream work;

    work << output;
    work << "Fantasy Number: " << _nodes[node]._fantasyNumber << std::endl;
    work << "Index:"           << _nodes[node]._index         << std::endl;

    if (_nodes[node]._branchVariable != NULL) {
        work << "Branch Variable: " << _nodes[node]._branchVariable->_nome    << std::endl;
		work << "Value:" 		    << _nodes[node]._branchVariable->_valorFO << std::endl;
    }
    else {
        work << "Branch Variable : NULL"  << std::endl;
        work << "Value: 0"                << std::endl;
    }

    work << "UB: "  << _nodes[node]._originalBound  << std::endl;
    work << "LB: "  << _nodes[node]._lowerBound     << std::endl;

    work << "Optimal found: " << _nodes[node]._optimalFound  << std::endl;
    work << "Executed: "      << _nodes[node]._executed      << std::endl;
    work << "Pruned: "        << _nodes[node]._pruned        << std::endl;
    work << "Run Time: "      << _nodes[node]._totalRunTime  << std::endl;
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
    if (!_nodes[node]._executed) {
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