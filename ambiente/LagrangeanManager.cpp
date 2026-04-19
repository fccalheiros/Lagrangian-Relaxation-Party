#include "LagrangeanManager.h"
#include "algoritmo.h"
#include <sstream>
#include <iomanip>
#include <fstream>



LagrangeanManager::LagrangeanManager(Configuration * config):
    _algo(NULL),
    _direction(MINIMIZA),
    _countConstraints(0),
    _countConstraintsND(0),
    _cutsInserted(0),
    _cutsFound(0),
    _cutsRemoved(0),
    _countFixed(0),
    _countFixedPartial(0),
    _totalVariaveis(0),
    _config(config)
{
  setLowerBound(_config->MINUS00);
  setUpperBound(_config->PLUS00);
  _variables.reserve(5000);
}

LagrangeanManager::LagrangeanManager(Configuration* config, Algoritmo * algo, char direcao):
    _algo(algo),
    _direction(direcao),
    _countConstraints(0),
    _countConstraintsND(0),
    _cutsInserted(0),
    _cutsFound(0),
    _cutsRemoved(0),
    _countFixed(0),
    _countFixedPartial(0),
    _totalVariaveis(0),
    _config(config)
{   
    setLowerBound(_config->MINUS00);
    setUpperBound(_config->PLUS00);
    _variables.reserve(5000);
   
}

LagrangeanManager::LagrangeanManager(LagrangeanManager* m):
    _algo(m->_algo),
    _direction(m->_direction),
    _countConstraints(0),
    _countConstraintsND(0),
    _cutsInserted(0),
    _cutsFound(0),
    _cutsRemoved(0),
    _countFixed(0),
    _countFixedPartial(0),
    _totalVariaveis(m->_totalVariaveis),
    _config(m->_config)
{
    setLowerBound(_config->MINUS00);
    setUpperBound(_config->PLUS00);
    _variables.reserve(5000);
 
}

LagrangeanManager* LagrangeanManager::CopyAndClean(LagrangeanManager* m) {
    if (m == NULL) {
        m = new LagrangeanManager(this);
    }
    else 
    {
        m->_upperBound           = _config->PLUS00;
        m->_lowerBound           = _config->MINUS00;
        m->_algo                 = _algo;
        m->_direction            = _direction;
        m->_countConstraints     = 0;
        m->_countConstraintsND   = 0;
        m->_cutsInserted         = 0;
        m->_cutsFound            = 0;
        m->_cutsRemoved          = 0;
        m->_countFixed           = 0;
        m->_countFixedPartial    = 0;
        m->_totalVariaveis       = _totalVariaveis;
        m->_config               = _config;
    }

    VariableIterator var, varFim;
    ConstraintIterator rest, restFim;


    ConstraintsBounds(rest, restFim);
    for (; rest != restFim; rest++) {
        m->InsertConstraint((*rest)->CopyAndClean(NULL));
    }

    ConstraintsNDBounds(rest, restFim);
    for (; rest != restFim; rest++) {
        m->InsertConstraintND((*rest)->CopyAndClean(NULL));
    }

    VariableBounds(var, varFim);
    for (; var != varFim; var++) {

        //copy variable
        Variable* v = (*var)->CopyAndClean(NULL);
        m->InsertVariable(v);
        
        //insert in restrictions
        rest = (*var)->_constraints.begin();
        restFim = (*var)->_constraints.end();
        for (; rest != restFim; rest++)
            v->poeRestricao( m->_constraints[(*rest)->_index] );

        //Insert the copied variable in best solution vector if it was in the source manager best solution.

        VariableIterator bestSolutionIt = _best.begin();  
        for (; bestSolutionIt != _best.end(); bestSolutionIt++)
            if ((*bestSolutionIt)->_nome == v->_nome) {
                m->_best.push_back(v->CopyAndClean(NULL));
                break;
            }
    }

    //cuts are not copied

    m->setUpperBound(getUpperBound());
    m->_end = m->_variables.end();

    return m;
}

LagrangeanManager::~LagrangeanManager() {
    FreeMemory();
}

