#include "BBTree.h"
#include "RGPLagrangeanRelaxation.h"


#include <sstream>
#include <fstream>



BBTreeNode::BBTreeNode() : 
    _father(0), 
    _manager(nullptr), 
    _algo(nullptr), 
    _branchVariable(nullptr),
    _value(1), 
    _leftSon(-1), 
    _rightSon(-1), 
    _index(0), 
    _fantasyNumber(0),
    _optimalFound(false), 
    _executed(false), 
    _pruned(false),
    _lowerBound(0), 
    _upperBound(0), 
    _originalBound(0),
    _initialCost(0), 
    _totalRunTime(0) 
{
}

BBTreeNode::BBTreeNode(Configuration* config)
    : BBTreeNode() { 
    _lowerBound = config->MINUS00;
    _upperBound = config->PLUS00;
}

// Construtor de cópia
BBTreeNode::BBTreeNode(const BBTreeNode& node) : 
    _father(node._father),
    _manager(node._manager),
    _algo(node._algo),
    _branchVariable(node._branchVariable),
    _value(node._value),
    _leftSon(node._leftSon),
    _rightSon(node._rightSon),
    _index(node._index),
    _fantasyNumber(node._fantasyNumber),
    _optimalFound(node._optimalFound),
    _executed(node._executed),
    _pruned(node._pruned),
    _lowerBound(node._lowerBound),
    _upperBound(node._upperBound),
    _originalBound(node._originalBound),
    _initialCost(node._initialCost), 
    _totalRunTime(node._totalRunTime) 
{
}


BBTreeNode::~BBTreeNode() {}

bool BBTreeNode::hasChild() {
    return (_leftSon != -1 || _rightSon != -1);
}

/********* BBTree **********/


BBTree::BBTree()
    : _current(0)
    , _nodesCount(0)
    , _nodesExecuted(0)
    , _config(nullptr)
{
    _nodes.reserve(static_cast<int>(std::pow(2, 5)));
    createEmptyNode(-1);
}

BBTree::BBTree(LagrangeanManager* manager, Algoritmo* algo, Configuration* config)
    : _current(0)
    , _nodesCount(0)
    , _nodesExecuted(0)
    , _config(config)
{
    _nodes.reserve(static_cast<int>(std::pow(2, _config->MAX_DEPTH + 1)));
    createEmptyNode(-1);

    _nodes[_current]._manager = manager;
    _nodes[_current]._algo = algo;
}   


BBTree::BBTree(LagrangeanManager* manager, Algoritmo* algo,
    SearchAlgorithm sa, Configuration* config)
    : BBTree(manager, algo, config) 
{
    if (sa == SearchAlgorithm::BFS)
        populateTreeBFS(0, _config->MAX_DEPTH);
    else if (sa == SearchAlgorithm::DFS)
        populateTreeDFS(0, _config->MAX_DEPTH);
}

BBTree::~BBTree() {
    _nodes.erase(_nodes.begin(),_nodes.end());
}

void BBTree::populateTreeDFS(int node, int depth) {

    if (depth == 0) return;

    createEmptyNode(node);
    _nodes[node]._leftSon = _nodesCount - 1;    
    populateTreeDFS(_nodes[node]._leftSon, depth - 1);
    
    createEmptyNode(node);
    _nodes[node]._rightSon = _nodesCount - 1;
    populateTreeDFS(_nodes[node]._rightSon, depth - 1);


}

