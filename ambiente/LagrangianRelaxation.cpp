#include <stdio.h>
#include <algorithm>

#include "LagrangianRelaxation.h"
#include "Constraint.h"
#include "grafo.h"


/************ Comeco da LagrangianRelaxation *********************/

LagrangianRelaxation::LagrangianRelaxation(Configuration *config):
  Algoritmo(config)
{
    _alfa = (float)config->INI_ALFA;
    _lambda = config->LAMBDA;
    _naoMudouLI = 0;
    _ultimoLI = config->MINUS00;
    _ultimoLIHeuristica = _ultimoLI;
    _fracionario = 0;
    _reStart = false;
    _somaMultiplicadores = 0;
    _ordenou = false;
    _valorFracionario = 0;
}


LagrangianRelaxation::~LagrangianRelaxation() 
{
}

void LagrangianRelaxation::setLagrangianManager(LagrangianManager *mes) {
    Algoritmo::setLagrangianManager(mes);
}

void LagrangianRelaxation::Inicializacao() {
  Algoritmo::Inicializacao();
}



/*************** SolveRelaxation ******************/



void LagrangianRelaxation::ComputeLagrangianCosts(float & multiplierSum) {

    ConstraintIterator pComeco, pFim;
    VariableIterator   rComeco, rFim;
    VariableIterator   vComeco, vFim;

  /*
  // Inicializa o custo lagrangiano de cada variavel com seu custo na FO
  for ( _manager->GetActiveVariablesRange(vComeco, vFim); vComeco != vFim; vComeco++ ) 
     (*vComeco)->initializeLagrangianCost();

  soma = 0;  // Soma dos multiplicadores lagrangianos

  
  // Calcula os novos custo lagrangianos de cada variavel
  // Percorre todas as restricoes, e para cada uma percorre todas suas variaveis
  for ( _manager->GetConstraintRange(pComeco, pFim) ; pComeco != pFim ; pComeco++) {
    Constraint *rest = (Constraint *) (*pComeco);
    float multiplicador = rest->getLagrangian();
    if ( ! rest->_fixa ) {
       soma += multiplicador * rest->getRHS();
       for ( rest->ConstraintIterators(rComeco, rFim); rComeco != rFim; rComeco++) {
	  Variable *var = (*rComeco);
	  //float coef = 1.0;
	  //var->_valorLag -= coef*multiplicador;
	  var->_valorLag -= multiplicador;
       }
    }
  }
  */

    multiplierSum = 0;  // Soma dos multiplicadores lagrangianos
    for ( _manager->GetConstraintRange(pComeco, pFim) ; pComeco != pFim ; pComeco++)  
        multiplierSum += (*pComeco)->_lagrangian * (*pComeco)->getRHS();

	//Compute reduced costs of active variables
    ComputeReducedCosts(true);

	//Add reduced costs related to cuts valids for the current iteration
    for ( _manager->GetCutsRange(pComeco, pFim) ; pComeco != pFim ; pComeco++) {  
        Constraint *rest = (Constraint *) (*pComeco);
        float multiplicador = rest->getLagrangian();
        multiplierSum += multiplicador * rest->getRHS();
        for ( rest->ConstraintIterators(rComeco, rFim); rComeco != rFim; rComeco++) {  
	        ((Variable *)(*rComeco))->_valorLag -= multiplicador;        //Está assumindo que o coeficiente da restrição é 1
        }
    }

    _somaMultiplicadores = multiplierSum;
}

// generic relaxation without any non dualized constraint
void LagrangianRelaxation::SolveRelaxation(Solucao& sol, float& valor, float InitialCost) {

    ComputeLagrangianCosts(valor);
    valor += InitialCost;
    sol.erase(sol.begin(), sol.end());

    bool naoOrdena = (_iteracoes > 100) && ((_iteracoes % 2) == 0);

    cout << valor << " ** ";

    VariableIterator vIt, vEnd;
    _manager->GetActiveVariablesRange(vIt, vEnd);

    if (naoOrdena) {
        _ordenou = false;
        while (vIt != vEnd) {
            if ((*vIt)->_valorLag <= 0) {
                sol.push_back(*vIt);
                valor += (*vIt)->_valorLag;
            }
        }
    }
    else {
        _manager->Ordena2(CompareLagrangian <Variable*> ());
        _ordenou = true;
        while ((*vIt)->_valorLag <= 0) {
            sol.push_back(*vIt);
            valor += (*vIt)->_valorLag;
        }
        
    }

    cout << valor << endl << endl;
}