void LagrangeanManager::FreeMemory() {

    VariableIterator varIni,varFim, varLixo;
    ConstraintIterator proIni,proFim, proLixo;

    VariableBounds(varIni,varFim);
    for (; varIni != varFim;) {
        varLixo = varIni;
        varIni++;
        delete *varLixo;
    }

    ConstraintsBounds(proIni, proFim);
    for (; proIni != proFim;) {
        proLixo = proIni;
        proIni++;
        delete *proLixo;
    }

    ConstraintsNDBounds(proIni, proFim);
    for (; proIni != proFim;) {
        proLixo = proIni;
        proIni++;
        delete* proLixo;
    }

    CutsBounds(proIni, proFim);
    for (; proIni != proFim;) {
        proLixo = proIni;
        proIni++;
        delete* proLixo;
    }

    vector <Variable*>().swap(_variables);
    vector<Constraint*>().swap(_constraints);
    vector<Constraint*>().swap(_constraintsND);
    vector<Constraint*>().swap(_cuts);

    _end = _variables.end();

}

void LagrangeanManager::GenerateProblem(char *arq) {  
  ReadProblem(arq);
  CreateProblem();
  FinalizeProblemCreation();
}

void LagrangeanManager::FinalizeProblemCreation() {
    _variables.shrink_to_fit();
    _end = _variables.end();


    for (int i = 0; i < _countConstraints; i++) {
        _constraints[i]->_index = i;
    }

    VariableIterator vIt, vEnd;
    for (VariableBounds(vIt, vEnd); vIt != vEnd; vIt++) {
        (*vIt)->ResetCoveredConstraints();
    }
}

void LagrangeanManager::CheckBounds(float valRelaxado, float valHeuristica, vector <Variable *> &solHeu, bool resHeuristica){

    float LI = getLowerBound();

    if ( _direction == MINIMIZA ) {   
        if ( valRelaxado > LI ) 
            setLowerBound(valRelaxado);
        if ( resHeuristica ) 
            if ( valHeuristica <= getUpperBound() ) {
	            setUpperBound(valHeuristica);
                StoreIncumbent(solHeu);
            }
    }
    else {
        if ( valRelaxado < getUpperBound() )  
            setUpperBound(valRelaxado);
        if ( resHeuristica ) 
            if ( valHeuristica >= LI ) {
    	        setLowerBound(valHeuristica);
                StoreIncumbent(solHeu);
            }
     }
}


void LagrangeanManager::Solve(float InitialCost, float KnownBound ) { 
  
    bool testeParada = false;
    bool resHeuristica;
    Solucao relaxada;
    Solucao heuristica;
    float valorRelaxado;
    float valorHeuristica;
      
    _algo->setLagrangeanManager(this);  
    _algo->Inicializacao();
    setBound(KnownBound);

    while ( ! testeParada ) {
    
        CleanUp();
        CustomProcessing();
        _algo->Relaxacao(relaxada, valorRelaxado, InitialCost);
        _algo->FixaVariaveis(relaxada, valorRelaxado, InitialCost);
        resHeuristica = _algo->Heuristica(relaxada, heuristica, valorHeuristica, InitialCost);
        CheckBounds(valorRelaxado, valorHeuristica, heuristica, resHeuristica);
        _algo->GeraCortes(relaxada);
        _algo->SubGradiente(relaxada);
        testeParada = _algo->TesteParada();
    }

    _algo->Finalizacao();
    cout << endl << PrintVariableVector(relaxada) << endl;
    FinalStats();
}


