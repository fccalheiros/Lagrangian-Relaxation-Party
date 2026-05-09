#ifndef GRAFO_H
#define GRAFO_H

#ifndef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif

#include <map>
#include <list>
#include <vector>

#include "restricao_hash.h"
#include "Variable.h"
#include "LagrangianManager.h"

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

	std::map <Variable*, int, lessVariavel <Variable*>> _nosVariaveis;

	std::vector <GraphNode> _infoNos;
	std::vector <Variable *> _nos;
	std::list <int> _dfsPath;

	std::vector < std::vector < int > > _arestas;

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