void LagrangianRelaxation::UpdateSubgradient(Solucao &sol){ 
    int i = 0;
    int var;
    ConstraintIterator rest,fim, restLixo;
    float LI = _manager->getLowerBound();
    float LS = _manager->getUpperBound();
    float tw;
    int solSize = static_cast<int>(sol.size());

    int tamanho = static_cast<int>(_manager->_constraints.size() + _manager->_cuts.size());
    cout << "CUTS: " << _manager->_cuts.size() << endl; 
    vector <float> Gw(tamanho);
    float moduloGw = 0;

    _iteracoes++;

    _manager->GetConstraintRange(rest,fim);
    for (i=0; rest != fim; rest++ ) {
        Gw[i] = (*rest)->getRHS();
        i++;
    }
    for (var=0 ; var < solSize; var++) {
        for ( i=0; i < sol[var]->_linhasCobertas; i++) 
            Gw[ (sol[var]->_constraints[i])->_index ] -= 1; // Está cravado em 1 supondo que os indices das variáveis sempre são 1
    }
    _manager->GetConstraintRange(rest,fim);
    for (i=0; rest != fim; rest++ ) {
        moduloGw += Gw[i] * Gw[i]; 
        i++;
    }

    // Trocar o getCoefficient pelo retintercessao , ver se o corte esta ordenado
    _manager->GetCutsRange(rest,fim);
    for (; rest != fim; rest++ ){
        Gw[i] = (*rest)->getRHS();
        for (var=0; var < solSize; var++)
            Gw[i] -= (*rest)->getCoefficient(sol[var]);
        if ( _fracionario != 0 ) 
            Gw[i] -= (*rest)->getCoefficient(_fracionario) * _valorFracionario;
        moduloGw += Gw[i] * Gw[i];
        i++;
    }

   /**** Testes para orientar o passo do subgradiente ****/

    if ( _ultimoLI == LI ) 
        _naoMudouLI++;
    else {
        _naoMudouLI = 0;
        _ultimoLI = LI;
    }
  
    if ( _naoMudouLI == _config->ITERATIONS_CHANGE_ALFA ) {   
        _alfa = _lambda * _alfa;
        _naoMudouLI = 0;
    }

  
    float razao = LI/LS;
    if ( (razao > 0.8) || (razao < -0.8) )
        tw = _alfa*( ( (float)1.02 * LS) - LI );
    else {
        tw = _alfa * LI * (float) 0.2;
        if ( tw < 0 ) tw = -1*tw;
        if (tw == 0) tw = 10;
    }
    /**** Testes para orientar o passo do subgradiente ****/

    i = 0;
    _manager->GetConstraintRange(rest,fim);
    for (; rest != fim; rest++ ){
        float ml = (*rest)->_lagrangian;
        (*rest)->setLagrangian( ml + tw*Gw[i]/moduloGw );
        i++;
    }  
  
    _manager->GetCutsRange(rest,fim);
    bool sai = (fim == rest);
    if ( !sai ) fim--;
    i = tamanho - 1;
    while ( ! sai ) {
        sai = ( fim == rest );
        float ml = (*fim)->_lagrangian;
        (*fim)->setLagrangian( ml + tw*Gw[i]/moduloGw);
        i--;
        if ( (*fim)->_lagrangian == 0 ) {
            restLixo = fim;
            if (!sai) fim --;
            _manager->RemoveCut(restLixo);
        }
        else if ( !sai ) fim--;
    }

}

/********************************************************/
/* Comeco da heuristica para obtencao de solucao viavel */
/********************************************************/

bool LagrangianRelaxation::TemIntercessao(Solucao &solHeu, Variable *var) {

    for (unsigned int i=0; i < solHeu.size(); i++) {

        if ( var->Intercepta( solHeu[i] ) ) 
            return true;
    }
    return false;

}

void LagrangianRelaxation::InicializacoesHeuristica() {

    ConstraintIterator rest, restFim;
    _manager->GetNDConstraintRange(rest, restFim);
    for (; rest != restFim; rest++)
        (*rest)->Uncover();

}