void LagrangeanManager::SetVariableForBranch(Variable* v, short int value) {

    VariableIterator vIt,vBegin, vEnd, vBranch, vRecicle;
    bool sai;

    VariableBounds(vBranch, vEnd);

    for (; vBranch != vEnd; vBranch++) {
        if (v->_nome == (*vBranch)->_nome)
            break;
    }

    if (vBranch == vEnd) return;

    //remove branch variable from Primal Solution 
    vIt = _best.begin();
    vEnd = _best.end();
    for (; vIt != vEnd; vIt++) {
        if ((*vBranch)->_nome == (*vIt)->_nome) {
            _best.erase(vIt);
            break;
        }
    }

    if (value == 0) {
        FixVariable(vBranch);
    }
    else
    { 
        ConstraintIterator  cIt, cEnd, cBegin, cRecicle;

        //Delete logically all constraints from the choosen variable
        cIt = (*vBranch)->_constraints.begin();
        cEnd = (*vBranch)->_constraints.end();
        for (; cIt != cEnd; cIt++) {
            (*cIt)->LogicalDelete();
        }


        //Remove variables that were in logically deleted constraints 
        //Removal will include vBranch 
        VariableBounds(vBegin, vIt);
        sai = (vIt == vBegin);
        if (!sai) vIt--;
        while (!sai) {
            sai = (vIt == vBegin);

            cIt = (*vIt)->_constraints.begin();
            cEnd = (*vIt)->_constraints.end();
            for (; cIt != cEnd; cIt++) {
                if ((*cIt)->LogicalDeleted()) {
                    vRecicle = vIt;
                    if (!sai) vIt--;
                    RemoveVariable(vRecicle);
                    break;
                }
            }

            if (cIt == cEnd && !sai)
                    vIt--;
        }

        //delete fisically all logically deleted constraints
        ConstraintsBounds(cBegin, cIt);
        sai = (cIt == cBegin);
        if (!sai) cIt--;
        while (!sai) {
            sai = (cIt == cBegin);
            if ((*cIt)->LogicalDeleted()) {
                cRecicle = cIt;
                if (!sai) cIt--;
                RemoveConstraint(cRecicle);
            }
            else if (!sai) cIt--;
        }

        ConstraintsNDBounds(cBegin, cIt);
        sai = (cIt == cBegin);
        if (!sai) cIt--;
        while (!sai) {
            sai = (cIt == cBegin);
            if ((*cIt)->LogicalDeleted()) {
                cRecicle = cIt;
                if (!sai) cIt--;
                RemoveConstraint(cRecicle);
            }
            else if (!sai) cIt--;
        }

        //rebuild index so a new branch will work 

        for (unsigned int i = 0; i < _constraints.size(); i++) {
            _constraints[i]->_index = i;
        }
        for (unsigned int i = 0; i < _constraintsND.size(); i++) {
            _constraintsND[i]->_index = i;
        }

        _end = _variables.end();

    }


}

void LagrangeanManager::FixVariable(VariableIterator var) {
    (*var)->_fixaEmZero = true;
    _end--;
    _countFixed++;
    _countFixedPartial++;
    iter_swap(var,_end);
}

void LagrangeanManager::FixLastVariable() {
    _end--;
    (*_end)->_fixaEmZero = true;
    _countFixed++;
    _countFixedPartial++;
}


void LagrangeanManager::InsertVariable(Variable *var) {
    unsigned int i = _variables.size();
    unsigned int j = _variables.capacity();
    if ( i == j ) {
        i = (unsigned int)(i * 1.2);
        _variables.reserve(i);
    }
    _variables.push_back(var);
}

void LagrangeanManager::InsertConstraint(Constraint *restricao) {
    _constraints.push_back(restricao);
    _countConstraints++;
}

void LagrangeanManager::InsertConstraintND(Constraint *restricao) {
    _constraintsND.push_back(restricao);
    _countConstraintsND++;
}

// Use only with regular constraints 
// Dont use with cuts
void LagrangeanManager::MarkConstraintForDeletion(Variable* var) {

    int set = 0;
    ConstraintIterator it = var->_constraints.begin();
    ConstraintIterator end = var->_constraints.end();

    for (; it != end; it++) {
        (*it)->LogicalDelete();
        set++;
    }
    _countConstraints = _constraints.size() - set;
    cout << "Restricoes Marcadas: " << set << endl;
}


void LagrangeanManager::InsertCut(Constraint *restricao) {
    ConstraintIterator comeco;
    ConstraintIterator fim;
    CutsBounds(comeco, fim);
    _cutsFound++;
    for (;comeco != fim; comeco++) {
        if ( ((RGPCut *)(*comeco))->Compare( (RGPCut *) restricao ) )
            return; 
    } 
    restricao->SortVariablesByName();
    _cutsInserted++;
    _cuts.push_back(restricao);
}

void LagrangeanManager::RemoveVariable(VariableIterator &it) {
      delete *it;
      _variables.erase(it);
}

