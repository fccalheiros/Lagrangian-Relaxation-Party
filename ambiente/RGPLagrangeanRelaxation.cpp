#include "RGPLagrangeanRelaxation.h"


RGPLagrangeanRelaxation::RGPLagrangeanRelaxation(Configuration *config) :
  LagrangeanRelaxation(config) 
{
  _primeira = true;
  _vezes1 = 0;
  _vezes2 = 0;
  _maximofator = 0;
}

RGPLagrangeanRelaxation::~RGPLagrangeanRelaxation() 
{
}


// This relaxation considers the cardinality constraint of the RGP problem.
void RGPLagrangeanRelaxation::Relaxacao(Solucao& sol, float& valor, float InitialCost) {

	int i;

	InicializaRelaxacao(valor);
	valor += InitialCost;
	sol.erase(sol.begin(), sol.end());

	bool naoOrdena;
	//naoOrdena = (_iteracoes > 100) && ((_iteracoes % 2) == 0);
	//naoOrdena = ((_iteracoes % 12) != 0);
	//if ( _reStart ) naoOrdena = ( (_iteracoes % 10) != 0 ); 
	
	//felipe - nunca ordena - Faz apenas a estatística de ordem com 4 * o número de variáveis da solução ótima
	naoOrdena = true;

	int Cardinality = ((RGPManager*)_manager)->_numeroPontos + 1;

	if (naoOrdena) {
		_manager->EstOrdemVariaveis2(Cardinality, CompareLagrangean <Variable*>());
		_ordenou = false;
	}
	else {
		_manager->Ordena3(CompareLagrangean <Variable*>());
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


// felipe - versão adaptada para usar com o pricing beta - nesse procedimento é necessário verificar se a variável está priced out.
/*
void RGPLagrangeanRelaxation::Relaxacao(Solucao& sol, float& valor, float InitialCost) {

	int i,j ;

	InicializaRelaxacao(valor);
	valor += InitialCost;
	sol.erase(sol.begin(), sol.end());

	bool naoOrdena;
	//naoOrdena = (_iteracoes > 100) && ((_iteracoes % 2) == 0);
	//naoOrdena = ((_iteracoes % 12) != 0);
	//if ( _reStart ) naoOrdena = ( (_iteracoes % 10) != 0 ); 

	//felipe - retirar 
	naoOrdena = false;

	int Cardinality = ((RGPManager*)_manager)->_numeroPontos + 1;

	if (naoOrdena) {
		_manager->EstOrdemVariaveis2(Cardinality, CompareLagrangean <Variable*>());
		_ordenou = false;
	}
	else {
		_manager->Ordena2(CompareLagrangean <Variable*>());
		_ordenou = true;
	}

	sol.reserve(Cardinality);
	cout << valor << " ** ";

	i = 0;
	j = 0;
	while (i < Cardinality) {
		while (((RGPVariable*)(_manager->_variables[j]))->_out) {
			j++;
		}
		sol.push_back(_manager->_variables[j]);
		valor += _manager->_variables[j]->_valorLag;
		i++;
		j++;
	}

	cout << valor << " ** " << j << endl << endl;
}
*/

void RGPLagrangeanRelaxation::FixaVariaveis(Solucao &solRel, float valor, float InitialCost) {
   
	float LI = _manager->getLowerBound();
	float LS = _manager->getUpperBound();
	
	cout << _iteracoes << " : " << LS << " --- " << LI << " --- " << valor << " ---> " << _alfa << endl;
	cout << "Variables : " << _manager->_countFixed << " " << _manager->ActiveVariables() << " " << _manager->_variables.size() << endl;


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
		_manager->Ordena3( CompareLagrangean <Variable *> () );
		_ordenou = true;
	}

	int contaFixadas = 0;


	// disabled
	//at first run a lowcost fixation algorithm
	//float V2 = LS - (valor - _manager->GetMaxLagrangean(solRel)) - _config->STOP_GAP;
	VariableIterator vIt, vDummy;
	_manager->VariableBounds(vDummy, vIt);
	int InitialTotal = distance(vDummy, vIt);

	/*
	vIt--;
	while ((*vIt)->_valorLag > V2) {
		vIt--;
		_manager->FixLastVariable();
		contaFixadas++;
	}
	*/
	
	int numVar =  ((RGPManager*)_manager)->_numeroPontos + 1;  
	VariableIterator var,varFim,varTeste, varFimTeste, varInicioTeste, varTesteAnt;   
	RGPVariable * vTeste;
	float soma;
	int contaVar;
   
	int veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY ,hiX1, hiX2;

	_manager->VariableBounds(varTeste,varFimTeste);
	varInicioTeste  = varTeste;
	varTesteAnt     = varFimTeste;
	int total       = distance(varTeste, varFimTeste);
	float passo     = _config->STEP_TRY_FIX;
	float acumulado = passo;
	bool fixou      = true;
   
	while ( fixou && (acumulado <= _config->MAX_TRY_FIX) ) {
		fixou       = false;
		varTeste    = varInicioTeste + (int) ( total * (1-acumulado) );   
		varFimTeste = varTesteAnt; 
		varTesteAnt = varTeste;
		acumulado  += passo;
		for ( ; varTeste != varFimTeste; ) {

			vTeste = (RGPVariable *) *varTeste;
			if ( vTeste->_fixaEmZero || vTeste->_fixa ) break; 
			soma = _somaMultiplicadores + vTeste->_valorLag + InitialCost;
			contaVar = 1;
			vTeste->RetornaSegmentos(veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY ,hiX1, hiX2);
			_manager->VariableBounds(var,varFim);
			for (; var != varFim; var++ ) {
				if ( ! ((RGPVariable *)(*var))->Intercepta(veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY ,hiX1, hiX2) ) 
					if ( (*var)->_nome != vTeste->_nome ) { 
						soma += (*var)->_valorLag;
						contaVar++;
						if ( contaVar == numVar ) break;
						if ( soma > LS - _config->STOP_GAP ) break;
					}
			}
			//cout << "Indice fixa: " << std::distance(_manager->_variables.begin(), var) << " Totais:" << std::distance(_manager->_variables.begin(), _manager->_end) <<endl;
			// Fixando a variavel
			if ( soma > LS - _config->STOP_GAP) {
				_manager->FixVariable(varTeste);
				contaFixadas++;
				fixou = true; 
			}
			else {
				if (contaVar != numVar) {
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

/******** Relaxacao com restricao de area em baixo **********/
// este codigo e inativo e devera ser checado

void RGPLagrangeanRelaxation::Relaxacao2(Solucao& sol, float& valor, float InitialCost) {

	int i = 0;
	int area = ((RGPManager*)_manager)->Area();
	float areaVar;
	float somaArea = 0;
	InicializaRelaxacao(valor);
	sol.erase(sol.begin(), sol.end());
	_manager->Ordena(ComparaArea <Variable*>());

	while (somaArea < area) {
		areaVar = (float)((RGPVariable*)_manager->_variables[i])->Area();
		somaArea += areaVar;
		//printf("%1.4f ",_manager->_variables[i]->retCustoLag() / areaVar);
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
bool RGPLagrangeanRelaxation::TemIntercessao(Solucao& solHeu, Variable* var) {

	int veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY, hiX1, hiX2;
	((RGPVariable*)var)->RetornaSegmentos(veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY, hiX1, hiX2);
	for (unsigned int i = 0; i < solHeu.size(); i++) {
		if (((RGPVariable*)(solHeu[i]))->Intercepta(veX, veY1, veY2, vdX, vdY1, vdY2, hsY, hsX1, hsX2, hiY, hiX1, hiX2))
			return true;
	}
	return false;

}