bool LagrangianRelaxation::RunPrimalHeuristic(Solucao &solRel, Solucao &solHeu, float &valor, float InitialCost) {
  
    //return false;
 
    float LI = _manager->getLowerBound();
    float LS = _manager->getUpperBound();
    _ultimoLIHeuristica = LI;

    Variable *var;
    bool particao = false;
    bool impossivel = false;
    int descobertas = _manager->_countConstraints + _manager->_countConstraintsND; 

    solHeu.erase(solHeu.begin(),solHeu.end());

    InicializacoesHeuristica();
    valor = InitialCost;

    VariableIterator begin, It, end;
    _manager->GetActiveVariablesRange(It,end);
    while ( ( !particao ) && ( !impossivel ) ) {
        if (valor > LS) return particao;
        var = *It;
        if ( var->IsValid() ) {
            if ( ! TemIntercessao(solHeu,var) ) {
	            solHeu.push_back(var);
	            valor += var->getCost();
	            descobertas -= var->_linhasCobertas;
	        }	
        }

        It++; 
        particao = (descobertas == 0);
        impossivel = (It == end) && ! particao;
    }

    _manager->GetActiveVariablesRange(begin, end);

    if (particao) {
        cout << "Primal Solution Found: " << valor << " --- " << distance(begin,It) << endl;
    }
    else
        cout << "Primal Solution Fail:  " << descobertas << " Cost: " << valor << " --- " << distance(begin, It) << endl;
  
    return particao;

}


void LagrangianRelaxation::GenerateCuts(Solucao &solRel) {

    if (!_config->CUT_GENERATION) return;

    int i,j;
    int tamanho = (int) solRel.size();
    if (tamanho == 0)
        return;

    Grafo g(tamanho);

    for (i=0; i< tamanho ; i++)
        g.InsereNo(solRel[i]);

    //g.ImprimeNos();
    for (i=0; i < tamanho - 1 ; i++ ) {
        for (j=i+1; j < tamanho ; j++)
            if ( solRel[i]->Intercepta(solRel[j]) ) {
                g.InsereAresta(solRel[i],solRel[j]);
                //cout << "Interceptou: " << solRel[i]->getName() << " - " << solRel[j]->getName() << endl;
            }
    }
    //g.Imprime();
    g.Clique(_manager);
    g.CicloImpar(_manager);
}

bool LagrangianRelaxation::Price(Solucao& relaxed) {

    if (! _manager->OptimalFound()) return false;

    VariableIterator vIt = relaxed.begin();
    VariableIterator vEnd = relaxed.end();
    float max = _config->MINUS00;

    for (; vIt != vEnd; vIt++) {
        if ((*vIt)->_valorLag > max)
            max = (*vIt)->_valorLag;
    }

    _manager->GetActiveVariablesRange(vIt, vEnd);

    int count = 0;
    for (; vIt != vEnd; vIt++) {
        if ((*vIt)->getLagrangianCost() < max && (*vIt)->IsPricedOut()) {
            cout << "x" << (*vIt)->getName() << " : " << (*vIt)->getLagrangianCost() << endl;
            _manager->PriceInVariable(vIt);
            count++;
        }
    }
    cout << "Pricing Variables Included: " << count << endl;
    return true;
 }

// implementing here.... decide later if have to move to son class of LagrangianRelaxation

void LagrangianRelaxation::ComputeReducedCosts(bool onlyActiveVariables) {

	VariableIterator vFirst, vLast, vDummy;
    
    if (onlyActiveVariables) {
        _manager->GetActiveVariablesRange(vFirst, vLast);
        for (; vFirst != vLast;) {
            Variable* var = (*vFirst);
            var->initializeLagrangianCost();
            for (int i = 0; i < var->_linhasCobertas; i++) {
                var->_valorLag -= var->_constraints[i]->_lagrangian;   //Está assumindo que o coeficiente da restrição é 1
            }
            vFirst++;
        }
    }
    else {
        //assume the two set are contiguous.
        _manager->GetZeroFixedVariablesRange(vFirst, vDummy);
        _manager->GetPricedOutVariablesRange(vDummy, vLast);
        for (; vFirst != vLast;) {
            Variable* var = (*vFirst);
            var->initializeLagrangianCost();
            for (int i = 0; i < var->_linhasCobertas; i++) {
                var->_valorLag -= var->_constraints[i]->_lagrangian;   //Está assumindo que o coeficiente da restrição é 1
            }
            vFirst++;
        }
	}
};