void LagrangeanManager::RemoveConstraint(ConstraintIterator &it) {
    delete *it;
    _constraints.erase(it);
    _countConstraints--;
}

void LagrangeanManager::RemoveConstraintND(ConstraintIterator &it) {
    delete *it;
    _constraintsND.erase(it);
    _countConstraintsND--;
}

void LagrangeanManager::RemoveCut(ConstraintIterator &it) {
    //cout << "RGPCut removido : " << endl; ((Constraint *)(*it))->Imprime();
    delete *it;
    _cuts.erase(it);
    _cutsRemoved++;
}

int LagrangeanManager::ActiveVariables() {
    return ( distance(_variables.begin(),_end) );
} 

void LagrangeanManager::VariableBounds(VariableIterator& comeco, VariableIterator& fim) {
    comeco = _variables.begin();
    fim = _end;
}

void LagrangeanManager::ConstraintsBounds(ConstraintIterator &comeco, ConstraintIterator &fim){
    comeco = _constraints.begin();
    fim    = _constraints.end();
}

void LagrangeanManager::ConstraintsNDBounds(ConstraintIterator &comeco, ConstraintIterator &fim){
    comeco = _constraintsND.begin();
    fim    = _constraintsND.end();
}

void LagrangeanManager::CutsBounds(ConstraintIterator &comeco, ConstraintIterator &fim){
    comeco = _cuts.begin();
    fim    = _cuts.end();
}

Constraint * LagrangeanManager::getConstraint (int i) {
    return _constraints[i];
}

Constraint * LagrangeanManager::getConstraintND (int i) {
    return _constraintsND[i];
}

Constraint * LagrangeanManager::getCut (int i) {
    return _cuts[i];
}

void LagrangeanManager::CoveredConstraints(Variable *var, vector <Constraint *> &linhas) {

    ConstraintIterator rest,fim;

    linhas.erase(linhas.begin(),linhas.end());
    ConstraintsBounds(rest,fim); 

    for (; rest != fim; rest++) {
        if ( (*rest)->getCoefficient(var) != 0 ) 
            linhas.push_back( (*rest) );
    }

}


void LagrangeanManager::PrintLagrangean() {
    unsigned int i = 0;
    int j;
    bool prim;
    Variable *var;
    j = 0;
    prim = true;
    for (i=0;i< _variables.size();i++) {
        var = _variables[i];
        if ( ! (var->_fixa || var->_fixaEmZero) ) {
            j++;
            if ( (j % 12) == 0 )  cout << endl;
            if ( ! prim )  cout << " + ";
                prim = false;
            cout << var->_valorLag << " x" << _variables[i]->retNome();
        }
    }
}


