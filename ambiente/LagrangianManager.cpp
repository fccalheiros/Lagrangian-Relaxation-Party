#include "LagrangianManager.h"
#include "algoritmo.h"
#include <sstream>
#include <iomanip>
#include <fstream>

LagrangianManager::LagrangianManager(Configuration* config, Algoritmo * algo, Direction direction, size_t max_sort_depth):
    _algo(algo),
    _direction(direction),
    _countConstraints(0),
    _countConstraintsND(0),
    _cutsInserted(0),
    _cutsFound(0),
    _cutsRemoved(0),
    _countFixed(0),
    _countFixedPartial(0),
    _totalVariaveis(0),
    _config(config),
	_max_sort_depth(max_sort_depth),
	_pool(static_cast<size_t>(1) << _max_sort_depth)
{   
    setLowerBound(_config->MINUS00);
    setUpperBound(_config->PLUS00);
    _variables.reserve(5000);
   
}

LagrangianManager::LagrangianManager(Configuration* config)
    : LagrangianManager(config, nullptr) {
}


LagrangianManager::LagrangianManager(LagrangianManager* m)
    : LagrangianManager(m->_config, m->_algo, m->_direction, m->_max_sort_depth)
{
    _nonZeroCount = m->_nonZeroCount;
    _totalVariaveis = m->_totalVariaveis; // ajuste específico
}


