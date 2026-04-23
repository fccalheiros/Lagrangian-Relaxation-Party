#include "Variable.h"

Variable::Variable():
    _nome(0),
    _valorFO(0),
    _valorLag(0),
    _marca(false),
    _fixaEmZero(false),
    _fixa(false),
    _linhasCobertas(0)
{}

Variable::Variable(float coef, int nome):
    _nome(nome),
    _valorFO(coef),
    _valorLag(coef),
    _marca(false),
    _fixaEmZero(false),
    _fixa(false),
    _linhasCobertas(0)
{}

Variable::Variable(Variable* v): 
    _nome(v->_nome),
    _valorFO(v->_valorFO),
    _valorLag(0),
    _marca(false),
    _fixaEmZero(false),
    _fixa(false),
    _linhasCobertas(v->_linhasCobertas)
{
}

Variable* Variable::CopyAndClean(Variable* v) {

    if (v == NULL) {
        v = new Variable(this);
    }
    else {
        v->_nome = _nome;
        v->_valorFO = _valorFO;
        v->_linhasCobertas = _linhasCobertas;

        v->_valorLag = 0;
        v->_marca = false;
        v->_fixaEmZero = false;
        v->_fixa = false;
    }

    v->_constraints.reserve(_constraints.size());

    return v;
}


Variable::~Variable() {
}

int Variable::retNome () const {
    return _nome;
}

float Variable::retCusto() const {
    return _valorFO;
}

float Variable::retCustoLag() const {
    return _valorLag;
}

void Variable::poeNome (int s) {
    _nome = s;
}

void Variable::poeCusto(float coef) {
    _valorFO = coef;
}

void Variable::poeCustoLag(float custo) {
    _valorLag = custo;
}

void Variable::poeRestricao(Constraint *restricao) {
    size_t i = _constraints.size();
    size_t j = _constraints.capacity();
    if ( i == j ) {
        i = static_cast<size_t>(i * 1.2);
        _constraints.reserve(i);
    }
    _constraints.push_back(restricao);
}

void Variable::iniciaCustoLag() {
    _valorLag = _valorFO;
}

void Variable::FixaEmZero() {
  _fixaEmZero = true;
}

void Variable::RetiraFixZero() {
  _fixaEmZero = false;
}

bool Variable::EstaFixada() const {
  return _fixaEmZero;
}

void Variable::Marca() {
  _marca = true;
}

void Variable::Desmarca() {
  _marca = false;
}

bool Variable::Valida() const {
  if ( _fixaEmZero ) return false;
  if ( _fixa ) return false;
  return true;
}

