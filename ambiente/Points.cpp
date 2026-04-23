#include "Points.h"
#include <sstream>


Points::Points() 
{
    _gridX.reserve(200);
    _gridY.reserve(200);
}

Points::~Points() { }

void Points::Insere(int x, int y) {
    _px[x] = y;
    _py[y] = x;

    _gridX.push_back(x);
    _gridY.push_back(y);
}

int Points::retornaX (int y) {
 
    map<int, int>::const_iterator it = _py.find(y);

    if ( it != _py.end() ) 
    return ( (*it).second );
    return -1;
}

int Points::retornaY(int x) {

    map<int, int>::const_iterator it = _px.find(x);
    if (it != _px.end())
        return ((*it).second);
    return -1;
}

int Points::retornaIndiceX(int x) {
  size_t i = 0, m;
  size_t f = _gridX.size();

  while (i != f) {
      m = (i+f) /2;
      if ( _gridX[m] < x ) i = m + 1;
      else f = m;
  }
  
  return static_cast<int>(i);
}

int Points::retornaXDelta (int x, int delta) {

  int indice = retornaIndiceX(x);
  return _gridX[static_cast<size_t>(indice+delta)];

}

int Points::retornaIndiceY(int y) {

  GridIter it = lower_bound(_gridY.begin(),_gridY.end(),y);
  
  return (int)( distance(_gridY.begin(),it));
}

void Points::retornaPontoX(int& x, int& y, int indice) {
    x = _gridX[indice];
    y = _gridXespelho[indice];
}

int Points::retornaYDelta (int y, int delta) {

  int indice = retornaIndiceY(y);
  return _gridY[static_cast<size_t>(indice+delta)];

}

GridIter Points::retornaIterY(int y) {
   return ( lower_bound(_gridY.begin(),_gridY.end(),y) );
}

void Points::LimiteGridX(GridIter &comeco, GridIter &fim){
  comeco = _gridX.begin();
  fim    = _gridX.end();
}

void Points::LimiteGridY(GridIter &comeco, GridIter &fim){
  comeco = _gridY.begin();
  fim    = _gridY.end();
}

void Points::LimitePontosX(PointsIterator &comeco, PointsIterator &fim)
{
  comeco = _px.begin();
  fim    = _px.end();
}

void Points::LimitePontosY(PointsIterator &comeco, PointsIterator &fim)
{
  comeco = _py.begin();
  fim    = _py.end();
}

void Points::FinalizaGrid(int inix, int fimx, int iniy, int fimy) {

    _gridX.push_back(inix);
    _gridX.push_back(fimx);
    _gridY.push_back(iniy);
    _gridY.push_back(fimy);

    _gridXespelho.reserve(_gridY.size());
    _gridXespelho.push_back(iniy);

    sort(_gridX.begin(),_gridX.end());
    sort(_gridY.begin(),_gridY.end());

    PointsIterator it = _px.begin();
    for (; it != _px.end();it++) {
        _gridXespelho.push_back( (*it).second );
    }

    _gridXespelho.push_back(fimy);

}

string Points::Print() {

    stringstream work;

    work << "Points " << endl;

    PointsIterator it = _px.begin();
    for (; it != _px.end(); it++) {
        work << (*it).first << " " << (*it).second << endl;
    }
    work << "X Grid" << endl;

    GridIter it2 = _gridX.begin();
    for (; it2 != _gridX.end(); it2++)
        work << *it2 << " ";
    work << endl;

    work<< "Y Grid" << endl;
    it2 = _gridY.begin();
    for (; it2 != _gridY.end(); it2++)
        work << *it2 << " ";
    work << endl;

    return work.str();
}