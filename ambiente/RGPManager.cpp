#include "RGPManager.h"
#include "Time.h"
#include "RGPLagrangianRelaxation.h"

#include <stdio.h>

using namespace std;

RGPManager::RGPManager(Configuration* config)
    : RGPManager(config, nullptr, Direction::MINIMIZE) 
{
}

RGPManager::RGPManager(Configuration* config, Algoritmo * algo, Direction direction):
  LagrangianManager(config, algo,direction),
    _colunas(0),
    _colunas2(0),
    _colunas3(0),
    _colunas4(0),
    _JaImprimiu(true),
    _numeroPontos(0),
    _faixa(0)
{
}

RGPManager::RGPManager(RGPManager* m) :
    LagrangianManager( (LagrangianManager *) m),
    _colunas(m->_colunas),
    _colunas2(m->_colunas2),
    _colunas3(m->_colunas3),
    _colunas4(m->_colunas4),
    _JaImprimiu(m->_JaImprimiu),
    _numeroPontos(m->_numeroPontos),
    _faixa(m->_faixa)
{

}

LagrangianManager* RGPManager::CopyAndClean(LagrangianManager* m) {

    RGPManager* m1;
    if (m == NULL) {
        m = new RGPManager(this);
        m1 = (RGPManager*)m;
    }
    else {
         m1 = (RGPManager*)m;
         m1->_colunas      = _colunas;
         m1->_colunas2     = _colunas2;
         m1->_colunas3     = _colunas3;
         m1->_colunas4     = _colunas4;
         m1->_JaImprimiu   = _JaImprimiu;
         m1->_nonZeroCount          = _nonZeroCount;
         m1->_numeroPontos = _numeroPontos;
         m1->_faixa        = _faixa;
    }

    m1->_instancia = _instancia;

    return LagrangianManager::CopyAndClean(m);
}

void RGPManager::SetVariableForBranch(Variable* v, short int value) {
    LagrangianManager::SetVariableForBranch(v, value);
    if (value == 1)
        _numeroPontos--;
}

RGPManager::~RGPManager() { }

void RGPManager::ReadProblem(char *arq) {
    int pt;
    int x,y;

    FILE *fp = fopen(arq,"r");
    if ( fp == 0 ) {
        fprintf(stderr,"File not found: %s\n",arq);
        exit(1);
    }
  
    std::ignore = fscanf(fp,"%d\n",&_numeroPontos);
    std::ignore = fscanf(fp,"%d\n",&_faixa);

    for (pt=0; pt < _numeroPontos; pt++) {
        std::ignore = fscanf(fp,"%d %d\n",&x,&y);
        _instancia.Insere(x,y);
    }
    fclose(fp);

    _instancia.FinalizaGrid(0,_faixa,0,_faixa);

    cout << _instancia.Print();
    
}

void RGPManager::CreateProblem() {
    int i;
    size_t tamanho = static_cast<size_t>((_numeroPontos+1)*(_numeroPontos+1));
    GridIter x1,y1,x2,y2, fimx, fimy;

    _constraints.reserve(tamanho + 1);
    for (i=0; i < tamanho; i++) 
        InsertConstraint( new Constraint(1, ConstraintSign::Equal, 0, 0) );        

    _instancia.LimiteGridX(x1,fimx);

    for (; x1 != ( fimx-1 ) ; x1++) {
        for (x2=x1+1; x2 != fimx; x2++) {
            _instancia.LimiteGridY(y1,fimy);
            for (; y1 != ( fimy-1 ); y1++) {
                for (y2=y1+1; y2 != fimy; y2++) {
	                if ( !VerificaTerminalDentro(x1,y1,x2,y2) ) {
	                    if ( !VerificaTerminalNoCorner(x1,y1,x2,y2) ) 
	                        if ( !VerificaSteiners(x1,y1,x2,y2) ) {
		                        CreateVariable(x1,y1,x2,y2);
	                        }
	                }
	                else break;
                }
            }
        }
    }

    PostGenerationConstraintsReduction();

    char msg[20] = "";
    sprintf(msg, "%10.0f", _nonZeroCount);

    cout << endl;
    cout << "Points: " << _numeroPontos << endl;
    cout << "Original Variables: " << _colunas << endl;
    cout << "Variables after first reduction: " << _colunas2 << endl;
    cout << "Variables after second reduction: " << _colunas3 << endl; 
    cout << "Starting Variables: " << _colunas4 << endl; 
    cout << "Constraints: " << _constraints.size() << endl;
    cout << "Not Nulls: " << msg << endl;

    _totalVariaveis = _colunas4;
}

