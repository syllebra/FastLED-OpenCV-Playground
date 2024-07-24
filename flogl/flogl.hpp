#ifndef FLOGL_HPP
#define FLOGL_HPP

#include "platform.h"
#include "FastLED.h"
#include "Config.hpp"
#include <unistd.h>

namespace flogl {

struct LED{
   LED(float x, float y, float z, float size = 10.0):
         x(x), y(y), z(z),
         size(size),
         color(NULL)
   {
   }
	float x, y, z;
	float size;
   CRGB* color;
};

class Flogl
{
public:
   Flogl(std::vector<LED>& led_coordinates, const Config& config = Config());

   int draw();

   void delay(unsigned int ms);
   void show();
   
   ~Flogl();
private:
   class Impl;
   Impl& m_i;
};

}
#endif // FLOGL_HPP
