#ifndef __MAIN_H
#define __MAIN_H

#include <stdlib.h>
#include <iostream>

#pragma warning(disable:4996)

#include "Constraint.h"
#include "LagrangianManager.h"
#include "BBTree.h"

int originalMain(int argc, char* argv[]);
void Cuts(LagrangianManager* prob, FILE* output);
static void StartStats();
static void EndStats();
static bool CheckUsage(int argc, char* argv[]);
SearchAlgorithm ParseBranchStrategy(const string& strategy);
Configuration* LoadConfig(const char* configFile);

template<class T> struct  Imprime  {
public:
    FILE *_fp;
    Imprime() { _fp = stdin; }
    Imprime(FILE *fp) { _fp = fp; };
    void operator() (T &rest) { rest->Print(_fp); }
};



#endif