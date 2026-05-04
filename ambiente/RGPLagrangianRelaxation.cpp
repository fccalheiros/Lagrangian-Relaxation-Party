#include "RGPLagrangianRelaxation.h"
#include "RGPManager.h"

RGPLagrangianRelaxation::RGPLagrangianRelaxation(Configuration *config) :
  LagrangianRelaxation(config) 
{
  _primeira = true;
  _vezes1 = 0;
  _vezes2 = 0;
  _maximofator = 0;
}

RGPLagrangianRelaxation::~RGPLagrangianRelaxation() 
{
}


// This relaxation considers the cardinality constraint of the RGP problem.
void RGPLagrangianRelaxation::SolveRelaxation(Solucao& sol, float& valor, float InitialCost) {

	int i;

	ComputeLagrangianCosts(valor);
	valor += InitialCost;
	sol.erase(sol.begin(), sol.end());

	bool naoOrdena;
	//naoOrdena = (_iteracoes > 100) && ((_iteracoes % 2) == 0);
	//naoOrdena = ((_iteracoes % 12) != 0);
	//if ( _reStart ) naoOrdena = ( (_iteracoes % 10) != 0 ); 
	
	//felipe - nunca ordena - Faz apenas a estatística de ordem com 4 * o número de variáveis da solução ótima
	naoOrdena = true;

	int Cardinality = ((RGPManager*)_manager)->getCardinality();

	if (naoOrdena) {
		_manager->EstOrdemVariaveis2(Cardinality, CompareLagrangian <Variable*>());
		_ordenou = false;
	}
	else {
		_manager->Ordena3( CompareLagrangian <Variable*> () );
		_ordenou = true;
	}

	sol.reserve(Cardinality);
	cout << valor << " ** ";

	for (i = 0; i < Cardinality; i++) {
		Variable* var = (Variable*)(_manager->_variables[i]);
		sol.push_back(var);
		valor += var->_valorLag;
	}
	cout << valor << endl << endl;
}

bool RGPLagrangianRelaxation::ColumnGeneration(Solucao& relaxed, float& newLowerBound, float InitialCost) {

	bool isAnyVariablePricedIn = false;
	const double EPS = 1e-6;
	priority_queue<float> heap;
	float RelaxedSolutionCost = 0.0;
	float limit = 0.0;
	float bestRC;
	VariableIterator vIt, vLast, bestVar;

	if (PricingTrigger() || _manager->OptimalFound()) {

		for (auto it = relaxed.begin(); it != relaxed.end(); it++) {
			float lagrangianCost = (*it)->getLagrangianCost();
			heap.push(lagrangianCost);
			RelaxedSolutionCost += lagrangianCost;
			if (lagrangianCost > limit) {
				limit = lagrangianCost;
			}
		}


		_manager->GetPricedOutVariablesRange(vIt, vLast);
		bestRC = limit;
		bestVar = vLast;

		for (; vIt != vLast; ++vIt) {

			Variable* var = *vIt;

			var->initializeLagrangianCost();
			for (int i = 0; i < var->_linhasCobertas; i++) {
				var->_valorLag -= var->_constraints[i]->_lagrangian;
			}

			if (var->_valorLag < bestRC) {
				bestRC = var->_valorLag;
				bestVar = vIt;
			}

			if (var->_valorLag < limit - EPS) {
				cout << "x" << var->getName() << " : " << var->getCost() << " -> " << var->_valorLag << endl;
				var->logicalPriceIn();
				isAnyVariablePricedIn = true;
			}

			if (var->_valorLag < heap.top()) {
				RelaxedSolutionCost += var->_valorLag - heap.top();
				heap.pop();
				heap.push(var->_valorLag);
			}
		}

		if (!isAnyVariablePricedIn && _manager->OptimalFound()) {
			if (bestRC < limit && bestVar != vLast) {
				_manager->MarkVariableForPriceIn(bestVar);
				isAnyVariablePricedIn = true;
				RelaxedSolutionCost += bestRC - heap.top();
			}
		}

		if (isAnyVariablePricedIn) {
			_manager->CommitPriceIn();
			newLowerBound = RelaxedSolutionCost + InitialCost + _somaMultiplicadores;
			return true;
		}
	}
	return false;
}

