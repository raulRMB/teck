#ifndef TK_C_TRANSFORM_2D
#define TK_C_TRANSFORM_2D

#include "core/types.h"

namespace tk
{

struct CTransform
{
  v2 Position = v2(0.f);
  f32 Rotation = 0.f;
  f32 Scale = 0.f;
};

} // namespace tk

#endif // !TK_C_TRANSFORM_2D
