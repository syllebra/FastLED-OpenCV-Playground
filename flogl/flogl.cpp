#include "flogl.hpp"
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>
#include <string>

#include <opencv2/opencv.hpp>

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

#include "LedTexture.hpp"

   class Flogl::Impl
   {
   public:
      Impl(std::vector<LED> &led_coordinates, const Config &config);
      ~Impl();

      int draw();

      std::vector<LED> &m_leds;
      cv::Mat render;
   };

   Flogl::Impl::Impl(std::vector<LED> &leds, const Config &config) : m_leds(leds)
   {

      // image = imread( argv[1], 1 );
      this->render = cv::Mat::zeros(cv::Size(config.width()/3, config.height()/3), CV_8UC3);

      cv::namedWindow("FastLED Dev", cv::WINDOW_AUTOSIZE);
      cv::imshow("FastLED Dev", this->render);
      cv::waitKey(10);
   }

   int Flogl::Impl::draw()
   {

      static cv::Mat fl = cv::imread("lamp_low.png");


      this->render = cv::Scalar(0,0,0);
      int i = 0;
      for (LED &p : m_leds)
      {
         // cv::Mat part;
         // cv::resize(fl,part,cv::Size(p.size*4, p.size*4));
         
         // cv::multiply(part, cv::Scalar(p.color->b,p.color->g,p.color->r), part);
         // //cv::Mat col = tmp * cv::Scalar(p.color->b,p.color->g,p.color->r);
         // cv::Mat insetImage(this->render, cv::Rect(p.x/3,p.y/3, part.cols, part.rows));
         // insetImage+=part;
         //part.copyTo(insetImage);
         cv::circle(this->render, cv::Point(p.x/3,p.y/3), p.size/3, cv::Scalar(p.color->b,p.color->g,p.color->r), -1, cv::LINE_AA);
         // printf("%d: %.2f %.2f %.2f\n", i, p.x,p.y,p.z);
         // printf("%d: %d %d %d\n", i, p.color->r,p.color->g,p.color->b);
         // // Fill the GPU buffer
         // m_led_position_size_data[i] = p;
         // m_led_color_data[i] = p;
         i++;
      }
      cv::imshow("FastLED Dev", this->render);
      int key = cv::waitKey(5);
      if(key == 27)
         exit(0);
      return key; // m_window.shouldClose();
   }

   Flogl::Impl::~Impl()
   {
      cv::destroyAllWindows();
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
