#include "Variable.h"
#include "Constraint.h"


Constraint::Constraint()
    : Constraint(1.0f, ConstraintSign::Equal, 0.0f, 50) {
}

Constraint::Constraint(float rhs, ConstraintSign sign = ConstraintSign::Equal, float ml = 0, int varCount = 50):
    _rhs(rhs),
    _direction(sign),
    _lagrangean(ml),
    _index(0),
    _deleted(false),
    _covered(false)
{
    _variables.reserve(varCount);
}

Constraint::Constraint(Constraint* r)
    : Constraint(r->_rhs, r->_direction, 0.0f, static_cast<int>(r->_variables.size()))
{
    _index = r->_index;
    _deleted = r->_deleted;
    _covered = r->_covered;
}


Constraint* Constraint::CopyAndClean(Constraint* r) 
{
    if (r == NULL) {
        r = new Constraint(this);
    }
    else {
        r->_rhs = _rhs;
        r->_direction = _direction;
        r->_lagrangean = 0;
        r->_index = _index;
        r->_covered = _covered;
    }
    return r;
}


Constraint::~Constraint() 
{
    _variables.erase(_variables.begin(), _variables.end());
}


int Constraint::Degree() {
    VariableIterator it, itFim;
    ConstraintIterators(it, itFim);
    return static_cast<int>(distance(it,itFim));
}


bool Constraint::setDirection(ConstraintSign dir) {
    _direction = dir;
    return true;
}

void Constraint::setLagrangean(float ml) {
    _lagrangean = ml;
    if ( _direction  == ConstraintSign::LowerEqual ) {
        if ( _lagrangean > 0 )
            _lagrangean = 0;
    }

}


void Constraint::ConstraintIterators(VariableIterator& begin, VariableIterator& end) {
    begin = _variables.begin();
    end   = _variables.end();
}

void Constraint::SortVariablesByName()
{ 
    VariableIterator it;
    VariableIterator itFim;
    ConstraintIterators(it, itFim);
    sort(it, itFim, CompareNames <Variable*>());
}

void Constraint::CleanUpConstraint() {
    _variables.erase(_variables.begin(), _variables.end());
}



void Constraint::InsertVariable(Variable* var, float coef) {
    size_t i = _variables.size();
    size_t j = _variables.capacity();
    if (i == j) {
        i = static_cast<size_t>(i * 1.2);
        _variables.reserve(i);
    }
    _variables.push_back(var);
}

void Constraint::RemoveVariable(VariableIterator & it) {
    _variables.erase(it);
}

float Constraint::getIntercession(vector <Variable*>& sol)
{
    float count = 0;
    size_t solSize = sol.size();
    size_t i;
    VariableIterator begin, end;

    for (ConstraintIterators(begin, end), i = 0; i < solSize; i++) {
        begin = lower_bound(begin, end, sol[i], CompareNames <Variable*>());
        if (begin == end) return count;
        if ((*begin)->_nome == sol[i]->_nome)
            count++;
    }

    return count;
}


float Constraint::getCoefficient(Variable* var)
{
    VariableIterator it, itFim;

    ConstraintIterators(it, itFim);
    if (binary_search(it, itFim, var, CompareNames <Variable*>()))
        return 1;
    return 0;
}



void Constraint::Print(FILE* fp) {
    int i = 1;
    VariableIterator it, itFim;

    ConstraintIterators(it, itFim);

    if (it == itFim) return;

    fprintf(fp, "x%d", (*it)->retNome());

    for (it++; it != itFim; it++) {
        if ((i % 12) == 0) fprintf(fp, "\n");
        fprintf(fp, " + x%d", (*it)->retNome());
        i++;
    }
    if (_direction == ConstraintSign::LowerEqual)
        fprintf(fp, "<");

    fprintf(fp, "= %f\n", _rhs);
    fflush(fp);
}


void Constraint::PrintBeasley() {
    int i = 0;
    VariableIterator it, itFim;
    
    ConstraintIterators(it, itFim);
    cout << distance(it,itFim);

    for (; it != itFim; it++) {
        if ((i % 12) == 0) cout << "\n";
        cout << (((*it)->_nome) + 1) << " ";
        i++;
    }
}