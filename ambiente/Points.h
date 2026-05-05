#ifndef _PONTOS_H
#define _PONTOS_H

#include <map>
#include <set>
#include <algorithm>
#include <vector>
#include <iostream>

using namespace std;

typedef vector<int>::iterator GridIterator;
typedef map < int, int>::iterator PointsIterator;



struct eqInt {
  bool operator()(int a, int b) const
    {
      return ( a == b );
    }

};

#include <stdlib.h>
struct eqstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};


class Points {
  
protected:

    map < int, int > _px;
    map < int, int > _py;

    vector <int> _gridX;
    vector <int> _gridXespelho;
    vector <int> _gridY;

 public:

    Points();
    ~Points();
  
    void Insere(int x, int y);

    int retornaX (int y);
    int retornaY (int x);

    int retornaXDelta (int x, int delta);
    int retornaYDelta (int y, int delta);
    GridIterator retornaIterY(int y);
  
    int retornaIndiceX (int x);
    int retornaIndiceY (int y);

    void retornaPontoX(int &x, int &y, int indice);

    void LimiteGridX(GridIterator &comeco, GridIterator &fim);
    void LimiteGridY(GridIterator &comeco, GridIterator &fim);
  
    void LimitePontosX(PointsIterator &comeco, PointsIterator &fim);
    void LimitePontosY(PointsIterator &comeco, PointsIterator &fim);  

    void FinalizaGrid(int inix, int fimx, int iniy, int fimy);

    string Print();

};

#endif
