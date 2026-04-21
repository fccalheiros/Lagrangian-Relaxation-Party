#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "RGPVariable.h"

RGPVariable::RGPVariable() : Variable(),
    _hx1(0), _hx2(0), _vy1(0), _vy2(0),
    _x1(0), _x2(0), _y1(0), _y2(0) {
}


RGPVariable::RGPVariable(float coef, int nome)
    : RGPVariable() {
    Variable::operator=(Variable(coef, nome));
}

RGPVariable::RGPVariable(float coef, int nome, int x1, int y1, int x2, int y2)
    : RGPVariable(coef, nome) {
    Retangulo(x1, y1, x2, y2);
}

RGPVariable::RGPVariable(RGPVariable* v) :
    Variable((Variable*) v)
{    
     _hx1 = v->_hx1;
     _hx2 = v->_hx2;
     _vy1 = v->_vy1;
     _vy2 = v->_vy2;
     _x1  = v->_x1;
     _x2  = v->_x2;
     _y1  = v->_y1;
     _y2  = v->_y2;
}


Variable* RGPVariable::CopyAndClean(Variable* v) {

    if (v == NULL) {
        v = new RGPVariable(this);
        //felipe -- retirar
        ((RGPVariable*)v)->_out = _out;
    }
    else 
    {   
        RGPVariable* v1 = (RGPVariable*) v;
        v1->_hx1 = _hx1;
        v1->_hx2 = _hx2;
        v1->_vy1 = _vy1;
        v1->_vy2 = _vy2;
        v1->_x1 = _x1;
        v1->_x2 = _x2;
        v1->_y1 = _y1;
        v1->_y2 = _y2;

    }

    return Variable::CopyAndClean(v);

}

void RGPVariable::Retangulo(int x1, int y1, int x2, int y2) {
  _x1 = x1; 
  _y1 = y1; 
  _x2 = x2; 
  _y2 = y2; 
}

void RGPVariable::retRetangulo(int &x1, int &y1, int &x2, int &y2) {
  x1 = _x1; 
  y1 = _y1; 
  x2 = _x2; 
  y2 = _y2; 
}

void RGPVariable::ImprimeRetangulo() {
  cout << _x1 << " " << _y1 << " " << _x2 << " " << _y2;
}


int RGPVariable::Perimetro() {
  return ( 2*(_x2-_x1) + 2*(_y2-_y1) );
}

int RGPVariable::Area() {
  return ( (_x2 - _x1) * (_y2 - _y1) );
}

void RGPVariable::poeVerticalEsq(int vy1)         { _vy1 = vy1; }

void RGPVariable::poeVerticalDir(int vy2)         { _vy2 = vy2; }

void RGPVariable::poeHorizontalSup(int hx1)       { _hx1 = hx1; }
     
void RGPVariable::poeHorizontalInf(int hx2)       { _hx2 = hx2; }
 

void RGPVariable::retVerticalEsq(int &x, int &y1, int &y2) {
  x = _x1;
  y1 = _y1;
  y2 = _y2;

  if ( _vy1 < _y1 ) 
    y1 = _vy1;
  else if ( _vy1 > _y2 )
    y2 = _vy1;
}

void RGPVariable::retVerticalDir(int &x, int &y1, int &y2) {
  x = _x2;
  y1 = _y1;
  y2 = _y2;

  if ( _vy2 < _y1 ) 
    y1 = _vy2;
  else if ( _vy2 > _y2 )
    y2 = _vy2;
}

void RGPVariable::retHorizontalSup(int &y, int &x1, int &x2) {
  y = _y1;
  x1 = _x1;
  x2 = _x2;

  if ( _hx1 < _x1 ) 
    x1 = _hx1;
  else if ( _hx1 > _x2 )
    x2 = _hx1;
}

void RGPVariable::retHorizontalInf(int &y, int &x1, int &x2) {
  y = _y2;
  x1 = _x1;
  x2 = _x2;

  if ( _hx2 < _x1 ) 
    x1 = _hx2;
  else if ( _hx2 > _x2 )
    x2 = _hx2;
}

/* Verifica se dois segmentos se interceptam */

bool RGPVariable::InterceptaSegmentos(int x1, int y1, int x2, int y2, int vx1, int vy1, int vx2, int vy2) { 
  if ( x1 == x2 ) { 
    if (vx1 >= x1) return false;
    if (vx2 <= x1) return false;
    if (y1 >= vy1) return false;
    if (y2 <= vy1) return false;
    return true;
  }
  if (x1 >= vx1) return false;
  if (x2 <= vx1) return false;
  if (vy1 >= y1) return false;
  if (vy2 <= y1) return false;
  return true;
}

// Usar quando o primeiro segmento for vertical ( x igual )
bool RGPVariable::InterceptaSegmentosV(int x1, int y1, int x2, int y2, int vx1, int vy1, int vx2, int vy2) {
   if (vx1 >= x1) return false;
   if (vx2 <= x1) return false;
   if (y1 >= vy1) return false;
   if (y2 <= vy1) return false;
   return true;
}

// Usar quando o primeiro segmento for horizontal ( y igual )
bool RGPVariable::InterceptaSegmentosH(int x1, int y1, int x2, int y2, int vx1, int vy1, int vx2, int vy2) {
   if (x1 >= vx1) return false;
   if (x2 <= vx1) return false;
   if (vy1 >= y1) return false;
   if (vy2 <= y1) return false;
   return true;
}

