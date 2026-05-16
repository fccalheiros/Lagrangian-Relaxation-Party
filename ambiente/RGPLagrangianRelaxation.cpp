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
void RGPLagrangianRelaxation::SolveRelaxation(VariableSet& sol, float& valor, float InitialCost) {

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
		_manager->NthElementCustom(4 * Cardinality, CompareLagrangian <Variable*>());
		_ordenou = false;
	}
	else {
		//_manager->Ordena3( CompareLagrangian <Variable*> () );
		_manager->SortActiveVariables(CompareLagrangian <Variable*>(), SortMode::PARALLEL_MERGE);
		_ordenou = true;
	}

	sol.reserve(Cardinality);

	for (i = 0; i < Cardinality; i++) {
		Variable* var = (Variable*)(_manager->_variables[i]);
		sol.push_back(var);
		valor += var->_lagrangianCost;
	}
	std::cout << std::endl;
}

bool RGPLagrangianRelaxation::ColumnGeneration(VariableSet& relaxed, float& newLowerBound, float InitialCost) {

	bool isAnyVariablePricedIn = false;
	const double EPS = 1e-6;
	std::priority_queue<float> heap;
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
				var->_lagrangianCost -= var->_constraints[i]->_lagrangian;
			}

			if (var->_lagrangianCost < bestRC) {
				bestRC = var->_lagrangianCost;
				bestVar = vIt;
			}

			if (var->_lagrangianCost < limit - EPS) {
				std::cout << "x" << var->getName() << " : " << var->getCost() << " -> " << var->_lagrangianCost << std::endl;
				var->logicalPriceIn();
				isAnyVariablePricedIn = true;
			}

			if (var->_lagrangianCost < heap.top()) {
				RelaxedSolutionCost += var->_lagrangianCost - heap.top();
				heap.pop();
				heap.push(var->_lagrangianCost);
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

void RGPLagrangianRelaxation::FixVariables(VariableSet &solRel, float valor, float InitialCost) {
   
	float LI = _manager->getLowerBound();
	float LS = _manager->getUpperBound();
	
	std::cout << _iteracoes << " : " << LS << " --- " << LI 
		<< " --- " << _somaMultiplicadores + InitialCost << " --- " << valor 
		<< " ---> " << _alfa << std::endl;
	std::cout << "Variables : " << _manager->_countFixed << " " 
<< _manager->getActiveVariablesCount() << " " << _manager->_variables.size() << std::endl;


	if ( valor < LI ) 
			return;               
	_vezes1++;
	if ( (LI/LS)  < _config->RATIO_TRY_FIX) 
			return;
	_vezes2++;
	if ( ( LS - valor)  < (float)_config->STOP_GAP ) 
			return;

	std::cout << "** Fixa Variaveis **" << std::endl;

	if (! _ordenou)  {
		_manager->SortActiveVariables(CompareLagrangian <Variable*>(), SortMode::PARALLEL_MERGE);

		_ordenou = true;
	}

	int contaFixadas = 0;


	// disabled
	//at first run a lowcost fixation algorithm
	//float V2 = LS - (valor - _manager->GetMaxLagrangian(solRel)) - _config->STOP_GAP;
	VariableIterator vIt, vDummy;
	_manager->GetActiveVariablesRange(vDummy, vIt);
	size_t InitialTotal = std::distance(vDummy, vIt);
	
	int CardinalityRestriction =  ((RGPManager*)_manager)->getCardinality();  
	VariableIterator var,varFim,varTeste, varFimTeste, varInicioTeste, varTesteAnt;   
	RGPVariable * vTeste;
	float soma;
	int contaVar;
   
	int veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY ,hiX1, hiX2;

	_manager->GetActiveVariablesRange(varTeste,varFimTeste);
	varInicioTeste  = varTeste;
	varTesteAnt     = varFimTeste;
	size_t total    = std::distance(varTeste, varFimTeste);
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
			soma = _somaMultiplicadores + vTeste->_lagrangianCost + InitialCost;
			contaVar = 1;
			vTeste->RetornaSegmentos(veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY ,hiX1, hiX2);
			_manager->GetActiveVariablesRange(var,varFim);
			for (; var != varFim; var++ ) {
				if ( ! ((RGPVariable *)(*var))->Intercepta(veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY ,hiX1, hiX2) ) 
					if ( (*var)->_name != vTeste->_name ) { 
						soma += (*var)->_lagrangianCost;
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
					std::cout << "Dif2: " ;
					std::cout << "x" << vTeste->_name << " ++ " << soma << " ++ " << contaVar << " ++ " << valor << std::endl;
				}
				varTeste++;  	
			}
		} // end for
	} // end while

   if ( contaFixadas > 0 ) {
		std::cout << "Fixadas: " << contaFixadas << std::endl;
		float porc = (float)((float)contaFixadas/(float)InitialTotal); 
		if ( porc > _maximofator ) { 
			_maximofator = porc;
			std::cout << "Maior porcentual " << porc << std::endl;
		}
		else std::cout << "Porcentual " << porc << std::endl;
		if ( _primeira ) { 
			std::cout << "Primeira Fixacao - Fator =  " << LI/LS << std::endl; 
			_primeira = false;
		 }
   }
   std::cout << "Total: " << InitialTotal << std::endl;

}

/******** SolveRelaxation com restricao de area em baixo **********/
// este codigo e inativo e devera ser checado

void RGPLagrangianRelaxation::Relaxacao2(VariableSet& sol, float& valor, float InitialCost) {

	int i = 0;
	int area = ((RGPManager*)_manager)->Area();
	float areaVar;
	float somaArea = 0;
	ComputeLagrangianCosts(valor);
	sol.erase(sol.begin(), sol.end());
	_manager->SortActiveVariables(ComparaArea <Variable*>(),SortMode::PARALLEL_MERGE);

	while (somaArea < area) {
		areaVar = (float)((RGPVariable*)_manager->_variables[i])->Area();
		somaArea += areaVar;
		//printf("%1.4f ",_manager->_variables[i]->getLagrangianCost() / areaVar);
		if (somaArea < area) {
			sol.push_back(_manager->_variables[i]);
			valor += _manager->_variables[i]->_lagrangianCost;
			i++;
		}
		else {
			_fracionario = _manager->_variables[i];
			_valorFracionario = ((float)area - somaArea + areaVar) / areaVar;
			valor += _valorFracionario * _manager->_variables[i]->_lagrangianCost;
		}
	}
	//std::cout << std::endl;
	//std::cout << "TAMANHO :" << sol.size() << std::endl;
}



// Virtual implementation that runs faster than the base class one
// Both must give the same result.
bool RGPLagrangianRelaxation::TemIntercessao(VariableSet& solHeu, Variable* var) {

	int veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY, hiX1, hiX2;
	((RGPVariable*)var)->RetornaSegmentos(veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY, hiX1, hiX2);
	for (unsigned int i = 0; i < solHeu.size(); i++) {
		if (((RGPVariable*)(solHeu[i]))->Intercepta(veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY, hiX1, hiX2))
			return true;
	}
	return false;

}
