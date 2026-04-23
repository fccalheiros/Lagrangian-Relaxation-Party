#ifndef _VARIABLE_H
#define _VARIABLE_H
#include <vector>

using namespace std;



class Constraint;
typedef vector<Constraint*>::iterator ConstraintIterator;

class Variable {

 public:

	int   _nome;              
	float _valorFO;
	float _valorLag;          // custo Lagrangeano da variavel
	bool  _marca;
	bool  _fixaEmZero;
	bool  _fixa;

	short int _linhasCobertas;
 
	vector <Constraint *> _constraints;

 public:
  
	Variable();
	Variable(float coef, int nome = 0);
	Variable(Variable* v);

	virtual Variable* CopyAndClean(Variable *v);

	virtual ~Variable();

	int    retNome() const;
	float  retCusto() const;
	float  retCustoLag() const;  

	void poeNome(int s);
	void poeCusto(float coef);
	void poeCustoLag(float custo);
	void poeRestricao(Constraint *restricao);

	void iniciaCustoLag();

	void FixaEmZero();
	void RetiraFixZero();
	bool EstaFixada() const;
  
	void Marca(); 
	void Desmarca();
	bool Valida() const;

	void setCoveredConstraints(int i) { _linhasCobertas = i; _constraints.reserve(i); }
	int  getCoveredConstraints() const { return _linhasCobertas; }
	void addCoveredConstraints(int i) { _linhasCobertas += i;  }
	void ResetCoveredConstraints()    { _linhasCobertas = (short int)_constraints.size(); }

	void ConstraintsBounds(ConstraintIterator& begin, ConstraintIterator& end) { begin = _constraints.begin();   end = _constraints.end(); }



	virtual inline bool Intercepta(Variable *varBase) { return false; }

	bool operator==(const Variable& v) { return _nome == v._nome; }

	bool operator==(const Variable* v) { return this->_nome == v->_nome; }



};

#endif
