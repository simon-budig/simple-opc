#include <math.h>

#include "render-utils.h"

double
euclid_3d (double x,
           double y,
           double z)
{
   return sqrt (x*x + y*y + z*z);
}


void
pixel_set (double *framebuffer,
           int     x,
           int     y,
           int     z,
           double  red,
           double  green,
           double  blue)
{
  if (x < 0 || y < 0 || z < 0)
    return;

  if (x >= 8 || y >= 8 || z >= 8)
    return;

  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 0] = red;
  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 1] = green;
  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 2] = blue;
}


void
render_pixel (double *framebuffer,
              int     x,
              int     y,
              int     z,
              double  red,
              double  green,
              double  blue,
              double  alpha)
{
  if (x < 0 || y < 0 || z < 0)
    return;

  if (x >= 8 || y >= 8 || z >= 8)
    return;

  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 0] *= 1.0   - alpha;
  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 0] += red   * alpha;

  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 1] *= 1.0   - alpha;
  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 1] += green * alpha;

  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 2] *= 1.0   - alpha;
  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 2] += blue  * alpha;
}


void
interpolate_pixel (double *fb,
                   double  x,
                   double  y,
                   double  z,
                   double  red,
                   double  green,
                   double  blue,
                   double  alpha)
{
  render_pixel (fb,
                (int) x,
                (int) y,
                (int) z,
                red, green, blue,
                alpha *
                (1.0 - (x - (int) x)) *
                (1.0 - (y - (int) y)) *
                (1.0 - (z - (int) z)));

  render_pixel (fb,
                1 + (int) x,
                (int) y,
                (int) z,
                red, green, blue,
                alpha *
                ((x - (int) x)) *
                (1.0 - (y - (int) y)) *
                (1.0 - (z - (int) z)));

  render_pixel (fb,
                (int) x,
                1 + (int) y,
                (int) z,
                red, green, blue,
                alpha *
                (1.0 - (x - (int) x)) *
                ((y - (int) y)) *
                (1.0 - (z - (int) z)));

  render_pixel (fb,
                (int) x,
                (int) y,
                1 + (int) z,
                red, green, blue,
                alpha *
                (1.0 - (x - (int) x)) *
                (1.0 - (y - (int) y)) *
                ((z - (int) z)));

  render_pixel (fb,
                (int) x,
                1 + (int) y,
                1 + (int) z,
                red, green, blue,
                alpha *
                (1.0 - (x - (int) x)) *
                ((y - (int) y)) *
                ((z - (int) z)));

  render_pixel (fb,
                1 + (int) x,
                (int) y,
                1 + (int) z,
                red, green, blue,
                alpha *
                ((x - (int) x)) *
                (1.0 - (y - (int) y)) *
                ((z - (int) z)));

  render_pixel (fb,
                1 + (int) x,
                1 + (int) y,
                (int) z,
                red, green, blue,
                alpha *
                ((x - (int) x)) *
                ((y - (int) y)) *
                (1.0 - (z - (int) z)));

  render_pixel (fb,
                1 + (int) x,
                1 + (int) y,
                1 + (int) z,
                red, green, blue,
                alpha *
                ((x - (int) x)) *
                ((y - (int) y)) *
                ((z - (int) z)));
}


void
render_blob (double *framebuffer,
             double cx, double cy, double cz,
             double red, double green, double blue,
             double r, double s)
{
  int X, Y, Z;
  double x, y, z;

  for (X = 0; X < 8; X++)
    {
      for (Y = 0; Y < 8; Y++)
        {
          for (Z = 0; Z < 8; Z++)
            {
              double d;

              x = X * 0.25;
              y = Y * 0.25;
              z = Z * 0.25;

              d = pow ((x - cx) * (x - cx) +
                       (y - cy) * (y - cy) +
                       (z - cz) * (z - cz), 0.5);

              d /= r;
              d = (d - 0.5) * s + 0.5;
              d = CLAMP (d, 0.0, 1.0);

              d = pow (d, s);

              render_pixel (framebuffer, X, Y, Z, red, green, blue, 1.0 - d);
            }
        }
    }
}


void
render_paddle (double *framebuffer,
               double  x,
               double  y,
               double  z,
               double  red,
               double  green,
               double  blue,
               double  size)
{
  int ix, iy;

  x *= 4;
  y *= 4;
  size *= 4;

  ix = (int) (x - size/2);
  iy = (int) (y - size/2);

  for (iy = (int) (y - size/2); iy < y + size/2 + 1; iy ++)
    {
      for (ix = (int) (x - size/2); ix < x + size/2 + 1; ix ++)
        {
          double alpha =
              (CLAMP (size - ABS (((double) ix) - x), 0.0, 1.0) *
               CLAMP (size - ABS (((double) iy) - y), 0.0, 1.0));

          render_pixel (framebuffer, ix, iy, ROUND (z),
                        red, green, blue, alpha);
        }
    }
}


void
framebuffer_set (double *framebuffer,
                 double  red,
                 double  green,
                 double  blue)
{
  int x, y, z;

  for (x = 0; x < 8; x++)
    {
      for (y = 0; y < 8; y++)
        {
          for (z = 0; z < 8; z++)
            {
              pixel_set (framebuffer, x, y, z, red, green, blue);
            }
        }
    }
}


void
framebuffer_dim (double *framebuffer,
                 double  alpha)
{
  int i;

  for (i = 0; i < 8 * 8 * 8 * 3; i++)
    framebuffer[i] *= alpha;
}


void
framebuffer_merge (double *fb,
                   double *effect1,
                   double *effect2,
                   double  alpha)
{
  int i;

  for (i = 0; i < 8 * 8 * 8 * 3; i++)
    {
      fb[i] = effect1[i] * (1.0 - alpha) + effect2[i] * alpha;
    }
}


