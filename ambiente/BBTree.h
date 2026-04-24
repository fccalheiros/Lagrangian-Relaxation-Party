#ifndef ARVOREBB_H
#define ARVOREBB_H

#define _CRT_SECURE_NO_WARNINGS

#include <vector> 
#include <iostream>
#include <string>

#include "Variable.h"
#include "LagrangeanManager.h"
#include "algoritmo.h"
#include "Configuration.h"

enum class SearchAlgorithm { DFS, BFS };

class BBTreeNode {
  
    friend class BBTree;

 public: 
  
     BBTreeNode();
     BBTreeNode(Configuration *config);
     BBTreeNode(const BBTreeNode&);
    ~BBTreeNode();

    bool hasChild();

 protected:
  
    short int _father;
    short int _leftSon;
    short int _rightSon;
    short int _index;
    short int _fantasyNumber;

    float _upperBound;
    float _lowerBound;
    float _initialCost;
    float _originalBound;

    bool _optimalFound; 
    bool _executed;
    bool _pruned;

    float _totalRunTime;

    LagrangianManager* _manager;
    Algoritmo* _algo;

    Variable* _branchVariable;
    short int _value;

};


class BBTree {

public:

    BBTree();
    BBTree(LagrangianManager* manager, Algoritmo* algo, Configuration *config);
    BBTree(LagrangianManager* manager, Algoritmo* algo, SearchAlgorithm sa, Configuration *config);
    ~BBTree();

    void populateTreeDFS(int father, int depth);
    inline void populateTreeBFS(int father, int depth);

    void ExecuteNextNode();
 
    inline void setCurrentNodeBranchVariable(Variable* v);
    inline void setLeftSonValue(short int  value);
    inline void setRightSonValue(short int  value);

    inline float getUpperBound() { return _nodes[0]._upperBound; }
    inline float getLowerBound() { return _nodes[0]._lowerBound; }

    float evaluateLowerBound(int node);
    float evaluateUpperBound(int node);


    void GO();

    string Print();
    void Print(string filename);
    
 protected:

    short int _current;
    short int _nodesCount;
    short int _nodesExecuted;


    vector <BBTreeNode> _nodes;
    Configuration *_config;

protected:

    bool StopTest();
    Variable* ChooseBranchVariable();

    inline void pruneSubTree(BBTreeNode* node);
    void pruneSubTree(int node);
    void reBoundSubtree(int node);

    inline short int getFather(int node);
    inline BBTreeNode* getFather(BBTreeNode* node);

    inline void createEmptyNode(int father);
    inline void cleanUpNode(int node);
    inline void cleanUpNode(BBTreeNode* node);

    void PrintBFS(int node, string & output);
    void SetFantasyNumber(int node, int& number);
};

#endif