void BBTree::populateTreeBFS(int node, int depth) {

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

bool BBTree::StopTest() {

    float UB, LB;
    BBTreeNode* node = &(_nodes[_current]);
    BBTreeNode* lastNode;

    if (node->_optimalFound)
        pruneSubTree(node);
        
    UB = node->_upperBound;
    LB = node->_lowerBound;
    if (LB > UB) 
        LB = UB - (float) 0.0001;

    lastNode = node;
    node = getFather(node);

    while (node != NULL) {
 
        if (UB < node->_upperBound) {
            node->_upperBound = UB;
            if ( (UB - node->_lowerBound) < _config->STOP_GAP) {
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
    if ((_nodes[0]._upperBound - _nodes[0]._lowerBound) < _config->STOP_GAP)
        return true;
    else
        reBoundSubtree(0);

    return false;
}

Variable* BBTree::ChooseBranchVariable() {

    return _nodes[_current]._algo->ChooseBranchVariable();

}

void BBTree::ExecuteNextNode() {
    Variable* v = NULL;

    while (_nodes[_current]._executed || _nodes[_current]._pruned) {
        _current++;
        if (_current == _nodesCount)
            return;
    }

    if (_current != 0) {
        int father = getFather(_current);
        v = _nodes[father]._branchVariable;

        _nodes[_current]._manager = _nodes[father]._manager->CopyAndClean(NULL);
        _nodes[_current]._algo = _nodes[father]._algo->getNew();
        _nodes[_current]._manager->SetAlgorithm(_nodes[_current]._algo);
        _nodes[_current]._manager->SetVariableForBranch(v, _nodes[_current]._value);

        cout << endl << endl << "------------------------------------------------------------------------------- " << endl;
        cout << "   Starting new node: " << _current << endl;
        cout << "   Father node: " << _nodes[_current]._father << endl;
        cout << "   Branch Variable: " << v->_nome << " Cost: " << v->_valorFO << " Branch Value : " << _nodes[_current]._value  << " Initial Cost: " << _nodes[_current]._initialCost << endl;
        cout << "------------------------------------------------------------------------------- " << endl;
    }
    else {
        cout << endl << "------------------------------------------------------------------------------- " << endl;
        cout << "   Starting root node" <<  endl;
        cout << "------------------------------------------------------------------------------- " << endl;
    }

    _nodes[_current]._manager->Solve(_nodes[_current]._initialCost, getUpperBound());
    _nodes[_current]._executed = true;
    _nodesExecuted++;

    _nodes[_current]._lowerBound = _nodes[_current]._manager->getLowerBound();
    _nodes[_current]._upperBound = _nodes[_current]._manager->getUpperBound();
    _nodes[_current]._optimalFound = _nodes[_current]._manager->OptimalFound();
    _nodes[_current]._originalBound = _nodes[_current]._manager->getBound();
    _nodes[_current]._totalRunTime = _nodes[_current]._manager->TotalRunTime();

    int node = getFather(_current);
    if (node >= 0) {
        if (_nodes[_nodes[node]._leftSon]._executed && _nodes[_nodes[node]._rightSon]._executed)
            cleanUpNode(node);
    }

    cout << "------------------------------------------------------------------------------- " << endl;
    cout << "   End node execution: " << _current << endl;
    if ( _current != 0) {
        cout << "   Father node: " << _nodes[_current]._father << endl;
        cout << "   Branch Variable: " << v->_nome << " Cost: " << v->_valorFO << " Branch Value : " << _nodes[_current]._value << endl;
    }
    cout << "   Optimality Proved?  " << _nodes[_current]._optimalFound << endl;
    cout << "   Upper Bound: " << _nodes[_current]._upperBound << " --- Lower Bound: " << _nodes[_current]._lowerBound << endl;
    cout << "------------------------------------------------------------------------------- " << endl;
}


void BBTree::GO() 
{

    Variable* v;

    while (!StopTest()) {
        
        ExecuteNextNode();

        if (_current == _nodesCount) break;

        if (!_nodes[_current]._optimalFound && _nodes[_current].hasChild() ) {
            v = ChooseBranchVariable();
            //cout << endl << "Branch Variable: " << v->_nome << " --- " << v->_valorFO << endl;
            setCurrentNodeBranchVariable(v);
            setLeftSonValue(1);
            setRightSonValue(0);
        }

    }
    cout << Print();

    cout << endl << endl;
    cout << "Number of Nodes: " << _nodesExecuted << endl;
    if (_nodes[0]._optimalFound)
        cout << "Branch End - Optimality proved : " << _nodes[0]._upperBound << endl;
    else {
        cout << "Branch End - Optimality not proved - Best Bound: " << _nodes[0]._upperBound << endl;
        cout << "Lower Bound: " << evaluateLowerBound (0) << endl;
    }
}

void BBTree::setCurrentNodeBranchVariable(Variable* v) {
    _nodes[_current]._branchVariable = v;
}

void BBTree::setLeftSonValue(short int  value)
{
    if (_nodes[_current]._leftSon > 0) {
        _nodes[_nodes[_current]._leftSon]._value = value;
        _nodes[_nodes[_current]._leftSon]._initialCost = _nodes[_current]._branchVariable->_valorFO * value + _nodes[_current]._initialCost;
    }
}

void BBTree::setRightSonValue(short int  value)
{
    if (_nodes[_current]._rightSon > 0) {
        _nodes[_nodes[_current]._rightSon]._value = value;
        _nodes[_nodes[_current]._rightSon]._initialCost = _nodes[_current]._branchVariable->_valorFO * value + _nodes[_current]._initialCost;
    }
}


void BBTree::createEmptyNode(int father) {

    BBTreeNode *n = new BBTreeNode(_config);
    n->_father = father;
    n->_index = _nodesCount;

    _nodes.push_back(*n);
    _nodesCount++;
}

BBTreeNode* BBTree::getFather(BBTreeNode* node) {

    int n = node->_father;
   
    if (n < 0) return NULL;

    return (&_nodes[n]);
}

short int BBTree::getFather(int node) {
    return _nodes[node]._father;
}

void BBTree::pruneSubTree(BBTreeNode* node) {
    pruneSubTree(node->_index);
}

void BBTree::pruneSubTree(int node) {
    if (node > 0) {

        if ( _nodes[node]._pruned ) return;

        _nodes[node]._pruned = true;
        cleanUpNode(node);
        pruneSubTree(_nodes[node]._leftSon);
        pruneSubTree(_nodes[node]._rightSon);

    }
}

void BBTree::reBoundSubtree(int node) {
    
    if (node < 0) return;

    float UB = getUpperBound();
    if ( (UB - _nodes[node]._lowerBound) < _config->STOP_GAP) {
        _nodes[node]._optimalFound = true;
        pruneSubTree(node);
    } 
    else {
        reBoundSubtree(_nodes[node]._leftSon);
        reBoundSubtree(_nodes[node]._rightSon);
    }

}

void BBTree::cleanUpNode(int node){
    if (node >= 0) {
        if (_nodes[node]._manager == NULL) 
            return;
        _nodes[node]._manager->FreeMemory();
        //_nodes[node]._manager = NULL;
    }
}

void BBTree::cleanUpNode(BBTreeNode* node){
    if (node != NULL)
        cleanUpNode(node->_index);
}

float BBTree::evaluateLowerBound(int node) {

    if (node < 0) return _config->MINUS00;
    float lbl = evaluateLowerBound(_nodes[node]._leftSon);
    float lbr = evaluateLowerBound(_nodes[node]._rightSon);
    float lb = (lbl < lbr ? lbl : lbr);

    return (lb > _nodes[node]._lowerBound ? lb : _nodes[node]._lowerBound);
}



float BBTree::evaluateUpperBound(int node) {

    if (node < 0) return _config->PLUS00;
    float ubl = evaluateUpperBound(_nodes[node]._leftSon);
    float ubr = evaluateUpperBound(_nodes[node]._rightSon);
    float ub = (ubl > ubr ? ubl : ubr);

    return (ub < _nodes[node]._upperBound ? ub : _nodes[node]._upperBound);
}


void BBTree::Print(string filename) {
    ofstream file;
    file.open(filename);
    file << Print();
    file.close();
}

string BBTree::Print() {

    int number = 1;
    string str = "";
    SetFantasyNumber(0, number);
    PrintBFS(0, str);
    return str;
}

void BBTree::PrintBFS(int node, string & output) {
    
    if (node < 0) return;
    if (!_nodes[node]._executed) return;
    stringstream work;
    
    work << output;
    work << "Fantasy Number: " << _nodes[node]._fantasyNumber << endl;
    work << "Index:" << _nodes[node]._index << endl;

    if (_nodes[node]._branchVariable != NULL) {
        work << "Branch Variable: " << _nodes[node]._branchVariable->_nome << endl;
        work << "Value:" << _nodes[node]._branchVariable->_valorFO << endl;
    }
    else {
        work << "Branch Variable : NULL" << endl;
        work << "Value: 0"  << endl;
    }

    work << "UB: " << _nodes[node]._originalBound << endl;
    work << "LB: " << _nodes[node]._lowerBound << endl;
    work << "Optimal found: "<< _nodes[node]._optimalFound << endl;
    work << "Executed: " << _nodes[node]._executed << endl;
    work << "Pruned: " << _nodes[node]._pruned << endl;
    work << "Run Time: " << _nodes[node]._totalRunTime << endl;
    work << "------------------------------------------------------" << endl;

    output = work.str();

    PrintBFS(_nodes[node]._leftSon , output);
    PrintBFS(_nodes[node]._rightSon, output);

}

void BBTree::SetFantasyNumber(int node, int &number) {
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
