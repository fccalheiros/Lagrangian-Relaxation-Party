#ifndef _VARIABLE_H
#define _VARIABLE_H
#include <vector>

using namespace std;

class Constraint;
typedef vector<Constraint*>::iterator ConstraintIterator;

class Variable {

 private:
	bool  _fixa = false;
	bool  _isFixedToZero = false;
	bool  _isPricedOut = false;

 public:

	int   _nome;              
	float _valorFO;
	float _valorLag;          // Lagrangian cost of the variable in the current iteration. 


	short int _linhasCobertas;
	vector <Constraint *> _constraints;

 public:
  
	Variable();
	Variable(float coef, int nome = 0, short int coveredConstraints = 0, float valorLag = 0.0f);
	Variable(Variable* v);

	virtual Variable* CopyAndClean(Variable *v);

	virtual ~Variable();

	int    getName() const { return _nome; }
	float  getCost() const { return _valorFO; }
	float  getLagrangianCost() const { return _valorLag; }

	void setName(int s) { _nome = s; }
	void setCost(float cost) { _valorFO = cost; }
	void setLagrangianCost(float cost) { _valorLag = cost; }
	void addConstraint(Constraint *constraint);

	void initializeLagrangianCost() { _valorLag = _valorFO; }

	void FixToZero() { _isFixedToZero = true; }
	void UnFix() { _fixa = false; _isFixedToZero = false; }
	bool IsFixed() const { return _fixa || _isFixedToZero; }
	bool IsValid() const { return !_isFixedToZero && !_fixa; }
	void setPricedOut() { _isPricedOut = true; }
	void unsetPricedOut() { _isPricedOut = false; }
	bool IsPricedOut() const { return _isPricedOut; }


	void setCoveredConstraints(int i)  { _linhasCobertas = i; _constraints.reserve(i); }
	int  getCoveredConstraints() const { return _linhasCobertas; }
	void addCoveredConstraints(int i)  { _linhasCobertas += i;  }
	void ResetCoveredConstraints()     { _linhasCobertas = (short int)_constraints.size(); }

	void GetConstraintRange(ConstraintIterator& begin, ConstraintIterator& end) 
		{ begin = _constraints.begin();   end = _constraints.end(); }

	virtual inline bool Intercepta(Variable *varBase) { return false; }

	bool operator==(const Variable& v) { return _nome == v._nome; }

	bool operator==(const Variable* v) { return this->_nome == v->_nome; }

};

#endif
