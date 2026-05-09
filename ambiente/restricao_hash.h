#ifndef _RESTRICAO_HASH_H
#define _RESTRICAO_HASH_H

#include <stdlib.h>
#include "Constraint.h"



template <class T = Variable*>
struct lessVariavel {
    bool operator()(const T& x, const T& y) const {
        return x->_nome < y->_nome;
    }
};

struct eqVariavel {
    bool operator()(const Variable *v1, const Variable *v2) const
        {
            return ( v1->_nome == v2->_nome );
        }

};

#endif
