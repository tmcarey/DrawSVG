#include "software_renderer.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>

#include "triangulation.h"

using namespace std;

namespace CMU462 {


// Implements SoftwareRenderer //

void SoftwareRendererImp::draw_svg( SVG& svg ) {

  // set top level transformation
  transformation = svg_2_screen;

  // draw all elements
  for ( size_t i = 0; i < svg.elements.size(); ++i ) {
    draw_element(svg.elements[i]);
  }

  // draw canvas outline
  Vector2D a = transform(Vector2D(    0    ,     0    )); a.x--; a.y--;
  Vector2D b = transform(Vector2D(svg.width,     0    )); b.x++; b.y--;
  Vector2D c = transform(Vector2D(    0    ,svg.height)); c.x--; c.y++;
  Vector2D d = transform(Vector2D(svg.width,svg.height)); d.x++; d.y++;

  rasterize_line(a.x, a.y, b.x, b.y, Color::Black);
  rasterize_line(a.x, a.y, c.x, c.y, Color::Black);
  rasterize_line(d.x, d.y, b.x, b.y, Color::Black);
  rasterize_line(d.x, d.y, c.x, c.y, Color::Black);

  // resolve and send to render target
  resolve();

}

void SoftwareRendererImp::set_sample_rate( size_t sample_rate ) {

  // Task 4: 
  // You may want to modify this for supersampling support
  this->sample_rate = sample_rate;

}

void SoftwareRendererImp::set_render_target( unsigned char* render_target,
                                             size_t width, size_t height ) {

  // Task 4: 
  // You may want to modify this for supersampling support
  this->render_target = render_target;
  this->target_w = width;
  this->target_h = height;
  this->super_sample_buffer = memset
}

void SoftwareRendererImp::draw_element( SVGElement* element ) {

  // Task 5 (part 1):
  // Modify this to implement the transformation stack

  switch(element->type) {
    case POINT:
      draw_point(static_cast<Point&>(*element));
      break;
    case LINE:
      draw_line(static_cast<Line&>(*element));
      break;
    case POLYLINE:
      draw_polyline(static_cast<Polyline&>(*element));
      break;
    case RECT:
      draw_rect(static_cast<Rect&>(*element));
      break;
    case POLYGON:
      draw_polygon(static_cast<Polygon&>(*element));
      break;
    case ELLIPSE:
      draw_ellipse(static_cast<Ellipse&>(*element));
      break;
    case IMAGE:
      draw_image(static_cast<Image&>(*element));
      break;
    case GROUP:
      draw_group(static_cast<Group&>(*element));
      break;
    default:
      break;
  }

}


// Primitive Drawing //

void SoftwareRendererImp::draw_point( Point& point ) {

  Vector2D p = transform(point.position);
  rasterize_point( p.x, p.y, point.style.fillColor );

}

void SoftwareRendererImp::draw_line( Line& line ) { 

  Vector2D p0 = transform(line.from);
  Vector2D p1 = transform(line.to);
  rasterize_line( p0.x, p0.y, p1.x, p1.y, line.style.strokeColor );

}

void SoftwareRendererImp::draw_polyline( Polyline& polyline ) {

  Color c = polyline.style.strokeColor;

  if( c.a != 0 ) {
    int nPoints = polyline.points.size();
    for( int i = 0; i < nPoints - 1; i++ ) {
      Vector2D p0 = transform(polyline.points[(i+0) % nPoints]);
      Vector2D p1 = transform(polyline.points[(i+1) % nPoints]);
      rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    }
  }
}

void SoftwareRendererImp::draw_rect( Rect& rect ) {

  Color c;
  
  // draw as two triangles
  float x = rect.position.x;
  float y = rect.position.y;
  float w = rect.dimension.x;
  float h = rect.dimension.y;

  Vector2D p0 = transform(Vector2D(   x   ,   y   ));
  Vector2D p1 = transform(Vector2D( x + w ,   y   ));
  Vector2D p2 = transform(Vector2D(   x   , y + h ));
  Vector2D p3 = transform(Vector2D( x + w , y + h ));
  
  // draw fill
  c = rect.style.fillColor;
  if (c.a != 0 ) {
    rasterize_triangle( p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, c );
    rasterize_triangle( p2.x, p2.y, p1.x, p1.y, p3.x, p3.y, c );
  }

  // draw outline
  c = rect.style.strokeColor;
  if( c.a != 0 ) {
    rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    rasterize_line( p1.x, p1.y, p3.x, p3.y, c );
    rasterize_line( p3.x, p3.y, p2.x, p2.y, c );
    rasterize_line( p2.x, p2.y, p0.x, p0.y, c );
  }

}

void SoftwareRendererImp::draw_polygon( Polygon& polygon ) {

  Color c;

  // draw fill
  c = polygon.style.fillColor;
  if( c.a != 0 ) {

    // triangulate
    vector<Vector2D> triangles;
    triangulate( polygon, triangles );

    // draw as triangles
    for (size_t i = 0; i < triangles.size(); i += 3) {
      Vector2D p0 = transform(triangles[i + 0]);
      Vector2D p1 = transform(triangles[i + 1]);
      Vector2D p2 = transform(triangles[i + 2]);
      rasterize_triangle( p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, c );
    }
  }

  // draw outline
  c = polygon.style.strokeColor;
  if( c.a != 0 ) {
    int nPoints = polygon.points.size();
    for( int i = 0; i < nPoints; i++ ) {
      Vector2D p0 = transform(polygon.points[(i+0) % nPoints]);
      Vector2D p1 = transform(polygon.points[(i+1) % nPoints]);
      rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    }
  }
}

void SoftwareRendererImp::draw_ellipse( Ellipse& ellipse ) {

  // Extra credit 

}

void SoftwareRendererImp::draw_image( Image& image ) {

  Vector2D p0 = transform(image.position);
  Vector2D p1 = transform(image.position + image.dimension);

  rasterize_image( p0.x, p0.y, p1.x, p1.y, image.tex );
}

void SoftwareRendererImp::draw_group( Group& group ) {

  for ( size_t i = 0; i < group.elements.size(); ++i ) {
    draw_element(group.elements[i]);
  }

}

// Rasterization //

// The input arguments in the rasterization functions 
// below are all defined in screen space coordinates

void SoftwareRendererImp::rasterize_point( float x, float y, Color color ) {

  // fill in the nearest pixel
  int sx = (int) floor(x);
  int sy = (int) floor(y);

  if ( sx < 0 || sx >= target_w ) return;
  if ( sy < 0 || sy >= target_h ) return;

    Color originalColor = Color(
      0.0f, //(float)(render_target[4 * (sx + sy * target_w)]) / 255.0f,
      0.0f, //(float)(render_target[4 * (sx + sy * target_w) + 1]) / 255.0f,
      0.0f, //(float)(render_target[4 * (sx + sy * target_w) + 2]) / 255.0f,
      1.0f
    );
    color.r *= color.a;
    color.g *= color.a;
    color.b *= color.a;
    render_target[4 * (sx + sy * target_w)] = (uint8_t)((((1 - color.a) * originalColor.r) + color.r) * 255);
    render_target[4 * (sx + sy * target_w) + 1] = (uint8_t)((((1 - color.a) * originalColor.g) + color.g) * 255);
    render_target[4 * (sx + sy * target_w) + 2] = (uint8_t)((((1 - color.a) * originalColor.b) + color.b) * 255);
    render_target[4 * (sx + sy * target_w) + 3] = (uint8_t)(255);
    /*render_target[4 * (sx + sy * target_w)] = (uint8_t)(color.r * 255);
    render_target[4 * (sx + sy * target_w) + 1] = (uint8_t)(color.g * 255);
    render_target[4 * (sx + sy * target_w) + 2] = (uint8_t)(color.b * 255);
    render_target[4 * (sx + sy * target_w) + 3] = (uint8_t)(color.a * 255);*/
}

void SoftwareRendererImp::rasterize_line( float x0, float y0,
                                          float x1, float y1,
                                          Color color) {

  // Task 2: 
  // Implement line rasterization

  bool ySteep = abs(y1 - y0) > abs(x1 - x0);
  const float WIDTH = 1;

  //Ensure Delta X > Delta Y and we are positively oriented
  if(ySteep){
    float temp = x0;
    x0 = y0;
    y0 = temp;
    temp = x1;
    x1 = y1;
    y1 = temp;
  }

  bool positiveOrientation = x1 > x0;
  if(!positiveOrientation){
    float temp = x0;
    x0 = x1;
    x1 = temp;
    temp = y0;
    y0 = y1;
    y1 = temp;
  }

  // transform coords so center of pixels are (0, 0)
  y1 -= 0.5f;
  y0 -= 0.5f;
  x1 -= 0.5f;
  x0 -= 0.5f;

  float deltaX = x1 - x0;
  float deltaY = y1 - y0;
  float gradient = deltaY / deltaX;
  if (deltaX == 0) {
    gradient = 1.0f;
  }
  
  float xStart = floor(x0 + 0.5f);
  float yStart = y0 + gradient  * (xStart - x0);
  float xDist = 1 - (x0 + 0.5f - xStart);
  float yDist = (yStart) - floor(yStart) ;
  float xPixel0 = xStart;
  float yPixel0 = floor(yStart);
  if (ySteep){
    rasterize_point(yPixel0, xPixel0, color * (xDist) * (1 - yDist));
    rasterize_point(yPixel0 + 1, xPixel0, color * xDist * (yDist));
  }else{
    rasterize_point(xPixel0, yPixel0, color * xDist * (1 - yDist));
    rasterize_point(xPixel0, yPixel0 + 1, color * xDist * (yDist));
  }

  float xEnd = floor(x1 + 0.5f);
  float yEnd = y1 + gradient * (xEnd - x1);
  xDist = (x1  + 0.5f - xEnd);
  yDist = (yEnd) - floor(yEnd);
  int xPixel1 = xEnd;
  int yPixel1 = floor(yEnd);
  if (ySteep){
    rasterize_point(yPixel1, xPixel1, color * (xDist) * (1 - yDist));
    rasterize_point(yPixel1 + 1, xPixel1, color * (xDist) * (yDist));
  }else{
    rasterize_point(xPixel1, yPixel1, color * (xDist) * (1 - yDist));
    rasterize_point(xPixel1, yPixel1 + 1, color * (xDist) * (yDist));
  }

  
  float yIntersect = yStart + gradient;
  float yIntersectFract = yIntersect - floor(yIntersect);
  if (ySteep) {
     for (float x = xPixel0 + 1; x < xPixel1; x++){
       rasterize_point(floor(yIntersect), x, color * (1 - (yIntersectFract)));
       for(float i = 1; i < WIDTH; i++){
         rasterize_point(floor(yIntersect) + i, x, color);
       }
       rasterize_point(floor(yIntersect) + WIDTH, x, color * yIntersectFract);
       yIntersect += gradient;
       yIntersectFract = yIntersect - floor(yIntersect);
     }
  }else{    
    for (float x = xPixel0 + 1; x < xPixel1; x++){
       rasterize_point(x, floor(yIntersect), color * (1 - (yIntersectFract)));
       for(float i = 1; i < WIDTH; i++){
         rasterize_point(x, floor(yIntersect) + i, color);
       }
       rasterize_point(x, floor(yIntersect) + WIDTH, color * yIntersectFract);
       yIntersect += gradient;
       yIntersectFract = yIntersect - floor(yIntersect);
     }
  }
}

void SoftwareRendererImp::rasterize_triangle( float x0, float y0,
                                              float x1, float y1,
                                              float x2, float y2,
                                              Color color ) {
  // Task 3: 
  // Implement triangle rasterization
  // transform coords so center of pixels are (0, 0)
  x0 -= 0.5f;
  x1 -= 0.5f;
  x2 -= 0.5f;
  y2 -= 0.5f;
  y1 -= 0.5f;
  y0 -= 0.5f;
  float maxX = floor(max(x0, max(x1, x2)) + 0.5f);
  float minX = floor(min(x0, min(x1, x2)) + 0.5f);
  float maxY = floor(max(y0, max(y1, y2)) + 0.5f);
  float minY = floor(min(y0, min(y1, y2)) + 0.5f);
  Vector2D vec0 = Vector2D((x1 - x0), (y1 - y0));
  Vector2D vec1 = Vector2D((x2 - x1), (y2 - y1));
  Vector2D vec2 = Vector2D((x0 - x2), (y0 - y2));
  bool isCounterClockwise = cross(vec0, -1 * vec2) > 0;

  float sampleRate = (float) sample_rate;
  float sampleOffset = 1.0f / (sampleRate);
  float halfOffset = sampleOffset / 2.0f;
  float totalSamples = (float) sampleRate * sampleRate;
  for (int x = minX; x < maxX + 1; x++){
    for (int y = minY; y < maxY + 1; y++){
      int trueSamples = 0;
      for (float sx = 0; sx < sampleRate; sx++){
        for (float sy = 0; sy < sampleRate; sy++){
          bool doesContain = false;
          float sampledX = (x - 0.5f) + halfOffset + (sx * sampleOffset);
          float sampledY = (y - 0.5f) + halfOffset + (sy * sampleOffset);
          if (isCounterClockwise)
          {
            doesContain = cross(Vector2D(sampledX - x0, sampledY - y0), vec0) <= 0
            && cross(Vector2D(sampledX - x1, sampledY - y1), vec1) <= 0 
            && cross(Vector2D(sampledX - x2, sampledY - y2), vec2) <= 0;
          }
          else
          {
            doesContain = cross(Vector2D(sampledX - x0, sampledY - y0), vec0) >= 0
            && cross(Vector2D(sampledX - x1, sampledY - y1), vec1) >= 0 
            && cross(Vector2D(sampledX - x2, sampledY - y2), vec2) >= 0;
          }
          if(doesContain){
            trueSamples++;
          }
        }
      }
      if(trueSamples > 0){
        rasterize_point(x, y, color * ((float)(trueSamples) / sample_rate));
      }
   }
  }
}

void SoftwareRendererImp::rasterize_image( float x0, float y0,
                                           float x1, float y1,
                                           Texture& tex ) {
  // Task 6: 
  // Implement image rasterization

}

// resolve samples to render target
void SoftwareRendererImp::resolve( void ) {

  // Task 4: 
  // Implement supersampling
  // You may also need to modify other functions marked with "Task 4".
  return;

}


} // namespace CMU462
