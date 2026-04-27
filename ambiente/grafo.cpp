#include "grafo.h"
#include "Constraint.h"


typedef map <Variable*, int, lessVariavel <Variable*>>::iterator GraphNodeIterator;

int contacortes = 0;

GraphNode::GraphNode() {
  _nivel = -1;
  _pai   = -1;
} 

GraphNode::GraphNode(const GraphNode &no) {
  _nivel  = no._nivel;
  _pai    = no._pai;
}


GraphNode::~GraphNode()
{ }


Grafo::Grafo(int size) :
    _proximo(0),
    _numeroNos(0),
    _aumentou(false)
{
    _nos.reserve(size);
    _arestas.reserve(size);    
}

Grafo::Grafo() :
    Grafo(40)
{
}

Grafo::~Grafo() {
}

void Grafo::InsereNo(Variable *no) {
    _nos.push_back(no);
    _nosVariaveis[no] = _proximo;

    _infoNos.push_back(_NoBase);

    _proximo++;
    _numeroNos++;
    _aumentou = true;
}

void Grafo::InsereAresta(int no1, int no2) {
  
    if (_aumentou ) {
        _arestas.resize(static_cast<size_t>(_proximo) + 3);
        _aumentou = false;
    }

    _arestas[no1].push_back(no2);
    _arestas[no2].push_back(no1);

}

void Grafo::InsereAresta(Variable *no1, Variable *no2) {
  
  if (_aumentou ) {
    _arestas.resize(static_cast<size_t>(_proximo) + 3);
    _aumentou = false;
  }
  short int n1 = _nosVariaveis[no1];
  short int n2 = _nosVariaveis[no2];

  _arestas[n2].push_back(n1);
  _arestas[n1].push_back(n2);

}

int Grafo::Grau(int no) {
  return static_cast<int>(_arestas[no].size());
}

bool Grafo::TemAresta(int no1, int no2) {
  unsigned int i;
  for (i=0; i < _arestas[no1].size() ; i++) {
    if ( _arestas[no1][i] == no2 ) 
      return true;
  }
  
  return false;
  
}

/************* DFS *****************/

void Grafo::DFS(int no) {
  
  int indice;
  int prof;
  int noTeste;

  list <int> fila;  
  vector <bool> marca ( _numeroNos);

  for (indice=0; indice < _numeroNos; indice++) { 
     marca[indice] = false;
     _infoNos[indice]._pai   = -1;
     _infoNos[indice]._nivel = -1;
  }
  _dfsPath.erase(_dfsPath.begin(),_dfsPath.end());

  fila.push_front(no);
  _infoNos[no]._nivel = 0;
 
    while ( fila.size() != 0 ) {

        _dfsPath.push_back(no);
        marca[no] = true;
        fila.pop_front();
        prof = _infoNos[no]._nivel + 1;
        int tamanho = static_cast<int>(_arestas[no].size());

        for (indice=0; indice < tamanho; indice++) {
            noTeste = _arestas[no][indice];
            if ( ! marca[noTeste] ) {
                _infoNos[noTeste]._nivel = prof;
                _infoNos[noTeste]._pai = no;
                fila.push_front(noTeste);
            }
        } // for

        while ( fila.size() > 0 ) {
            no = fila.front();
            if ( marca[no] ) fila.pop_front();
            else break;		   
        }

    } // While 
}


/************* Clique ***************/

