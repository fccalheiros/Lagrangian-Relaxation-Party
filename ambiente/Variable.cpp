#include "Variable.h"

Variable::Variable(float coef, int nome, short int coveredConstraints,float valorLag):
    _name(nome),
    _cost(coef),
    _lagrangianCost(valorLag),
    _linhasCobertas(coveredConstraints)
{}

Variable::Variable() : 
    Variable( 0.0f, 0, 0, 0.0f) {
}

Variable::Variable(Variable* v) :
    Variable(v->_cost, v->_name, v->_linhasCobertas, 0.0f)
{
}

Variable* Variable::CopyAndClean(Variable* v) {

    if (v == NULL) {
        v = new Variable(this);
    }
    else {
        v->_name = _name;
        v->_cost = _cost;
        v->_linhasCobertas = _linhasCobertas;

        v->_lagrangianCost = 0;
        v->_isFixedToZero = false;
        v->_fixa = false;
		if (_isPricedOut)
            v->setPricedOut();
        else 
			v->unsetPricedOut();
    }

    v->_constraints.reserve(_constraints.size());

    return v;
}


Variable::~Variable() {
}


void Variable::addConstraint(Constraint *constraint) {
    size_t i = _constraints.size();
    size_t j = _constraints.capacity();
    if ( i == j ) {
        i = static_cast<size_t>(i * 1.2);
        _constraints.reserve(i);
    }
    _constraints.push_back(constraint);
}