bool RGPManager::VerificaTerminalDentro(GridIter x1, GridIter y1, GridIter x2, GridIter y2) {
    x1++;
    int y;
    for(; x1 != x2; x1 ++) {
        y = _instancia.retornaY(*x1);
        if ( ( y > *y1) && (y < *y2) )
            return true;  /* Achou terminal dentro do retangulo */
    }
    _colunas++;
    return false;
}

bool RGPManager::VerificaTerminalNoCorner(GridIter x1, GridIter y1, GridIter x2, GridIter y2) {
    int y = _instancia.retornaY(*x1);
    if ( ( y == *y1) || (y == *y2) ) 
        return true;
  
    y = _instancia.retornaY(*x2);
    if ( (y == *y1) || (y == *y2) ) 
        return true;

    _colunas2++;
    return false;
}

bool RGPManager::VerificaSteiners(GridIter x1, GridIter y1, GridIter x2, GridIter y2) {
    int cpy1 = _instancia.retornaY(*x1);
    int cpy2 = _instancia.retornaY(*x2);
    int cpx1 = _instancia.retornaX(*y1);
    int cpx2 = _instancia.retornaX(*y2);
  
    if ( cpy1 > 0 ) {
        if ( (cpy1 < *y1) && (cpx1 < *x1) && (cpx1 > 0) ) return true;
        if ( (cpy1 > *y2) && (cpx2 < *x1) && (cpx2 > 0) ) return true;
    }
  
    if ( cpy2 > 0 ) {
        if ( (cpy2 < *y1) && (cpx1 > *x2) && (cpx1 > 0) ) return true;
        if ( (cpy2 > *y2) && (cpx2 > *x2) && (cpx2 > 0) ) return true; 
    }

    _colunas3++;
    return false;
}


/*************************************************************************/
/* Insercao da variavel nas listas internas do gerente e nas restricoes. */
/* Gera a informacao sobre as propriedades geometricas das variaveis     */
/*************************************************************************/

void RGPManager::CalculaSegmentos(RGPVariable *var, int px1, int py1, int px2, int py2) {
    int vy1, vy2, hx1, hx2;

    vy1 = _instancia.retornaY(px1);
    if ( (vy1 == -1) || (vy1 == _faixa) ) var->poeVerticalEsq(py1);
    else if ( vy1 < py1 ) var->poeVerticalEsq( _instancia.retornaYDelta(vy1,-1) );
    else if ( vy1 > py2 ) var->poeVerticalEsq( _instancia.retornaYDelta(vy1,1) );  
    else var->poeVerticalEsq( vy1 );
  
    vy2 = _instancia.retornaY(px2);
    if ( (vy2 == -1) || (vy2 == _faixa) ) var->poeVerticalDir(py1);
    else if ( vy2 < py1 ) var->poeVerticalDir( _instancia.retornaYDelta(vy2,-1) );
    else if ( vy2 > py2 ) var->poeVerticalDir( _instancia.retornaYDelta(vy2,1) );
    else var->poeVerticalDir( vy2 );
  
    hx1 = _instancia.retornaX(py1);
    if ( (hx1 == -1) || (hx1 == _faixa) ) var->poeHorizontalSup(px1);
    else if ( hx1 < px1 ) var->poeHorizontalSup( _instancia.retornaXDelta(hx1,-1) );
    else if ( hx1 > px2 ) var->poeHorizontalSup( _instancia.retornaXDelta(hx1,1) );
    else var->poeHorizontalSup( hx1 );

    hx2 = _instancia.retornaX(py2);
    if ( (hx2 == -1) || (hx2 == _faixa) ) var->poeHorizontalInf(px1);
    else if ( hx2 < px1 ) var->poeHorizontalInf( _instancia.retornaXDelta(hx2,-1) );
    else if ( hx2 > px2 ) var->poeHorizontalInf( _instancia.retornaXDelta(hx2,1) );
    else var->poeHorizontalInf( hx2 );
  
}