void Grafo::Clique(LagrangianManager *mestre, int Minimo) {
 
    if (_aumentou ) {
        _arestas.resize(static_cast<size_t>(_proximo) + 3);
        _aumentou = false;
    }

    vector <bool> marca( _numeroNos );
    Constraint* cut;
    bool fim = false;

    size_t maiorGrau = 0;
    size_t grau;

    int candidata;
    int tamanho;
    int maior = 0;
    unsigned int no;
    unsigned int indice;

    GraphNodeIterator it;
    for (it = _nosVariaveis.begin(); it != _nosVariaveis.end(); it++) {
        no = (*it).second;
        grau = _arestas[no].size();
        if ( grau > 0 ) {
            marca[no] = false;
            if (  grau > maiorGrau ) {
	            maiorGrau = grau;
	            maior = no;
            }
        }
        else 
            marca[no] = true;
    } 
    fim = ( maiorGrau == 0 );
  
    while ( ! fim ) {
		cut = new Constraint(1, ConstraintSign::LowerEqual,0.0f,30);
        cut->InsertVariable(_nos[maior], 1);
        marca[maior] = true;
        tamanho = 1;

        //cout << "Escolhido: " << maior->getName() << " " << indice << endl;

        for (indice = 0; indice < _arestas[maior].size(); indice++) {
            candidata = _arestas[maior][indice];
            VariableIterator rest;
            VariableIterator restFim;
            bool podeInserir = true;
      
            cut->ConstraintIterators(rest,restFim);
      
            for (; rest!= restFim; rest++) {
	            if ( ! TemAresta( _nosVariaveis[(*rest)], candidata  ) ) {
	                podeInserir = false;
	                break;
	            }	
            }  
            if ( podeInserir ) {
	            cut ->InsertVariable(_nos[candidata],1);
	            marca[candidata] = true;
                tamanho++;
            }
        }

        if (tamanho >= Minimo ) {
            mestre->InsertCut(cut);
            contacortes++;    
            //cut->Imprime();
        }
        else delete cut;
        fim = true;
        for (it=_nosVariaveis.begin(); it != _nosVariaveis.end(); it++) {
            no = (*it).second;
            if ( ! marca[no] ) {
	            maiorGrau = _arestas[no].size();
	            maior = no;
	            fim = false;
	            break;
            }
        } 
    
    } /* while (! fim ) */
}


/************* Ciclo Impar **************/


void Grafo::CicloImpar(LagrangianManager *mestre) {

    if (_aumentou ) {
        _arestas.resize(static_cast<size_t>(_proximo) + 3);
        _aumentou = false;
    }

    int no,noTeste;
    int indice;
    int grau;

	Constraint* cut;


    no = 0;
    DFS(no);
    grau = static_cast<int>(_arestas[no].size());
    for (indice = 0; indice < grau; indice++) {
        noTeste = _arestas[no][indice];
        if ( ( _infoNos[noTeste]._nivel > 3 ) && (_infoNos[noTeste]._nivel % 2 == 0) && (noTeste > no) ) {
			cut = new Constraint((float)((int)(_infoNos[noTeste]._nivel / 2)), ConstraintSign::LowerEqual, 0.0f, 30);
            while ( noTeste >= 0 ) {
	            cut->InsertVariable(_nos[noTeste],1);
	            noTeste = _infoNos[noTeste]._pai;
            }
            mestre->InsertCut(cut);
            contacortes++;
            break;
        }
    }
   
}

void Grafo::Imprime() {

    if (_aumentou ) {
        _arestas.resize(static_cast<size_t>(_proximo) + 3);
        _aumentou = false;
    }

    cout << _numeroNos << endl;
    unsigned int i = 0;
    unsigned int iFim = _numeroNos; 
    vector <int>::iterator itVizinhos, itVizinhosFim;

    for (; i < iFim; i++) {
        cout << _nos[i]->getName() << " --- ";
        itVizinhos    = _arestas[i].begin();
        itVizinhosFim = _arestas[i].end();
        for (; itVizinhos != itVizinhosFim; itVizinhos++) {
            cout << _nos[*itVizinhos]->getName() << " ";
        }
        cout << _arestas[i].size() << " " << i << " " << iFim << endl;
    }

}

void Grafo::ImprimeNos() {
    for (unsigned int k = 0 ; k < (unsigned int)_numeroNos; k++ ) 
        cout << _nos[k]->getName() << " " << _infoNos[k]._pai << " ";
    cout << endl;
}

void Grafo::ImprimeDFS() {
  Imprime();
  int no;
  list <int>::const_iterator it;
  
  for (it = _dfsPath.begin(); it != _dfsPath.end(); it++ ) {
    no = *it;
    cout << _nos[no]->getName() << " " << no << " " << _infoNos[no]._nivel << " " << _infoNos[no]._pai << endl;
  }


}
