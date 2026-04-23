#include "RGPCut.h"

RGPCut::RGPCut():
    Constraint(1,ConstraintSign::LowerEqual,0,30),
    _key(0) 
{}

RGPCut::RGPCut(float rhs):
    Constraint(rhs,ConstraintSign::LowerEqual,0,30),
    _key(0) 
{}

RGPCut::~RGPCut() { }

void RGPCut::InsertVariable(Variable *var, float  coef)  {   
    _key += var->_nome;
    Constraint::InsertVariable(var,coef);
}

bool RGPCut::Compare( RGPCut * rest ) {

    if ( _key != rest->_key ) 
        return false;

    if ( _variables.size() != rest->_variables.size() ) 
        return false;

    VariableIterator comeco,fim;
    rest->ConstraintIterators(comeco,fim);
    for (;comeco !=  fim; comeco++) {
        if ( getCoefficient( (*comeco) ) == 0 ) 
        return false;
    }

    if ( _rhs > rest->_rhs ) 
        _rhs = rest->_rhs;

  return true;
  
}
/*
bool RGPCut::operator== ( RGPCut * rest ) {

    if ( _key != rest->_key ) 
        return false;
    if ( _variables.size() != rest->_variables.size() ) 
        return false;

    ConstraintIterator comeco,fim;
    rest->ConstraintIterators(comeco,fim);
    for (;comeco !=  fim; comeco++) {
        if ( getCoefficient( (*comeco) ) == 0 )
            return false;
    }
  
    return true;
  
}

*/