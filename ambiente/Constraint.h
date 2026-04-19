#ifndef _CONSTRAINT_H
#define _CONSTRAINT_H

#include "Variable.h"
#include <algorithm>
#include <iostream>


#define LOWER_EQUAL   1
#define EQUAL         2
#define GREATER_EQUAL 3

typedef vector<Variable*>::iterator VariableIterator;

class Constraint {

public:

	vector <Variable *> _variables;
	float _lagrangean;
	short int _index;

protected:

	float _rhs;
	char _direction;
	bool _deleted;
	bool _covered;


public:

	Constraint();
	Constraint(float rhs, char dir, float ml, int varCount);          // Alterada... apaguei os valores default que estão no cpp
	Constraint(Constraint* r);

	virtual Constraint* CopyAndClean(Constraint* r);

	virtual ~Constraint();

	virtual float getCoefficient(Variable* var);
	virtual void InsertVariable(Variable* var, float coef);
	void  RemoveVariable(VariableIterator & it);

	float getIntercession(vector <Variable*>& sol);

	int Degree();

	void Print(FILE* fp = stdout);
	void PrintBeasley();
	virtual void Print() { cout << "Base Class Constraint dont implement Print Method." << endl; };

	void SortVariablesByName();
	void ConstraintIterators(VariableIterator& begin, VariableIterator& end);
	void CleanUpConstraint();


	float getRHS() { return _rhs; }
	int   getDirection() { return _direction; }
	float getLagrangean() { return _lagrangean; }

	void setRHS(float rhs) { _rhs = rhs; }
	bool setDirection(char dir);
	void setLagrangean(float ml);

	void LogicalDelete()  { _deleted = true; }
	bool LogicalDeleted() { return _deleted; }

	void Cover()      { _covered = true;  }
	void Uncover()    { _covered = false; }
	bool IsCovered()  { return _covered;  }

};

template <class T> struct  CompareNames  {
public:
	CompareNames() { }

	bool operator() (Variable* v1, Variable* v2) {
		if (v1->_nome < v2->_nome)
			return true;
		return false;
	}

};
#endif


