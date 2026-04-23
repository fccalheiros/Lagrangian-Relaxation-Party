#include "RGPCut.h"

RGPCut::RGPCut():
    Constraint(1,ConstraintSign::LowerEqual,0,30)
{}

RGPCut::RGPCut(float rhs):
    Constraint(rhs,ConstraintSign::LowerEqual,0,30)
{}

RGPCut::~RGPCut() { }



