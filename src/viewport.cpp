#include "viewport.h"

#include "CMU462.h"

namespace CMU462 {

void ViewportImp::set_viewbox( float centerX, float centerY, float vspan ) {

  // Task 5 (part 2): 
  // Set svg coordinate to normalized device coordinate transformation. Your input
  // arguments are defined as normalized SVG canvas coordinates.
  this->centerX = centerX;
  this->centerY = centerY;
  this->vspan = vspan; 

  double scale = (1.0f / (double)(vspan * 2.0f));
  double mat [9] = { 
    scale, 0.0, -(centerX - vspan) * scale,
    0.0, scale, -(centerY - vspan) * scale,
    0.0, 0.0, 1.0
  };
  set_svg_2_norm( Matrix3x3 (mat));
}

void ViewportImp::update_viewbox( float dx, float dy, float scale ) { 
  
  this->centerX -= dx;
  this->centerY -= dy;
  this->vspan *= scale;
  set_viewbox( centerX, centerY, vspan );
}

} // namespace CMU462