void RGPManager::CreateVariable(GridIter x1, GridIter y1, GridIter x2, GridIter y2) {
    int custo = 0;
    int segHorizontal, segVertical;
  
    RGPVariable *var;
    int nome = _colunas3-1;

    segHorizontal = *x2 - *x1;
    if ( *y1 == 0 ) custo += 2 * segHorizontal;
    else custo += segHorizontal;
    if ( *y2 == _faixa ) custo += 2 * segHorizontal;
    else custo += segHorizontal;
  
    segVertical = *y2 - *y1;
    if ( *x1 == 0 ) custo += 2 * segVertical;
    else custo += segVertical;
    if ( *x2 == _faixa ) custo += 2 * segVertical;
    else custo += segVertical;
 
    //float fator = (x2-x1) / (y2-y1);
    //if ( (fator < 0.333 ) || ( fator > 3 ) ) return; 
    //if (  ( (segVertical/segHorizontal) < 2/3 ) || ( (segVertical/segHorizontal) > 3/2) )
    //   return;
    

    var = new RGPVariable((float) custo,nome,*x1,*y1,*x2,*y2);
    CalculaSegmentos(var,*x1,*y1,*x2,*y2);

    //float ratio = stof(_config->getValue("PRICEOUTRATIO"));
    //float factor = ((float)segVertical / (float)segHorizontal);

    //to be reviewed - use a price out method
    // at this time the price out method can only be called when all variables are already generated.
    //if ( (factor < ratio) || ( (1/factor) < ratio) )
    //   var->setPricedOut();
    //else {
    //    var->unsetPricedOut();
    //    _colunas4++;
    //}
    _colunas4++;
    InsertVariable(var);
    InsertVariableIntoConstraint(var);

}

void RGPManager::InsertVariableIntoConstraint(Variable *var1) {

    int Ix1,Ix2,Iy1,Iy2;
    int Ix,Iy;
    int x1,x2,y1,y2;
    int line;

    RGPVariable *var = (RGPVariable *) var1;

    var->retRetangulo(x1,y1,x2,y2);

    Ix1 = _instancia.retornaIndiceX(x1);
    Ix2 = _instancia.retornaIndiceX(x2);
    Iy1 = _instancia.retornaIndiceY(y1);
    Iy2 = _instancia.retornaIndiceY(y2);

    line = ( _numeroPontos + 1 ) * Iy1;
    for ( Iy=Iy1; Iy < Iy2; Iy++ ) {
        for ( Ix=Ix1; Ix < Ix2; Ix++) {
            var->addConstraint( _constraints[static_cast<size_t>(line + Ix)] );
            _nonZeroCount++;
        }
        line += (_numeroPontos+1);
    }

}

void RGPManager::PostGenerationConstraintsReduction() {

    GridIter x, fimx;
    int y,Iy,Ix, index;

    _instancia.LimiteGridX(x,fimx);
    cout << endl;
    cout << "Eliminated Constraints." << endl;

    for (x ++; x != ( fimx-1 ) ; x++) {
        y =  _instancia.retornaY(*x);
        Ix = _instancia.retornaIndiceX(*x);
        Iy = _instancia.retornaIndiceY(y);

        index = (_numeroPontos + 1)*Iy + Ix;
		// delegate memory managemenet to the base class, just mark the constraint for deletion
        _constraints[index]->LogicalDelete();
        cout << "Indice: " << index << " " << *x << " " << y << endl;
    }

} 


