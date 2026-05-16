#ifndef _CONSTRAINT_H
#define _CONSTRAINT_H

#include "Variable.h"
#include <algorithm>
#include <iostream>



enum class ConstraintSign {
	LowerEqual = 1,  // ?
	Equal = 2,  // =
	GreaterEqual = 3   // ?
};

typedef std::vector<Variable*>::iterator VariableIterator;

class Constraint {

public:

	std::vector <Variable *> _variables;
	float _lagrangian;
	short int _index;

protected:

	float _rhs;
	ConstraintSign _direction;
	bool _deleted;
	bool _covered;
	int _key;


public:

	Constraint();
	Constraint(float rhs, ConstraintSign sign, float ml, int varCount);          // Alterada... apaguei os valores default que estão no cpp
	Constraint(Constraint* r);

	virtual Constraint* CopyAndClean(Constraint* r);

	virtual ~Constraint();

	virtual float getCoefficient(Variable* var);
	virtual void InsertVariable(Variable* var, float coef);
	void  RemoveVariable(VariableIterator & it);

	float getIntercession(std::vector <Variable*>& sol);

	int Degree();

	void Print(FILE* fp = stdout);
	void PrintBeasley();
	virtual void Print() { std::cout << "Base Class Constraint dont implement Print Method." << std::endl; };

	void SortVariablesByName();
	void ConstraintIterators(VariableIterator& begin, VariableIterator& end);
	void CleanUpConstraint();


	float getRHS() const  { return _rhs; }
	ConstraintSign  getDirection()  const { return _direction; }
	float getLagrangian() const { return _lagrangian; }
	int getKey() const { return _key; }

	void setRHS(float rhs) { _rhs = rhs; }
	bool setDirection(ConstraintSign dir);
	void setLagrangian(float ml);

	void LogicalDelete()  { _deleted = true; }
	bool LogicalDeleted() const { return _deleted; }

	void Cover()      { _covered = true;  }
	void Uncover()    { _covered = false; }
	bool IsCovered() const { return _covered;  }
	virtual bool Compare(Constraint* other);
};

template <class T> struct  CompareNames  {
public:
	CompareNames() { }

	bool operator() (Variable* v1, Variable* v2) {
		if (v1->_name < v2->_name)
			return true;
		return false;
	}

};
#endif


