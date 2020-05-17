#pragma once

#include "macros.h"
#include "model.h"

namespace pixel {

class ModelRenderer {
 public:
  ModelRenderer(model::Model model);

  bool Prepare();

  bool Update();

  bool Render();

 private:
  model::Model model_;

  P_DISALLOW_COPY_AND_ASSIGN(ModelRenderer);
};

}  // namespace pixel