void LagrangeanManager::ImprimeLP(FILE *saida) {
    unsigned int i = 0;
    int j;
    bool prim;
    Variable *var;
    Ordena ( CompareNames <Variable *> () );   
   
    fprintf(saida,"minimize\n");

    j = 0;
    prim = true;
    for (i=0;i < _variables.size() ;i++) {
        var = _variables[i];
        if ( ! (var->_fixa || var->_fixaEmZero) ) {
            j++;
            if ( (j % 12) == 0 )  
                fprintf(saida,"\n");
            if ( ! prim )  
                fprintf(saida," + ");
            prim = false;
            fprintf(saida,"%1.0f x%d",var->retCusto(),var->retNome());
        }
    }
    fprintf(saida,"\nsubject to\n");    
    Constraint * rest;
    VariableIterator it,itFim;

    for (i=0; i < _constraintsND.size(); i++) {
        prim = true;
        rest = _constraintsND[i];
        ((Constraint *)rest)->ConstraintIterators(it,itFim);
        j = 0;
        for ( ; it != itFim ; it++ ) {
            var = (*it);
            if ( ! (var->_fixa || var->_fixaEmZero) )  {
	            if ( ! prim ) fprintf(saida," + "); 
	            prim = false; 
	            fprintf(saida,"x%d",var->_nome);
	            j++;
	            if ( (j%12) == 0 ) fprintf(saida,"\n");
            }
       }
       fprintf(saida," = %2.2f\n",rest->getRHS()); 
    }
   //***********
    vector < vector <int> > matriz;
    matriz.resize(_constraints.size());
    VariableIterator  vComeco, vFim;

    for (j=0; j < (int)_variables.size() ;j++) {
        var = _variables[j];   
        if ( ! (var->_fixa || var->_fixaEmZero) ) {
	        for (i = 0; (int) i < var->_linhasCobertas; i++) {
	            matriz[var->_constraints[i]->_index].push_back(var->_nome);
	        }
        }
    }

    for (i=0; i < _constraints.size(); i++) {
        prim = true;
        for ( j=0; j < (int)matriz[i].size() ; j++ ) {
	        if ( ! prim ) fprintf(saida," + "); 
	        prim = false; 
	        fprintf(saida,"x%d", matriz[i][j] );
	        if ( ((j+1)%12) == 0 ) fprintf(saida,"\n");
        }
        fprintf(saida," = 1\n"); 
    }
   //********

    for (i=0; i < _cuts.size(); i++) {
        prim = true;
        rest = _cuts[i];
        ((Constraint *)rest)->ConstraintIterators(it,itFim);
        j = 0;
        for ( ; it != itFim ; it++ ) {
	        var = (*it);
	        if ( ! (var->_fixa || var->_fixaEmZero) )  {
	            if ( ! prim ) fprintf(saida," + "); 
	            prim = false; 
	            fprintf(saida,"x%d",var->_nome);
	            j++;
	            if ( (j%12) == 0 ) fprintf(saida,"\n");
	        }
        }
        fprintf(saida," <= %1.0f\n",rest->getRHS());
    }
    fprintf(saida,"binary\n");
    j = 0;
    for (i=0;i< _variables.size();i++) {
        var = _variables[i];
        if ( ! (var->_fixa || var->_fixaEmZero) ) {
            j++;
            if ( (j % 12) == 0 )  fprintf(saida,"\n");
            fprintf(saida,"x%d ",var->retNome());
        }
    }
    fprintf(saida,"end\n");
    fflush(saida);
}

string LagrangeanManager::PrintVariableVector(Solucao s) {
    stringstream work;

    VariableIterator vIt = s.begin();
    for (; vIt != s.end(); vIt++) {
        work << "x" << (*vIt)->retNome() << " : " << (*vIt)->retCusto() << " : " << (*vIt)->retCustoLag() <<  endl;
    }
    return work.str();
}

void LagrangeanManager::PrintLP(string filename) {
    ofstream file;
    file.open(filename);
    file << PrintLP();
    file.close();
}

string LagrangeanManager::PrintLP() {
  
    stringstream work;

    unsigned int i = 0;
    int j;
    bool first;
    Variable* var;

    Ordena(CompareNames <Variable*>());

    work << "minimize" << endl;

    j = 0;
    first = true;
    for (i = 0; i < _variables.size(); i++) {
        var = _variables[i];
        if (!(var->_fixa || var->_fixaEmZero)) {
            j++;
            if ((j % 12) == 0)
                work << endl;
            if (!first)
                work << " + ";
            first = false;
            work << std::fixed << std::setprecision(0) << var->retCusto() << " x" << var->retNome();
        }
    }
    work << endl << "subject to" << endl;


    Constraint* rest;
    VariableIterator it, itFim;

    for (i = 0; i < _constraintsND.size(); i++) {
        first = true;
        rest = _constraintsND[i];
        ((Constraint*)rest)->ConstraintIterators(it, itFim);
        j = 0;
        for (; it != itFim; it++) {
            var = (*it);
            if (!(var->_fixa || var->_fixaEmZero)) {
                if (!first) 
                    work << " + ";
                first = false;
                work << "x" << var->retNome();
                j++;
                if ((j % 12) == 0)
                    work << endl;
            }
        }
        work << " = " << std::fixed << std::setprecision(0) << rest->getRHS() << endl;

    }
    //***********
    vector < vector <int> > matrix;
    matrix.resize(_constraints.size());
    VariableIterator  vComeco, vFim;

    for (j = 0; j < (int)_variables.size(); j++) {
        var = _variables[j];
        if (!(var->_fixa || var->_fixaEmZero)) {
            for (i = 0; (int)i < var->_linhasCobertas; i++) {
                matrix[var->_constraints[i]->_index].push_back(var->_nome);
            }
        }
    }

    for (i = 0; i < _constraints.size(); i++) {
        first = true;
        for (j = 0; j < (int)matrix[i].size(); j++) {
            if (!first)
                work << " + ";
            first = false;
            work << "x" << matrix[i][j];
            if (((j + 1) % 12) == 0)
                work << endl;
        }
        work << " = 1" << endl;
    }
    //********

    for (i = 0; i < _cuts.size(); i++) {
        first = true;
        rest = _cuts[i];
        ((Constraint*)rest)->ConstraintIterators(it, itFim);
        j = 0;
        for (; it != itFim; it++) {
            var = (*it);
            if (!(var->_fixa || var->_fixaEmZero)) {
                if (!first) 
                    work << " + ";
                first = false;
                work << "x" << var->retNome();
                j++;
                if ((j % 12) == 0)
                    work << endl;
            }
        }
        work << " <= " << std::fixed << std::setprecision(0) << rest->getRHS() << endl;
    }
    work << "binary" << endl;
    j = 0;
    for (i = 0; i < _variables.size(); i++) {
        var = _variables[i];
        if (!(var->_fixa || var->_fixaEmZero)) {
            j++;
            if ((j % 12) == 0)  
                work << endl;
            work << "x" << var->retNome() << " ";
        }
    }
    work << "end" << endl;
    work << flush;
    return work.str();
}


