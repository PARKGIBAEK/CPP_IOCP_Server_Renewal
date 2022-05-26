module;
#include <stack>
#include <array>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <list>
#include <unordered_map>
#include <unordered_set>
export module Container;

import Types;


export template<typename Type, uint64 Size>
using Array = std::array<Type, Size>;

//export template<typename Type>
//using Vector = std::vector<Type, STL_Allocator<Type>>;
//
