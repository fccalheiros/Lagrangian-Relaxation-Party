#ifndef _VARIABLE_H
#define _VARIABLE_H
#include <vector>



class Constraint;
typedef std::vector<Constraint*>::iterator ConstraintIterator;

class Variable {

 private:
	bool  _fixa = false;
	bool  _isFixedToZero = false;
	bool  _isPricedOut = false;
	bool  _shouldBeCommited = false;

 public:

	int   _name;              
	float _cost;
	float _lagrangianCost;          // Lagrangian cost of the variable in the current iteration. 


	short int _linhasCobertas;
	std::vector <Constraint *> _constraints;

 public:
  
	Variable();
	Variable(float coef, int nome = 0, short int coveredConstraints = 0, float valorLag = 0.0f);
	Variable(Variable* v);

	virtual Variable* CopyAndClean(Variable *v);

	virtual ~Variable();

	int    getName() const { return _name; }
	float  getCost() const { return _cost; }
	float  getLagrangianCost() const { return _lagrangianCost; }

	void setName(int s) { _name = s; }
	void setCost(float cost) { _cost = cost; }
	void setLagrangianCost(float cost) { _lagrangianCost = cost; }
	void addConstraint(Constraint *constraint);

	void initializeLagrangianCost() { _lagrangianCost = _cost; }

	void FixToZero() { _isFixedToZero = true; }
	void UnFix() { _fixa = false; _isFixedToZero = false; }
	bool IsFixed() const { return _fixa || _isFixedToZero; }
	bool IsActive() const { return !_isFixedToZero && !_fixa && !_isPricedOut; }
	void setPricedOut() { _isPricedOut = true; }
	void unsetPricedOut() { _isPricedOut = false; }
	bool IsPricedOut() const { return _isPricedOut; }

	void logicalPriceIn() { _shouldBeCommited = true;  _isPricedOut = false; _isFixedToZero = false; }
	void logicalPriceOut() { _shouldBeCommited = true;  _isPricedOut = true; _isFixedToZero = false; }
	void ClearCommitMark() { _shouldBeCommited = false; }
	bool ShouldBeCommited() const { return _shouldBeCommited; }

	void setCoveredConstraints(int i)  { _linhasCobertas = i; _constraints.reserve(i); }
	int  getCoveredConstraints() const { return _linhasCobertas; }
	void addCoveredConstraints(int i)  { _linhasCobertas += i;  }
	void ResetCoveredConstraints()     { _linhasCobertas = (short int)_constraints.size(); }

	void GetConstraintRange(ConstraintIterator& begin, ConstraintIterator& end) 
		{ begin = _constraints.begin();   end = _constraints.end(); }

	virtual inline bool Intercepta(Variable *varBase) { return false; }

	bool operator==(const Variable& v) { return _name == v._name; }

	bool operator==(const Variable* v) { return this->_name == v->_name; }

};

struct VariablePartitionPolicy {

	void OnMoveToFixed(Variable* v) {
		v->FixToZero();
		v->unsetPricedOut();
	}

	void OnMoveToActive(Variable* v) {
		v->UnFix();
		v->unsetPricedOut();
	}

	void OnMoveToPricedOut(Variable* v) {
		v->setPricedOut();
		v->UnFix();
	}

	void OnCommit(Variable* v) {
		v->ClearCommitMark();
	}

	bool IsActive(Variable* v) const {
		return v->IsActive();
	}

	bool IsFixed(Variable* v) const {
		return v->IsFixed();
	}

	bool IsPricedOut(Variable* v) const {
		return v->IsPricedOut();
	}
};

#endif
