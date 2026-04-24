#ifndef GRAFO_H
#define GRAFO_H

#include <map>
#include <list>
#include <vector>

#include "restricao_hash.h"
#include "Variable.h"
#include "LagrangeanManager.h"

class GraphNode {

 public:
	GraphNode();
	GraphNode(const GraphNode &no);
	~GraphNode();

	short int _nivel;
	short int _pai;

};

class Grafo {

 protected:

	map <Variable*, int, lessVariavel <Variable*>> _nosVariaveis;

	vector <GraphNode> _infoNos;
	vector <Variable *> _nos;
	list <int> _dfsPath;

	vector < vector < int > > _arestas;

	short int _proximo;
	short int _numeroNos;
	GraphNode _NoBase;

 public:

	Grafo();
	Grafo(int size);
	~Grafo();

	void InsereNo(Variable *no);
	void InsereAresta(int no1,int no2);
	void InsereAresta(Variable *no1, Variable *no2);
	bool TemAresta(int no1, int no2);

	int Grau(int no);
	void Clique(LagrangianManager *mestre,int Minimo=3);
	void CicloImpar(LagrangianManager *mestre);
	void DFS(int no);

	void Imprime();
	void ImprimeNos();
	void ImprimeDFS();

 private:

	bool _aumentou;

};

#endif
