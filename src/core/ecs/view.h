#ifndef TK_VIEW_H
#define TK_VIEW_H

#include "entt.hpp"

namespace tk
{
template <typename T> using View = entt::view<T>;
}

#endif // !TK_VIEW_H
