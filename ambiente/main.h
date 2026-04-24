#ifndef __MAIN_H
#define __MAIN_H

#include <stdlib.h>
#include <iostream>

#pragma warning(disable:4996)

#include "Constraint.h"
#include "LagrangeanManager.h"


int main2(int argc, char* argv[]);
void cortes(LagrangeanManager* prob, FILE* saida);
float Psi(bool perturbacao);
float Custo(int var);

template <class T> struct  SomaMultiplicadores  {
public:
    SomaMultiplicadores() { resultado = 2; }
    void operator() (T &rest) 
    {  
            resultado += rest->getLagrangean(); 
            cout << resultado << " "; 
    }
    float resultado;
};

template<class T> struct  ImprimeCoeficiente  {
public:
    ImprimeCoeficiente() {};
    void operator() (T &var) { cout << var->retCusto() << ' ' << endl; }
};

template<class T> struct  Imprime  {
public:
    FILE *_fp;
    Imprime() { _fp = stdin; }
    Imprime(FILE *fp) { _fp = fp; };
    void operator() (T &rest) { ((Constraint *)rest)->Print(_fp); }
};



#endif