void LagrangeanManager::CleanUp() {
   
    if (ActiveVariables() < 1000) return;
   
    if ( ( (float)_countFixedPartial/(float)_totalVariaveis ) > _config->CLEANFACTOR) {
        //cout << "Limpa Problema: " << ((float)_countFixedPartial / (float)_totalVariaveis ) << " --- " << _config->CLEANFACTOR << endl;
        CleanUpProblem();
        _totalVariaveis -= _countFixedPartial;
        _countFixedPartial = 0;
    }
}

int LagrangeanManager::Audit() {
    int i;
    int res = 0;
    i = ActiveVariables();
    for ( ; i < (int) _variables.size() ; i++ ) {
        if ( ! _variables[i]->_fixaEmZero ) 
        res++;
    }
    return res;
}


void LagrangeanManager::CleanUpProblem() {

    Constraint *constraint;
    Variable *var;
    ConstraintIterator cIt, cItEnd, cItRecicle;
    VariableIterator vIt, vItFim, vItRecicle;
    bool finish;
   
    //Code commented since restriction dont store its variable anymore. Instead variables have pointers to its restrictions.
    /*
    ConstraintsNDBounds(cIt,cItEnd);
    _countConstraintsND = _constraintsND.size();
    while ( cIt != cItEnd ) {
        constraint = *cIt;
        ((Constraint *)constraint)->ConstraintIterators(vIt,vItFim);
        finish = (vIt == vItFim);
        while ( ! finish ) {
            var = (*vIt);
	        if ( var->_fixaEmZero ) {
	            vItRecicle =vIt;
	            vIt++;
	            ((Constraint *)constraint)->RemoveVariable(vItRecicle);
	        }
            else 
	            vIt++;
            finish = (vIt == vItFim);
        }
        cIt++;
    }

    ConstraintsBounds(cIt, cItEnd);
    _countConstraints = _constraints.size();
    while (cIt != cItEnd) {
        constraint = *cIt;
        ((Constraint*)constraint)->ConstraintIterators(vIt, vItFim);
        finish = (vIt == vItFim);
        while (!finish) {
            var = (*vIt);
            if (var->_fixaEmZero) {
                vItRecicle = vIt;
                vIt++;
                ((Constraint*)constraint)->RemoveVariable(vItRecicle);
            }
            else
                vIt++;
            finish = (vIt == vItFim);
        }
        cIt++;
    }   

    */

    // Cuts otherwise store their variables.
    CutsBounds(cIt,cItEnd);
    while ( cIt != cItEnd ) {
        constraint = *cIt;
        ((Constraint *)constraint)->ConstraintIterators(vIt,vItFim);
        finish = (vIt == vItFim);
        if (!finish) vItFim--;
        while ( ! finish ) {
            finish = (vIt == vItFim);
            var = (*vItFim);
            if ( var->_fixaEmZero ) {
                vItRecicle = vItFim;
                if (!finish) vItFim--;
                ((Constraint *)constraint)->RemoveVariable(vItRecicle);
            }
            else if (!finish) vItFim--;
        }
        cIt++;      
    }

    VariableIterator v1, v2;
    finish = _variables.size() == 0;
   
    v1 = _end;
    v2 = _variables.end();
    v2--;
    while ( ! finish ) {
        finish = ( v1 == v2 );
        var = *v2;
        if ( var->_fixaEmZero ) {
            vItRecicle = v2;
            v2--;
            RemoveVariable(vItRecicle);
        }
        else 
            break;
    }
 
    _end = _variables.end();

    CutsBounds(cIt, cItEnd);
    finish = (cIt == cItEnd);
    if (!finish) cItEnd--;

    while ( ! finish ) { 
        finish = (cIt == cItEnd);
        constraint = *cItEnd;
        if ( ((Constraint *)constraint)->Degree() <= 3 ) {
            cItRecicle = cItEnd;
            if (!finish) cItEnd--;
            RemoveCut(cItRecicle);
        }
        else 
            if (!finish) cItEnd--;
    }  

    cout << "CUTS after reduction: " << _cuts.size() << endl;
}