void RGPLagrangianRelaxation::FixVariables(Solucao &solRel, float valor, float InitialCost) {
   
	float LI = _manager->getLowerBound();
	float LS = _manager->getUpperBound();
	
	cout << _iteracoes << " : " << LS << " --- " << LI << " --- " << valor << " ---> " << _alfa << endl;
	cout << "Variables : " << _manager->_countFixed << " " << _manager->getActiveVariablesCount() << " " << _manager->_variables.size() << endl;


	if ( valor < LI ) 
			return;               
	_vezes1++;
	if ( (LI/LS)  < _config->RATIO_TRY_FIX) 
			return;
	_vezes2++;
	if ( ( LS - valor)  < (float)_config->STOP_GAP ) 
			return;

	cout << "** Fixa Variaveis **" << endl;

	if (! _ordenou)  {
		_manager->Ordena3( CompareLagrangian <Variable *> () );
		_ordenou = true;
	}

	int contaFixadas = 0;


	// disabled
	//at first run a lowcost fixation algorithm
	//float V2 = LS - (valor - _manager->GetMaxLagrangian(solRel)) - _config->STOP_GAP;
	VariableIterator vIt, vDummy;
	_manager->GetActiveVariablesRange(vDummy, vIt);
	size_t InitialTotal = distance(vDummy, vIt);
	
	int CardinalityRestriction =  ((RGPManager*)_manager)->getCardinality();  
	VariableIterator var,varFim,varTeste, varFimTeste, varInicioTeste, varTesteAnt;   
	RGPVariable * vTeste;
	float soma;
	int contaVar;
   
	int veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY ,hiX1, hiX2;

	_manager->GetActiveVariablesRange(varTeste,varFimTeste);
	varInicioTeste  = varTeste;
	varTesteAnt     = varFimTeste;
	size_t total    = distance(varTeste, varFimTeste);
	float passo     = _config->STEP_TRY_FIX;
	float acumulado = passo;
	bool fixou      = true;
   
	while ( fixou && (acumulado <= _config->MAX_TRY_FIX) ) {
		fixou       = false;
		varTeste = varInicioTeste + static_cast<size_t>(total * (1 - acumulado));
		varFimTeste = varTesteAnt; 
		varTesteAnt = varTeste;
		acumulado  += passo;
		for ( ; varTeste != varFimTeste; ) {

			vTeste = (RGPVariable *) *varTeste;
			if ( vTeste->IsFixed() ) break; 
			soma = _somaMultiplicadores + vTeste->_valorLag + InitialCost;
			contaVar = 1;
			vTeste->RetornaSegmentos(veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY ,hiX1, hiX2);
			_manager->GetActiveVariablesRange(var,varFim);
			for (; var != varFim; var++ ) {
				if ( ! ((RGPVariable *)(*var))->Intercepta(veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY ,hiX1, hiX2) ) 
					if ( (*var)->_nome != vTeste->_nome ) { 
						soma += (*var)->_valorLag;
						contaVar++;
						if ( contaVar == CardinalityRestriction ) break;
						if ( soma > LS - _config->STOP_GAP ) break;
					}
			}
			if ( soma > LS - _config->STOP_GAP) {
				_manager->FixVariable(varTeste);
				contaFixadas++;
				fixou = true; 
			}
			else {
				if (contaVar != CardinalityRestriction) {
					cout << "Dif2: " ;
					cout << "x" << vTeste->_nome << " ++ " << soma << " ++ " << contaVar << " ++ " << valor << endl;
				}
				varTeste++;  	
			}
		} // end for
	} // end while

   if ( contaFixadas > 0 ) {
		cout << "Fixadas: " << contaFixadas << endl;
		float porc = (float)((float)contaFixadas/(float)InitialTotal); 
		if ( porc > _maximofator ) { 
			_maximofator = porc;
			cout << "Maior porcentual " << porc << endl;
		}
		else cout << "Porcentual " << porc << endl;
		if ( _primeira ) { 
			cout << "Primeira Fixacao - Fator =  " << LI/LS << endl; 
			_primeira = false;
		 }
   }
   cout << "Total: " << InitialTotal << endl;
  
   //int cont = _manager->Audit();
   //if ( cont > 0  ) 
   //  cout << "Falhou a auditoria... " <<  cont << endl;

}

/******** SolveRelaxation com restricao de area em baixo **********/
// este codigo e inativo e devera ser checado

void RGPLagrangianRelaxation::Relaxacao2(Solucao& sol, float& valor, float InitialCost) {

	int i = 0;
	int area = ((RGPManager*)_manager)->Area();
	float areaVar;
	float somaArea = 0;
	ComputeLagrangianCosts(valor);
	sol.erase(sol.begin(), sol.end());
	_manager->Ordena(ComparaArea <Variable*>());

	while (somaArea < area) {
		areaVar = (float)((RGPVariable*)_manager->_variables[i])->Area();
		somaArea += areaVar;
		//printf("%1.4f ",_manager->_variables[i]->getLagrangianCost() / areaVar);
		if (somaArea < area) {
			sol.push_back(_manager->_variables[i]);
			valor += _manager->_variables[i]->_valorLag;
			i++;
		}
		else {
			_fracionario = _manager->_variables[i];
			_valorFracionario = ((float)area - somaArea + areaVar) / areaVar;
			valor += _valorFracionario * _manager->_variables[i]->_valorLag;
		}
	}
	//cout << endl;
	//cout << "TAMANHO :" << sol.size() << endl;
}



// Virtual implementation that runs faster than the base class one
// Both must give the same result.
bool RGPLagrangianRelaxation::TemIntercessao(Solucao& solHeu, Variable* var) {

	int veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY, hiX1, hiX2;
	((RGPVariable*)var)->RetornaSegmentos(veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY, hiX1, hiX2);
	for (unsigned int i = 0; i < solHeu.size(); i++) {
		if (((RGPVariable*)(solHeu[i]))->Intercepta(veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY, hiX1, hiX2))
			return true;
	}
	return false;

}