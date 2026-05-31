#ifndef __TRAITS_H__
#define __TRAITS_H__
 
#include "basetrait.h"

template <typename T, template<typename> class NodeType>
struct AscendingContainerTrait {
    using value_type = T;
    using Node       = NodeType<T>;
    using Comp       = less<T>;
};

template <typename T, template<typename> class NodeType>
struct DescendingContainerTrait {
    using value_type = T;
    using Node       = NodeType<T>;
    using Comp       = greater<T>;
};
 
#endif // __TRAITS_H__