#ifndef TKS_SHAPE_H
#define TKS_SHAPE_H

#include "update_system.h"

namespace tk
{

class SShape : public SUpdate
{
public:
  virtual void Update(f32 dt) override;
};

} // namespace tk

#endif // !TKS_SHAPE_H
