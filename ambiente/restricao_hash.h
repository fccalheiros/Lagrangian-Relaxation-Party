#ifndef _RESTRICAO_HASH_H
#define _RESTRICAO_HASH_H

#include <stdlib.h>
#include "Constraint.h"

using namespace std;

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

inline size_t __stl_hash_variavel(const Variable* __v)
{
  return (size_t) __v->_nome;
}


template<> struct hash<Variable *>                             // Alterada de _STL_TEMPLATE_NULL para template <>
{
    size_t operator()(const Variable* __v) const { return __stl_hash_variavel(__v); }
};
                                                                 // Incluida

#endif