/* Intercecao de duas variaveis. Testa se algum dos 4 segmentos de */
/* cada uma se intercepta */

bool RGPVariable::Intercepta(Variable *varBase) {
  int ax1, ax2, ay1, ay2;
  int bx1, bx2, by1, by2;
  int cx1, cx2, cy1, cy2;
  RGPVariable *var = (RGPVariable *) varBase;
  
  /* Horizontal superior + Vertical Esquerdo */
  retHorizontalSup(ay1,ax1,ax2);
  var->retVerticalEsq(bx1,by1,by2);
  if ( InterceptaSegmentosH(ax1,ay1,ax2,ay1, bx1,by1,bx1,by2) ) return true;
  
  /* Horizontal superior + Vertical Direito */
  var->retVerticalDir(cx2,cy1,cy2);
  if ( InterceptaSegmentosH(ax1,ay1,ax2,ay1, cx2,cy1,cx2,cy2) ) return true;
  
  /* Horizontal inferior + Vertical Esquerdo */
  retHorizontalInf(ay2,ax1,ax2);
  if ( InterceptaSegmentosH(ax1,ay2,ax2,ay2, bx1,by1,bx1,by2) ) return true;
  
  /* Horizontal inferior + Vertical Direito */ 
  if ( InterceptaSegmentosH(ax1,ay2,ax2,ay2, cx2,cy1,cx2,cy2) ) return true;
  
  /* Vertical esquerdo + Horizontal superior */
  retVerticalEsq(ax1,ay1,ay2);
  var->retHorizontalSup(by1,bx1,bx2);
  if ( InterceptaSegmentosV(ax1,ay1,ax1,ay2, bx1,by1,bx2,by1) ) return true;
  
  /* Vertical esquerdo + Horizontal inferior */
  var->retHorizontalInf(cy2,cx1,cx2);
  if ( InterceptaSegmentosV(ax1,ay1,ax1,ay2, cx1,cy2,cx2,cy2) ) return true;
  
  /* Vertical direito + Horizontal Superior */
  retVerticalDir(ax2,ay1,ay2);
  if ( InterceptaSegmentosV(ax2,ay1,ax2,ay2, bx1,by1,bx2,by1) ) return true;
  
  /* Vertical direito + Horizontal Inferior */
  if ( InterceptaSegmentosV(ax2,ay1,ax2,ay2, cx1,cy2,cx2,cy2) ) return true;
  return false;
}

// Recebe os segmentos VE, VD, HS, HI de outro retangulo para fazer a intercessao

bool RGPVariable::Intercepta(int veX, int veY1, int veY2, int vdX, int vdY1, int vdY2, 
                             int hsY, int hsX1, int hsX2, int hiY ,int hiX1, int hiX2) {
  int ax1, ax2, ay1, ay2;
  
  retHorizontalSup(ay1,ax1,ax2);
  /* Horizontal superior + Vertical Esquerdo */
 
  if ( InterceptaSegmentos(ax1,ay1,ax2,ay1, veX,veY1,veX,veY2) ) return true;
  /* Horizontal superior + Vertical Direito */
  if ( InterceptaSegmentos(ax1,ay1,ax2,ay1, vdX,vdY1,vdX,vdY2) ) return true;
  
  retHorizontalInf(ay2,ax1,ax2);
  /* Horizontal inferior + Vertical Esquerdo */
  if ( InterceptaSegmentos(ax1,ay2,ax2,ay2, veX,veY1,veX,veY2) ) return true;
  /* Horizontal inferior + Vertical Direito */ 
  if ( InterceptaSegmentos(ax1,ay2,ax2,ay2, vdX,vdY1,vdX,vdY2) ) return true;

  retVerticalEsq(ax1,ay1,ay2);
  /* Vertical esquerdo + Horizontal superior */
  if ( InterceptaSegmentos(ax1,ay1,ax1,ay2, hsX1,hsY,hsX2,hsY) ) return true;
  /* Vertical esquerdo + Horizontal inferior */
  if ( InterceptaSegmentos(ax1,ay1,ax1,ay2, hiX1,hiY,hiX2,hiY) ) return true;
  
  retVerticalDir(ax2,ay1,ay2);
  /* Vertical direito + Horizontal Superior */
  if ( InterceptaSegmentos(ax2,ay1,ax2,ay2, hsX1,hsY,hsX2,hsY) ) return true;
  /* Vertical direito + Horizontal Inferior */
  if ( InterceptaSegmentos(ax2,ay1,ax2,ay2, hiX1,hiY,hiX2,hiY) ) return true;
 
  return false;
} 

void RGPVariable::RetornaSegmentos(int &veX, int &veY1, int &veY2, int &vdX, int &vdY1, int &vdY2, 
				   int &hsY, int &hsX1, int &hsX2, int &hiY ,int &hiX1, int &hiX2)
{
   retVerticalEsq(veX,veY1,veY2);
   retVerticalDir(vdX,vdY1,vdY2);
   retHorizontalSup(hsY,hsX1,hsX2);
   retHorizontalInf(hiY,hiX1,hiX2);  
}