void RGPManager::Solve(float InitialCost, float KnownBound) {

    if ( _colunas3 < 50000 ) _JaImprimiu = true;
    LagrangianManager::Solve(InitialCost,KnownBound);

}


void RGPManager::CustomProcessing() {
    if ( (!_JaImprimiu) && (  _colunas3 - _countFixed < 50000 ) )  {

        string file = DefaultFilePrefix() + ".lp";
        FILE *saida = fopen(file.c_str(),"w");
        ImprimeLP(saida);
        fclose(saida);

        _JaImprimiu = true;

        cout << "Gerou arquivo com lp reduzido" << endl;
        cout << TempoPassado(_algo->_tempoInicio) << endl;
        cout << TempoCpuPassado(_algo->_tempoInicioCpu) << endl;
        cout << TempoCpuAtual << endl;

    }
}


int RGPManager::Guilhotina() {
    GridIter x1,x2,y1,y2;
    _instancia.LimiteGridX(x1,x2);
    _instancia.LimiteGridY(y1,y2);
    return ( GuilhotinaRecursivo(*x1,*y1,*(--x2),*(--y2)) );
}

int RGPManager::GuilhotinaRecursivo(int x1, int y1, int x2, int y2) {
    int numeroPontos = 0;
    int valorCorte   = 10000;
    int cv,ch;
    int px,py;

    for ( int i = 1; i <= _numeroPontos ; i ++ ) {
        _instancia.retornaPontoX(px,py,i);
        if ( (px > x1) && ( px < x2 )  && ( py > y1) && (py < y2) ) {
            numeroPontos++;
            cv =  GuilhotinaRecursivo(x1,y1,px,y2);
            cv += GuilhotinaRecursivo(px,y1,x2,y2);
            cv += (y2-y1);
            ch =  GuilhotinaRecursivo(x1,y1,x2,py);
            ch += GuilhotinaRecursivo(x1,py,x2,y2);
            ch += (x2-x1);
            if ( cv < valorCorte ) valorCorte = cv;
            if ( ch < valorCorte ) valorCorte = ch;
        }
    }
    if ( numeroPontos == 0 ) return 0;
    return valorCorte;
}

void RGPManager::PrintSolution() {

    PointsIterator comeco,fim;
    string file = DefaultFilePrefix() + ".sol";

    FILE *fp = fopen(file.c_str(),"w");
    fprintf(fp,"%d\n",_numeroPontos);
    fprintf(fp,"%d\n",_faixa);

    _instancia.LimitePontosX(comeco, fim);
    for (; comeco != fim; comeco++) {
        fprintf(fp,"%d %d\n",(*comeco).first,(*comeco).second);
    }

    VariableIterator it    = _best.begin();
    VariableIterator itFim = _best.end();
  
    int x1,y1,x2,y2;
    for (;it != itFim; it++) {
        ((RGPVariable *)(*it))->retRetangulo(x1,y1,x2,y2);
        fprintf(fp, "x%d -", (*it)->getName());
        fprintf(fp,"%d %d %d %d\n",x1,y1,x2,y2);
    }
    fclose(fp);
}

void RGPManager::FinalStats() {
    LagrangianManager::FinalStats();
    RGPLagrangianRelaxation* l = (RGPLagrangianRelaxation*)_algo;
    cout << "Vezes 1  = " << l->_vezes1 << endl << "Vezes 2 = " << l->_vezes2 << endl;
    cout << "Fator Maior = " << l->_maximofator << endl;
    
    //felipe -- retirar
    //RGPVariable* var;
    //for (int i = 0; i < 150; i++) {
    //    var = (RGPVariable*)_variables[i];
    //    cout << i << " : " << var->_nome << " --- " << var->_valorLag << " --- " << var->_isPricedOut << " --- " << var->getCost() << " --- "  << var->Area() << endl;
    //}
    cout << endl << "Best Solution Found: " << endl << PrintVariableVector(_best);
}

string RGPManager::DefaultFilePrefix() {
    char arquivo[20] = "";
    sprintf(arquivo, "w%dh%dn%d", _faixa, _faixa, _numeroPontos);
    return arquivo;
}