void LagrangeanManager::Restart() {

    setLowerBound(_config->MINUS00);
    ConstraintIterator cIt,cItEnd;  

    ConstraintsBounds(cIt,cItEnd);
    for ( ; cIt != cItEnd; cIt++) {
        (*cIt)->setLagrangean(0);
    }

    CutsBounds(cIt, cItEnd);
    for ( ; cIt != cItEnd; cIt++) {
        (*cIt)->setLagrangean(0);
    }

}

void LagrangeanManager::FinalStats() {

    float LI = getLowerBound();
    float LS = getUpperBound();
    printf("Limite Superior: %3.0f\n",LS);
    printf("Limite Inferior: %3.4f\n",LI);
    printf("Cortes Submetidos: %d\n", _cutsFound);
    printf("Cortes Inseridos:  %d\n", _cutsInserted);
    printf("Cortes Removidos:  %d\n", _cutsRemoved);
    printf("Cortes Finais:     %d\n", static_cast<int>(_cuts.size()));

    if ( (LS-LI) < _config->STOP_GAP) 
        printf("Optimality proved \n");
    else {
        printf("Optimality not proved \n");    

        //string file = DefaultFilePrefix() + ".lp";
        //FILE *saida = fopen(file.c_str(),"w");
        //ImprimeLP(saida);
        //fclose(saida);

    }

}

void LagrangeanManager::StoreIncumbent(Solucao &sol) {

    VariableIterator It;
    
    for (It = _best.begin(); It != _best.end(); It++)
        delete* It;

    _best.erase(_best.begin(), _best.end());

    for (It = sol.begin(); It != sol.end(); It++)
        _best.push_back((*It)->CopyAndClean(NULL));

}


void LagrangeanManager::CheckConstraints(Solucao& sol) {

    ConstraintIterator rest, fim, restLixo;
    VariableIterator vIt, vEnd;
    int i;
    int tamanho = _constraints.size() + _cuts.size();
    vector <bool> Check(tamanho);

    float maxCost = sol[sol.size() - 1]->retCustoLag();

    ConstraintsBounds(rest, fim);
    for (i = 0; rest != fim; rest++, i++) {
        Check[i] = true;
    }

    CutsBounds(rest, fim);
    for (; rest != fim; rest++, i++) {
        Check[i] = true;
        (*rest)->ConstraintIterators(vIt, vEnd);
        for (; vIt != vEnd; vIt++) {
            Check[i] = Check[i] && (*vIt)->retCustoLag() >=  maxCost;
        }
    }
   
    VariableBounds(vIt, vEnd);

    for (; vIt != vEnd; vIt++) {
        if ((*vIt)->retCustoLag() < maxCost) {
            (*vIt)->ConstraintsBounds(rest, fim);
            for (; rest != fim; rest++) {
                Check[(*rest)->_index] = false;
            }
        }
    }

    for (i = 0; i < tamanho; i++) {
        if (! Check[i])
            cout << "Constraint: " << i <<  endl;
        
    }



}