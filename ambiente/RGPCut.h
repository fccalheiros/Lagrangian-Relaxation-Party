#ifndef _RGPCUT_H
#define _RGPCUT_H

#include "Constraint.h"


class RGPCut : public Constraint {

 public:

    RGPCut();
    RGPCut(float rhs);
    virtual ~RGPCut();

};


#endif