LagrangianManager* LagrangianManager::CopyAndClean(LagrangianManager* m) {
    if (m == NULL) {
        m = new LagrangianManager(this);
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


    GetConstraintRange(rest, restFim);
    for (; rest != restFim; rest++) {
        m->InsertConstraint((*rest)->CopyAndClean(NULL));
    }

    GetNDConstraintRange(rest, restFim);
    for (; rest != restFim; rest++) {
        m->InsertConstraintND((*rest)->CopyAndClean(NULL));
    }

    GetActiveVariablesRange(var, varFim);
    for (; var != varFim; var++) {

        //copy variable
        Variable* v = (*var)->CopyAndClean(NULL);
        m->InsertVariable(v);

        //insert in restrictions
        rest = (*var)->_constraints.begin();
        restFim = (*var)->_constraints.end();
        for (; rest != restFim; rest++)
            v->addConstraint(m->_constraints[(*rest)->_index]);

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
    m->_activeVariablesEnd = m->_variables.end();
    m->_zeroFixedVariablesEnd = m->_variables.end();

    return m;
}

LagrangianManager::~LagrangianManager() {
    FreeMemory();
}

void LagrangianManager::FreeMemory() {

    VariableIterator varIni, varFim, varLixo;
    ConstraintIterator proIni, proFim, proLixo;

    GetActiveVariablesRange(varIni, varFim);
    for (; varIni != varFim;) {
        varLixo = varIni;
        varIni++;
        delete *varLixo;
    }

    GetConstraintRange(proIni, proFim);
    for (; proIni != proFim;) {
        proLixo = proIni;
        proIni++;
        delete *proLixo;
    }

    GetNDConstraintRange(proIni, proFim);
    for (; proIni != proFim;) {
        proLixo = proIni;
        proIni++;
        delete* proLixo;
    }

    GetCutsRange(proIni, proFim);
    for (; proIni != proFim;) {
        proLixo = proIni;
        proIni++;
        delete* proLixo;
    }

    vector <Variable*>().swap(_variables);
    vector<Constraint*>().swap(_constraints);
    vector<Constraint*>().swap(_constraintsND);
    vector<Constraint*>().swap(_cuts);

    _activeVariablesEnd = _variables.end();
    _zeroFixedVariablesEnd = _variables.end();

}

void LagrangianManager::GenerateProblem(char* arq) {
    ReadProblem(arq);
    CreateProblem();
    FinalizeProblemCreation();
    PostProblemCreationPriceOut();
    CommitPriceOut();
    //TestVariableVector();
}

void LagrangianManager::FinalizeProblemCreation() {

    _variables.shrink_to_fit();
    _activeVariablesEnd = _variables.end();
    _zeroFixedVariablesEnd = _variables.end();

    CleanupDeletedConstraints();

    for (int i = 0; i < _countConstraints; i++) {
        _constraints[i]->_index = i;
    }

    VariableIterator vIt, vEnd;
    for (GetActiveVariablesRange(vIt, vEnd); vIt != vEnd; vIt++) {
        (*vIt)->ResetCoveredConstraints();
    }

   
}

void LagrangianManager::ResetLagrangianCosts() {
	ConstraintIterator cIt, cEnd;
    GetConstraintRange(cIt, cEnd);
    for (; cIt != cEnd; cIt++) {
		(*cIt)->setLagrangian(0.0f);
    }
    GetCutsRange(cIt, cEnd);
    for (; cIt != cEnd; cIt++) {
		(*cIt)->setLagrangian(0.0f);
    }
	setDualBound(_config->MINUS00);
}

void LagrangianManager::UpdateBounds(float valRelaxado, float valHeuristica, vector <Variable *> &solHeu, bool resHeuristica){

    float LI = getLowerBound();

    if ( _direction == Direction::MINIMIZE ) {   
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


void LagrangianManager::Solve(float InitialCost, float KnownBound ) { 
  
    bool shouldStop = false;
    bool columnsAdded = false;
    bool primalFound;
    Solucao relaxedSolution;
    Solucao primalSolution;
    float relaxedValue;
    float primalValue;
	float newLowerBound;
      
    _algo->setLagrangianManager(this);  
    _algo->Inicializacao();
    setPrimalBound(KnownBound);

    while ( ! shouldStop ) {
    
        CleanUp();
        CustomProcessing();
        _algo->SolveRelaxation(relaxedSolution, relaxedValue, InitialCost);
        _algo->FixVariables(relaxedSolution, relaxedValue, InitialCost);
        primalFound = _algo->RunPrimalHeuristic(relaxedSolution, primalSolution, primalValue, InitialCost);
        UpdateBounds(relaxedValue, primalValue, primalSolution, primalFound);
        _algo->GenerateCuts(relaxedSolution);
        
        _algo->UpdateSubgradient(relaxedSolution);
        shouldStop = _algo->CheckStopCondition();
        columnsAdded = _algo->ColumnGeneration(relaxedSolution, newLowerBound, InitialCost);
        if (columnsAdded) {
            if (newLowerBound > getLowerBound() /2 ) {
                setDualBound(newLowerBound);
			}
            else {
                ResetLagrangianCosts();
            }
        }
		shouldStop = shouldStop && !columnsAdded;  
    }

    _algo->Finalizacao();
    cout << endl << PrintVariableVector(relaxedSolution) << endl;
    FinalStats();
}

// must be revisited
void LagrangianManager::SetVariableForBranch(Variable* v, short int value) {

    VariableIterator vIt,vBegin, vEnd, vBranch, vRecicle;
    bool sai;

    GetActiveVariablesRange(vBranch, vEnd);

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
        for ( (*vBranch)->GetConstraintRange(cIt, cEnd); cIt != cEnd; cIt++) {
            (*cIt)->LogicalDelete();
		}


        //Remove variables that were in logically deleted constraints 
        //Removal will include vBranch 
        GetActiveVariablesRange(vBegin, vIt);
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
        GetConstraintRange(cBegin, cIt);
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

        GetNDConstraintRange(cBegin, cIt);
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

        _activeVariablesEnd    = _variables.end();

        // It is not clear whether all fixed variables have been removed at this point.
        // If not, they may become part of the active set after branching.
		_zeroFixedVariablesEnd = _variables.end();

    }


}


void LagrangianManager::FixVariable(VariableIterator var) {
    (*var)->FixToZero();
    _activeVariablesEnd--;
    iter_swap(var,_activeVariablesEnd);

    _countFixed++;
    _countFixedPartial++;
}

void LagrangianManager::UnfixVariable(VariableIterator var) {
    auto* v = *var;

    // Do nothing if variable is not fixed
    if (!v->IsFixed())
        return;

    v->UnFix();

    // Move variable into the active region
    auto it = _activeVariablesEnd;
    iter_swap(var, it);

    // Expand active region
    ++_activeVariablesEnd;

    _countFixed--;
    _countFixedPartial--;
}

void LagrangianManager::PriceOutVariable(VariableIterator var) {
    (*var)->setPricedOut();

    // Move variable out of the active region
    _activeVariablesEnd--;
    iter_swap(var, _activeVariablesEnd);

    // After the swap, the target variable is now at _activeVariablesEnd
    auto it = _activeVariablesEnd;

    // Move variable from zero-fixed region into priced-out region
    _zeroFixedVariablesEnd--;
    iter_swap(it, _zeroFixedVariablesEnd);
}

void LagrangianManager::PriceInVariable(VariableIterator var) {
    auto* v = *var;

    // Do nothing if variable is not priced-out
    if (!v->IsPricedOut())
        return;
    v->unsetPricedOut();

    // Step 1: move from priced-out into zero-fixed region
    auto itZF = _zeroFixedVariablesEnd;
    iter_swap(var, itZF);
    ++_zeroFixedVariablesEnd;

    // After swap, variable is now at previous zeroFixed end
    auto it = prev(_zeroFixedVariablesEnd);

    // Step 2: move from zero-fixed into active region
    auto itActive = _activeVariablesEnd;
    iter_swap(it, itActive);
    ++_activeVariablesEnd;
}

void LagrangianManager::MarkVariableForPriceIn(VariableIterator var) {
    (*var)->logicalPriceIn();
}
void LagrangianManager::MarkVariableForPriceOut(VariableIterator var) {
    (*var)->logicalPriceOut();
}

void LagrangianManager::CommitPriceIn() {

    VariableIterator vFirst, vLast, selectedEnd;
    vFirst = _zeroFixedVariablesEnd;
    vLast = _variables.end();

    selectedEnd = stable_partition(
        vFirst,
        vLast,
        [](Variable* v) {
            return v->ShouldBeCommited();
        }
    );
    rotate(
        _activeVariablesEnd,
        vFirst,
        selectedEnd
    );

    auto count = distance(vFirst, selectedEnd);
    cout << "Commited Price In Variables: " << count << endl;
    _activeVariablesEnd += count;
    _zeroFixedVariablesEnd += count;

    for ( GetActiveVariablesRange(vFirst, vLast); vFirst != vLast; vFirst++) {
        (*vFirst)->ClearCommitMark();
	}
}

void LagrangianManager::CommitPriceOut() {

    VariableIterator aBegin = _variables.begin();
    VariableIterator aEnd = _activeVariablesEnd;

    auto remainingEnd = std::stable_partition(
        aBegin,
        aEnd,
        [](Variable* v) {
            return !v->ShouldBeCommited();
        }
    );

    auto count = distance(remainingEnd, aEnd);

    rotate(
        remainingEnd,
        aEnd,
        _variables.end()
    );

    _activeVariablesEnd = remainingEnd;
    _zeroFixedVariablesEnd -= count;


    for (auto it = _zeroFixedVariablesEnd; it != _variables.end(); ++it) {
        (*it)->ClearCommitMark();
    }
}

void LagrangianManager::InsertVariable(Variable *var) {
    size_t i = _variables.size();
    size_t j = _variables.capacity();
    if ( i == j ) {
        i = static_cast<size_t>(i * 1.2);
        _variables.reserve(i);
    }
    _variables.push_back(var);
}

void LagrangianManager::InsertConstraint(Constraint *restricao) {
    _constraints.push_back(restricao);
    _countConstraints++;
}

void LagrangianManager::InsertConstraintND(Constraint *restricao) {
    _constraintsND.push_back(restricao);
    _countConstraintsND++;
}

// Use only with regular constraints 
// Dont use with cuts
void LagrangianManager::MarkConstraintForDeletion(Variable* var) {

    int set = 0;
    ConstraintIterator it = var->_constraints.begin();
    ConstraintIterator end = var->_constraints.end();

    for (; it != end; it++) {
        (*it)->LogicalDelete();
        set++;
    }
    _countConstraints = static_cast<int>(_constraints.size() - set);
    cout << "Restricoes Marcadas: " << set << endl;
}


void LagrangianManager::InsertCut(Constraint *constraint) {
    ConstraintIterator begin;
    ConstraintIterator end;
    _cutsFound++;

    for (GetCutsRange(begin, end); begin != end; begin++) {
        if ( (*begin)->Compare(constraint) )
            return; 
    }

    constraint->SortVariablesByName();
    _cutsInserted++;
    _cuts.push_back(constraint);
}

void LagrangianManager::RemoveVariable(VariableIterator &it) {
      delete *it;
      _variables.erase(it);
}

void LagrangianManager::RemoveConstraint(ConstraintIterator &it) {
    delete *it;
    _constraints.erase(it);
    _countConstraints--;
}

void LagrangianManager::RemoveConstraintND(ConstraintIterator &it) {
    delete *it;
    _constraintsND.erase(it);
    _countConstraintsND--;
}

void LagrangianManager::RemoveCut(ConstraintIterator &it) {
    delete *it;
    _cuts.erase(it);
    _cutsRemoved++;
}

void LagrangianManager::GetActiveVariablesRange(VariableIterator& begin, VariableIterator& end   )
{
    begin = _variables.begin();    
    end = _activeVariablesEnd;
}
void LagrangianManager::GetZeroFixedVariablesRange(VariableIterator& begin, VariableIterator& end) {
    begin = _activeVariablesEnd;
	end = _zeroFixedVariablesEnd;
}

void LagrangianManager::GetPricedOutVariablesRange(VariableIterator& begin, VariableIterator& end) {
    begin = _zeroFixedVariablesEnd;
    end = _variables.end();
}

void LagrangianManager::GetConstraintRange(ConstraintIterator &begin, ConstraintIterator &end){
    begin = _constraints.begin();
    end    = _constraints.end();
}

void LagrangianManager::GetNDConstraintRange(ConstraintIterator &begin, ConstraintIterator &end){
    begin = _constraintsND.begin();
    end    = _constraintsND.end();
}

void LagrangianManager::GetCutsRange(ConstraintIterator &begin, ConstraintIterator &end){
    begin = _cuts.begin();
    end    = _cuts.end();
}

Constraint * LagrangianManager::getConstraint (int i) {
    return _constraints[i];
}

Constraint * LagrangianManager::getConstraintND (int i) {
    return _constraintsND[i];
}

Constraint * LagrangianManager::getCut (int i) {
    return _cuts[i];
}

void LagrangianManager::CoveredConstraints(Variable *var, vector <Constraint *> &linhas) {

    ConstraintIterator rest,fim;

    linhas.erase(linhas.begin(),linhas.end());
    GetConstraintRange(rest,fim); 

    for (; rest != fim; rest++) {
        if ( (*rest)->getCoefficient(var) != 0 ) 
            linhas.push_back( (*rest) );
    }

}


void LagrangianManager::PrintLagrangian() {
    unsigned int i = 0;
    int j;
    bool prim;
    Variable *var;
    j = 0;
    prim = true;
    for (i=0;i< _variables.size();i++) {
        var = _variables[i];
        if ( ! var->IsFixed() ) {
            j++;
            if ( (j % 12) == 0 )  cout << endl;
            if ( ! prim )  cout << " + ";
                prim = false;
            cout << var->_valorLag << " x" << _variables[i]->getName();
        }
    }
}


void LagrangianManager::ImprimeLP(FILE *saida) {
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
        if ( ! var->IsFixed() ) {
            j++;
            if ( (j % 12) == 0 )  
                fprintf(saida,"\n");
            if ( ! prim )  
                fprintf(saida," + ");
            prim = false;
            fprintf(saida,"%1.0f x%d",var->getCost(),var->getName());
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
            if ( ! var->IsFixed() )  {
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
        if ( ! var->IsFixed() ) {
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
	        if ( ! var->IsFixed() )  {
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
        if ( ! var->IsFixed() ) {
            j++;
            if ( (j % 12) == 0 )  fprintf(saida,"\n");
            fprintf(saida,"x%d ",var->getName());
        }
    }
    fprintf(saida,"end\n");
    fflush(saida);
}

string LagrangianManager::PrintVariableVector(Solucao s) {
    stringstream work;

    VariableIterator vIt = s.begin();
    for (; vIt != s.end(); vIt++) {
        work << "x" << (*vIt)->getName() << " : " << (*vIt)->getCost() << " : " << (*vIt)->getLagrangianCost() <<  endl;
    }
    return work.str();
}

void LagrangianManager::PrintLP(string filename) {
    ofstream file;
    file.open(filename);
    file << PrintLP();
    file.close();
}

string LagrangianManager::PrintLP() {
  
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
        if (!var->IsFixed()) {
            j++;
            if ((j % 12) == 0)
                work << endl;
            if (!first)
                work << " + ";
            first = false;
            work << std::fixed << std::setprecision(0) << var->getCost() << " x" << var->getName();
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
            if (!var->IsFixed()) {
                if (!first) 
                    work << " + ";
                first = false;
                work << "x" << var->getName();
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
        if (!var->IsFixed()) {
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
            if (!var->IsFixed()) {
                if (!first) 
                    work << " + ";
                first = false;
                work << "x" << var->getName();
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
        if (!var->IsFixed()) {
            j++;
            if ((j % 12) == 0)  
                work << endl;
            work << "x" << var->getName() << " ";
        }
    }
    work << "end" << endl;
    work << flush;
    return work.str();
}


void LagrangianManager::CleanUp() {
   
    if (getActiveVariablesCount() < 1000) return;
   
    if ( ( (float)_countFixedPartial/(float)_totalVariaveis ) > _config->CLEANFACTOR) {
       CleanUpProblem();
        _totalVariaveis -= _countFixedPartial;
        _countFixedPartial = 0;
    }
}

int LagrangianManager::Audit() {
    int i;
    int res = 0;
    i = getActiveVariablesCount();
    for ( ; i < (int) _variables.size() ; i++ ) {
        if ( ! _variables[i]->IsFixed() ) 
        res++;
    }
    return res;
}

void LagrangianManager::CleanupDeletedConstraints() {
    
    VariableIterator vIt, vEnd;
    ConstraintIterator cIt, cFirst;
	
    // Remove from variables
    for (GetActiveVariablesRange(vIt, vEnd); vIt != vEnd; ++vIt) {

        auto& constraints = (*vIt)->_constraints;
            
        for (auto it = constraints.begin(); it != constraints.end(); ) {
            if ((*it)->LogicalDeleted()) {
                it = constraints.erase(it);
            }
            else {
                ++it;
            }
        }
    }

	//remove from regular constraints
    GetConstraintRange(cFirst, cIt);
    for (; cIt != _constraints.begin(); ) {
		auto current = prev(cIt);
        if ((*current)->LogicalDeleted()) {
            cIt =_constraints.erase(current);
            --_countConstraints;
        }
        else
        {
            cIt = current;
        }
    
    }

}

void LagrangianManager::CleanUpProblem() {

    Constraint *constraint;
    Variable *var;
    ConstraintIterator cIt, cItEnd, cItRecicle;
    VariableIterator vIt, vItFim, vItRecicle;
    bool finish;
   
    //Code commented since restriction dont store its variable anymore. Instead variables have pointers to its restrictions.
    /*
    GetNDConstraintRange(cIt,cItEnd);
    _countConstraintsND = _constraintsND.size();
    while ( cIt != cItEnd ) {
        constraint = *cIt;
        ((Constraint *)constraint)->ConstraintIterators(vIt,vItFim);
        finish = (vIt == vItFim);
        while ( ! finish ) {
            var = (*vIt);
	        if ( var->_isFixedToZero ) {
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

    GetConstraintRange(cIt, cItEnd);
    _countConstraints = _constraints.size();
    while (cIt != cItEnd) {
        constraint = *cIt;
        ((Constraint*)constraint)->ConstraintIterators(vIt, vItFim);
        finish = (vIt == vItFim);
        while (!finish) {
            var = (*vIt);
            if (var->_isFixedToZero) {
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
    GetCutsRange(cIt,cItEnd);
    while ( cIt != cItEnd ) {
        constraint = *cIt;
        ((Constraint *)constraint)->ConstraintIterators(vIt,vItFim);
        finish = (vIt == vItFim);
        if (!finish) vItFim--;
        while ( ! finish ) {
            finish = (vIt == vItFim);
            var = (*vItFim);
            if ( var->IsFixed() ) {
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
   
	GetZeroFixedVariablesRange(v1, v2);
    v2--;
    while ( ! finish ) {
        finish = ( v1 == v2 );
        var = *v2;
        if ( var->IsFixed() ) {
            vItRecicle = v2;
            v2--;
            RemoveVariable(vItRecicle);
        }
        else 
            break;
    }
 
    _activeVariablesEnd = _variables.end();
	_zeroFixedVariablesEnd = _variables.end();

    GetCutsRange(cIt, cItEnd);
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

void LagrangianManager::Restart() {

    setLowerBound(_config->MINUS00);
    ConstraintIterator cIt,cItEnd;  

    GetConstraintRange(cIt,cItEnd);
    for ( ; cIt != cItEnd; cIt++) {
        (*cIt)->setLagrangian(0);
    }

    GetCutsRange(cIt, cItEnd);
    for ( ; cIt != cItEnd; cIt++) {
        (*cIt)->setLagrangian(0);
    }

}

void LagrangianManager::FinalStats() {

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

void LagrangianManager::StoreIncumbent(Solucao &sol) {

    VariableIterator It;
    
    for (It = _best.begin(); It != _best.end(); It++)
        delete* It;

    _best.erase(_best.begin(), _best.end());

    for (It = sol.begin(); It != sol.end(); It++)
        _best.push_back((*It)->CopyAndClean(NULL));

}


void LagrangianManager::CheckConstraints(Solucao& sol) {

    ConstraintIterator rest, fim, restLixo;
    VariableIterator vIt, vEnd;
    size_t i = 0;
    size_t size = _constraints.size() + _cuts.size();
    vector <bool> Check(size);

    float maxCost = sol[sol.size() - 1]->getLagrangianCost();

    for (GetConstraintRange(rest, fim), i = 0; rest != fim; rest++, i++) {
        Check[i] = true;
    }

    for (GetCutsRange(rest, fim); rest != fim; rest++, i++) {
        Check[i] = true;
        for ((*rest)->ConstraintIterators(vIt, vEnd); vIt != vEnd; vIt++) {
            Check[i] = Check[i] && (*vIt)->getLagrangianCost() >=  maxCost;
        }
    }
   
    for (GetActiveVariablesRange(vIt, vEnd) ; vIt != vEnd; vIt++) {
        if ((*vIt)->getLagrangianCost() < maxCost) {
            for ((*vIt)->GetConstraintRange(rest, fim); rest != fim; rest++) {
                Check[(*rest)->_index] = false;
            }
        }
    }

    for (i = 0; i < size; i++) {
        if (! Check[i])
            cout << "Constraint: " << i <<  endl;
        
    }

}
void LagrangianManager::TestVariableVector(){

    VariableIterator vBegin, vIt, vEnd;
    int i = 1;
    for (GetActiveVariablesRange(vBegin, vIt); vIt != vBegin; vIt--) {
        if (i % 7 == 0) {
            FixVariable(vIt);
        }
        else if (i % 13 == 0) {
            PriceOutVariable(vIt);
        }
        i++;
    }

    i = 1;
    for (GetPricedOutVariablesRange(vIt, vEnd); vIt != vEnd; vIt++) {
        if (i % 11 == 0) {
            MarkVariableForPriceIn(vIt);
        }	
        i++;    
    }
    CommitPriceIn();

    i = 1;
    for (GetActiveVariablesRange(vBegin, vIt); vIt != vBegin; vIt--) {
        if (i % 23 == 0) {
            MarkVariableForPriceOut(vIt);
        }
        i++;
    }
    CommitPriceOut();

    int active = 0;
    for (GetActiveVariablesRange(vIt, vEnd); vIt != vEnd; vIt++) {
        if ((*vIt)->IsFixed() || (*vIt)->IsPricedOut()) {
            cout << endl << "Achei uma variável errada na zona de ativas: ";
            cout << (*vIt)->IsFixed() << " - " << (*vIt)->IsPricedOut();
            return;
        }
        active++;
    }

    int fixed = 0;
    for (GetZeroFixedVariablesRange(vIt, vEnd); vIt != vEnd; vIt++) {
        if (!(*vIt)->IsFixed() || (*vIt)->IsPricedOut()) {
            cout << endl << "Achei uma variável errada na zona de fixas" << endl;
            cout << (*vIt)->IsFixed() << " - " << (*vIt)->IsPricedOut() ;
            return;
        }
        fixed++;
    }

    int pricedout = 0;
    for (GetPricedOutVariablesRange(vIt, vEnd); vIt != vEnd; vIt++) {
        if ((*vIt)->IsFixed() || !(*vIt)->IsPricedOut()) {
            cout << endl << "Achei uma variável errada na zona de priced out" << endl;
            cout << (*vIt)->IsFixed() << " - " << (*vIt)->IsPricedOut();
            return;
        }
        pricedout++;
    }
    cout << endl << "vector ok. Active: " << active << " Fixed: " << fixed << " PricedOut: " << pricedout << endl;
}