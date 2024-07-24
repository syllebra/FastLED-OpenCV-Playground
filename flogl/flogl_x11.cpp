#include "flogl.hpp"
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>
#include <string>

#include <assert.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>

#include <cairo.h>
#include <cairo-xlib.h>

#include <chrono>
#include <thread>

#include <math.h>


//using namespace cv;

// FastLED needs this symbol. Find a better place for it.
// It is supposed to be provided by the application, so
// we make it weak to allow for it to be reimplemented
uint16_t XY(uint8_t, uint8_t) __attribute__((weak));
uint16_t XY(uint8_t, uint8_t)
{
   return 0;
}

namespace flogl
{

   namespace
   {

      struct LedPosition
      {
         float x, y, z, size;

         void operator=(const LED &l)
         {
            x = l.x;
            y = l.y;
            z = l.z;
            size = l.size;
         }
      };

      struct LedColor
      {
         int8_t r, g, b;

         void operator=(const LED &l)
         {
            if (l.color != nullptr)
            {
               r = l.color->r;
               g = l.color->g;
               b = l.color->b;
            }
         }
      };

   }

   class Flogl::Impl
   {
   public:
      Impl(std::vector<LED> &led_coordinates, const Config &config);
      ~Impl();

      int draw();

      std::vector<LED> &m_leds;

      cairo_t *cr;
      Display *display;
      cairo_surface_t *surf;
      Window overlay;
   };



   Flogl::Impl::Impl(std::vector<LED> &leds, const Config &config) : m_leds(leds)
   {

      int width = 300;
      int height = 300;
      this->display = XOpenDisplay(NULL);
      Window root = DefaultRootWindow(this->display);
      //int default_screen = XDefaultScreen(this->display);

      // these two lines are really all you need
      XSetWindowAttributes attrs;
      attrs.override_redirect = true;

      XVisualInfo vinfo;
      if (!XMatchVisualInfo(this->display, DefaultScreen(this->display), 32, TrueColor, &vinfo)) {
         printf("No visual found supporting 32 bit color, terminating\n");
         exit(EXIT_FAILURE);
      }
      // these next three lines add 32 bit depth, remove if you dont need and change the flags below
      attrs.colormap = XCreateColormap(this->display, root, vinfo.visual, AllocNone);
      attrs.background_pixel = 0;
      attrs.border_pixel = 0;

      // Window XCreateWindow(
      //     Display *display, Window parent,
      //     int x, int y, unsigned int width, unsigned int height, unsigned int border_width,
      //     int depth, unsigned int class, 
      //     Visual *visual,
      //     unsigned long valuemask, XSetWindowAttributes *attributes
      // );
      this->overlay = XCreateWindow(
         this->display, root,
         0, 0, width, height, 0,
         vinfo.depth, InputOutput, 
         vinfo.visual,
         CWOverrideRedirect | CWColormap | CWBackPixel | CWBorderPixel, &attrs
      );

      XMapWindow(this->display, this->overlay);

      this->surf = cairo_xlib_surface_create(this->display, this->overlay,
                                    vinfo.visual,
                                    width, height);
      this->cr = cairo_create(this->surf);
   }

   int Flogl::Impl::draw()
   {
      int i = 0;
            //this->render = cv::Scalar(0,0,0);
      cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
      //cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
      cairo_rectangle(cr, 0, 0, 300, 300);
      cairo_paint_with_alpha(cr, 1.0);
      //cairo_set_operator(cr, CAIRO_OPERATOR_DEST);
      for (LED &p : m_leds)
      {
         //cv::circle(this->render, cv::Point(p.x,p.y), p.size, cv::Scalar(p.color->b,p.color->g,p.color->r), -1, cv::LINE_AA);
         cairo_set_source_rgb(cr,  p.color->r,p.color->g,p.color->b);
         //cairo_arc(cr,p.x*0.4,p.y*0.4,p.size*0.4,0,2*M_PI);
         cairo_rectangle(cr, p.x*0.4,p.y*0.4, p.size*0.4, p.size*0.4);
         cairo_fill(cr);
         //cairo_arc(cr, 150, 350, 50, 0, 2 * M_PI);
         // 
         // cairo_set_source_rgb(cr, 0, 0, 0);
         // cairo_stroke(cr);
         // printf("%d: %.2f %.2f %.2f\n", i, p.x,p.y,p.z);
         // printf("%d: %d %d %d\n", i, p.color->r,p.color->g,p.color->b);
         i++;
      }

      XFlush(this->display);
      std::this_thread::sleep_for(std::chrono::milliseconds(3));
      return 0; // m_window.shouldClose();
   }

   Flogl::Impl::~Impl()
   {
      // show the window for 10 seconds
      //std::this_thread::sleep_for(std::chrono::milliseconds(10000));

      cairo_destroy(this->cr);
      cairo_surface_destroy(this->surf);

      XUnmapWindow(this->display, this->overlay);
      XCloseDisplay(this->display);      
   }

   Flogl::Flogl(std::vector<LED> &led_coordinates, const Config &config) : m_i(*new Flogl::Impl(led_coordinates, config))
   {
   }

   int Flogl::draw()
   {
      return m_i.draw();
   }

   void Flogl::delay(unsigned int ms)
   {
      this->show();
      usleep(ms*1000);
   }
   void Flogl::show()
   {
      m_i.draw();
   }

   Flogl::~Flogl()
   {
      delete &m_i;
   }

}