bool LagrangianRelaxation::PricingTrigger() {

	return (_iteracoes % 200 == 0); // only to test, decide later the best trigger for pricing
}

bool LagrangianRelaxation::ColumnGeneration(Solucao& relaxed) {
	bool isAnyVariablePricedIn = false;
    const double EPS = 1e-6;
    VariableIterator bestVar;
    double bestRC = 0.0;

    if (PricingTrigger() || _manager->OptimalFound()  ) {
		// Compute reduced costs for remaining variables. Assuming that active variables are already updated in the SolveRelaxation step.
		ComputeReducedCosts(false);
		
        VariableIterator vFirst, vLast;
        _manager->GetPricedOutVariablesRange(vFirst, vLast);
		if (distance(vFirst, vLast) == 0) return false;
        for ( ; vFirst != vLast; vFirst++) {
            double rc = (*vFirst)->getLagrangianCost();
            // best column
            if (rc < bestRC) {
                bestRC = rc;
                bestVar = vFirst;
            }
            if (rc < -EPS) {
				_manager->MarkVariableForPriceIn(vFirst);
				isAnyVariablePricedIn = true;
            }
		}   
        if (!isAnyVariablePricedIn && _manager->OptimalFound()) {
            if (bestRC < 0) {
                _manager->PriceInVariable(bestVar);
                return true;
            }
        }
        if (isAnyVariablePricedIn) {
            _manager->CommitPriceIn();
            return true;
        }
	}
    return false;
}

/************* Teste de parada do algoritmo lagrangiano ************/
bool LagrangianRelaxation::CheckStopCondition() { 

    //if  ( ((_manager->getUpperBound() - _manager->getLowerBound()) < (float) _config->STOP_GAP) ||
     if ( (_manager->OptimalFound()) || (_iteracoes > _config->MAX_ITERATIONS) ) {
        return true; 
    }

    else if (_config->RESTART) {
        if ((_iteracoes % _config->RESTART_ITERATIONS) == 0)
            Restart();
    }

    return false; 
};

void LagrangianRelaxation::Restart() {
    _alfa = (float)_config->INI_ALFA;
    _lambda = _config->LAMBDA;
    _naoMudouLI = 0;
    _ultimoLI = _config->MINUS00;
    _ultimoLIHeuristica = _ultimoLI;
    _fracionario = 0;   
    _reStart = true;
    _manager->Restart();
}


Variable* LagrangianRelaxation::ChooseBranchVariableLowLagrangian() {

    VariableIterator vIt, vEnd, vSelected;
    float MIN = _config->PLUS00;

    _manager->GetActiveVariablesRange(vIt, vEnd);

    for (; vIt != vEnd; vIt++) {
        if ((*vIt)->_valorLag < MIN) {
            MIN = (*vIt)->_valorLag;
            vSelected = vIt;
        }
    }
    return (*vSelected)->CopyAndClean(NULL);

}
Variable* LagrangianRelaxation::ChooseBranchVariableHighIncumbentCost() {

    VariableIterator vIt, vEnd, vSelected;
    VariableIterator vItAll, vEndAll;

    if (_manager->_best.size() > 0) {

        sort(_manager->_best.begin(), _manager->_best.end(), GreaterCost <Variable*>());

        VariableIterator vIt, vEnd;
        vIt = _manager->_best.begin();
        vEnd = _manager->_best.end();

        for (; vIt != vEnd; vIt++) {
            _manager->GetActiveVariablesRange(vItAll, vEndAll);
            for (; vItAll != vEndAll; vItAll++) {
                if ((*vIt)->_nome == (*vItAll)->_nome)
                    return (*vIt)->CopyAndClean(NULL);
            }
        }

    }

    return (*(_manager->_variables.begin()))->CopyAndClean(NULL);
}


 Variable* LagrangianRelaxation::ChooseBranchVariable() {

     if (_config->getValue("VARIABLESTRATEGY").compare("LOWLAGRANGIAN") == 0)
         return ChooseBranchVariableLowLagrangian();

     return ChooseBranchVariableHighIncumbentCost();


 }