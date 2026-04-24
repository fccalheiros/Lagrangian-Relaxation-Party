#ifndef _RGPVARIABLE_H
#define _RGPVARIABLE_H

#include "Variable.h"

class RGPVariable : public Variable {



protected:

	short int _x1;
	short int _y1;
	short int _x2;
	short int _y2;

	// As variaveis a seguir controlam os seguimentos extendidos do RG-NLP
	// Cada variavel controla a coordenada do ponto que esta na mesma linha 
	// de grade que cada um dos quatro segmentos que compoem o retangulo.
	// Assim, _vx1 e': A coordenada x do ponto que esta no mesma reta 
	// da vertical superior.
  
	short int _vy1;
	short int _vy2;
	short int _hx1;
	short int _hx2;

 public:

	RGPVariable();
	RGPVariable(float coef, int nome = 0);
	RGPVariable(float coef, int nome, int x1, int y1, int x2, int y2);
	RGPVariable(RGPVariable* v);

	inline void Retangulo(int x1, int y1, int x2, int y2);
	void retRetangulo(int &x1, int &y1, int &x2, int &y2);
	void ImprimeRetangulo();
	int Perimetro();
	int Area();

	void poeVerticalEsq(int vy1);
	void poeVerticalDir(int vy2);
	void poeHorizontalSup(int hx1);
	void poeHorizontalInf(int hx2);

	void retVerticalEsq(int &x, int &y1, int &y2);
	void retVerticalDir(int &x, int &y1, int &y2);
	void retHorizontalSup(int &y, int &x1, int &x2);
	void retHorizontalInf(int &y, int &x1, int &x2);

	virtual inline bool Intercepta(Variable *varBase);
	virtual Variable* CopyAndClean(Variable* v);

	bool Intercepta(int veX, int veY1, int veY2, int vdX, int vdY1, int vdY2, 
				int hsY, int hsX1, int hsX2, int hiY ,int hiX1, int hiX2);

	void RetornaSegmentos(int &veX, int &veY1, int &veY2, int &vdX, int &vdY1, int &vdY2, 
			int &hsY, int &hsX1, int &hsX2, int &hiY ,int &hiX1, int &hiX2);
  
 protected:

	inline bool InterceptaSegmentos(int x1, int y1, int x2, int y2, int vx1, int vy1, int vx2, int vy2);
	inline bool InterceptaSegmentosV(int x1, int y1, int x2, int y2, int vx1, int vy1, int vx2, int vy2);
	inline bool InterceptaSegmentosH(int x1, int y1, int x2, int y2, int vx1, int vy1, int vx2, int vy2);


};

#endif
