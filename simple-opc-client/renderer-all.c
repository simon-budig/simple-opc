#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>

#include <fcntl.h>
#include <poll.h>
#include <linux/joystick.h>

#include "opc-client.h"
#include "render-utils.h"

#include "renderer_astern.h"
#include "renderer_ball.h"
#include "renderer_pong.h"

#define EFFECT_TIME 30.0

static double colors[] = {
  1.0,   0,   0,
    0, 1.0,   0,
    0,   0, 1.0,
  1.0, 1.0,   0,
  1.0,   0, 1.0,
    0, 1.0, 1.0,
};

#define NUM_COLORS 6

typedef void (*RenderFunc) (double *framebufer,
                            double  time,
                            double  joy_x,
                            double  joy_y);


void
mode_jumping_pixels (double *fb,
                     double  t,
                     double  joy_x,
                     double  joy_y)
{
  static double *offsets = NULL;
  int i, x, y;

  if (!offsets)
    {
      offsets = malloc (64 * sizeof (double));
      for (i = 0; i < 64; i++)
        {
          offsets[i] = drand48 () * 7 - 3.5;
        }
    }

  framebuffer_set (fb, 0.0, 0.3, 0.0);

  for (x = 0; x < 8; x++)
    {
      for (y = 0; y < 8; y++)
        {
          double z;

          z = CLAMP (sin (t) * 7 + offsets[x * 8 + y] + 3.5 , 0.0, 6.99);

          render_pixel (fb, x, y, 1 + (int) z,
                        1.0, 1.0, 1.0, z - (int) z);
          render_pixel (fb, x, y, (int) z,
                        1.0, 1.0, 1.0, 1.0 - (z - (int) z));
        }
    }
}


void
mode_lava_balloon (double *fb,
                   double  t,
                   double  joy_x,
                   double  joy_y)
{
  int X, Y;

  framebuffer_set (fb, 0.0, 0.0, 0.4);

  for (X = 0; X < 8; X++)
    {
      for (Y = 0; Y < 8; Y++)
        {
          double x, y, r, z;
          x = (((double) X) / 7.0 - 0.5) * 0.8 *  M_PI;
          y = (((double) Y) / 7.0 - 0.5) * 0.8 * M_PI;
          r = pow (x * x + y * y, 0.5);
          z = sin (r + t) * 0.7 + 0.7;

          render_pixel (fb, X, Y, 1 + (int) z,
                        1.0, 0.0, 0.0, z - (int) z);
          render_pixel (fb, X, Y, (int) z,
                        1.0, 0.0, 0.0, 1.0 - (z - (int) z));
        }
    }

  render_blob (fb,
               0.875, 0.875, fmod (t, 4.0) - 1.0,
               1.0, 1.0, 0.0,
               0.75, 1.0);
}


void
mode_random_blips (double *fb,
                   double  t,
                   double  joy_x,
                   double  joy_y)
{
  int x, y, z, i;
  framebuffer_dim (fb, 0.99);

  for (i = 0; i < 5; i++)
    {
      x = random () % 8;
      y = random () % 8;
      z = random () % 8;

      pixel_set (fb, x, y, z,
                 drand48 (), drand48 (), drand48 ());
    }
}


void
mode_astern (double *framebuffer,
             double  t,
             double  joy_x,
             double  joy_y)
{
  static int wait_counter = -1;  // wait when finished
  static int finished_astern=0;  //
  static int initiated_astern=0;

  if (!initiated_astern)
    {
      initiated_astern = 1;
      init_astern ();
    }
  if (!finished_astern)
    {
      render_map(framebuffer);
      finished_astern = astern_step();
      if (finished_astern < 0)
        {
          // fail, no route found
          wait_counter = 0;
          framebuffer_set(framebuffer, 1.0, 1.0, 1.0);
        }
    }
  else
    {
      if (wait_counter < 0)
        {
          wait_counter = 9;
        }

      if (wait_counter > 0)
        {
          render_path (framebuffer);
          wait_counter--;
        }
      else
        { // == 0
          destruct_astern ();
          initiated_astern = 0;
          finished_astern = 0;
          wait_counter = -1;
        }
    }
}


void
mode_2d_enum (double *fb,
              double  t,
              double  joy_x,
              double  joy_y)
{
  int ti = (int) fmod (t * 20, 512);
  int i;

  for (i = 0; i < ti; i++)
    {
      render_pixel_2d (fb, i % 32, i / 32,
                       1.0, 0.0, 0.0, 1.0);
    }
  for (i = ti; i < 512; i++)
    {
      render_pixel_2d (fb, i % 32, i / 32,
                       0.0, 1.0, 0.0, 1.0);
    }
}


void
mode_2d_circles (double *fb,
                 double  t,
                 double  joy_x,
                 double  joy_y)
{
  static double t0 = 0;
  static int num_circles = 0;
  static double cx[7] = { 0, };
  static double cy[7] = { 0, };
  static double cr[7] = { 0, };
  static int    cc[7] = { 0, };

  int i, j, x, y;

  if (t0 == 0)
    t0 = t;

  if (num_circles == 0 ||
      (num_circles < 7 && (random() % 24) < 1))
    {
      cx[num_circles] = drand48 () * 32;
      cy[num_circles] = drand48 () * 16;
      cr[num_circles] = 0;
      cc[num_circles] = random () % NUM_COLORS;
      num_circles += 1;
    }

  framebuffer_set (fb, 0.3, 0.0, 0.0);

  for (y = 0; y < 16; y++)
    {
      for (x = 0; x < 32; x++)
        {
          for (i = 0; i < num_circles; i++)
            {
              double d = hypot (x - cx[i], y - cy[i]);
              if (fabs (d - cr[i]) < 1.5)
                {
                  render_pixel_2d (fb, x, y,
                                   colors[cc[i]*3 + 0],
                                   colors[cc[i]*3 + 1],
                                   colors[cc[i]*3 + 2],
                                   1.0 - fabs (d - cr[i]) / 1.5);
                }
            }
        }
    }

  j = 0;
  for (i = 0; i < num_circles; i++)
    {
      cx[j] = cx[i];
      cy[j] = cy[i];
      cr[j] = cr[i] + (t - t0);
      cc[j] = cc[i];

      if (cr[j] < 36)
        {
          j ++;
        }
    }
  num_circles = j;

  t0 = t;
}


void
mode_ball_wave (double *fb,
                double  t,
                double  joy_x,
                double  joy_y)
{
  render_ball (t,fb);
}


void
mode_pong (double *fb,
           double  t,
           double  joy_x,
           double  joy_y)
{
  render_pong (t, fb, joy_x, joy_y);
}


void
mode_rect_flip (double *fb,
                double  t,
                double  joy_x,
                double  joy_y)
{
  int x, y, z;
  double dt, sdt, cdt;
  double nx, ny, nz, a;
  int pos;

  pos = (int) (fmod (t, 6.0 * 2.0) / 2.0);
  dt  = fmod (t, 2.0) / 2.0;

  dt = pow (dt, 3);

  dt = dt * M_PI / 2;
  cdt = cos (dt);
  sdt = sin (dt);

  switch (pos)
    {
      case 0:
        nx = + sdt;
        ny = 0;
        nz = - cdt;
        a = 7.0 * (nx + nz);
        break;
      case 1:
        nx = + cdt;
        ny = + sdt;
        nz = 0;
        a = 7.0 * nx;
        break;
      case 2:
        nx = 0;
        ny = + cdt;
        nz = - sdt;
        a = 0.0;
        break;
      case 3:
        nx = - sdt;
        ny = 0;
        nz = + cdt;
        a = 0.0;
        break;
      case 4:
        nx = - cdt;
        ny = - sdt;
        nz = 0;
        a = 7.0 * ny;
        break;
      case 5:
        nx = 0;
        ny = - cdt;
        nz = + sdt;
        a = 7.0 * (ny + nz);
        break;
      default:
        break;
    }

  framebuffer_set (fb, 0.2, 0.0, 0.0);

  for (x = 0; x < 7.99; x++)
    {
      for (y = 0; y < 7.99; y++)
        {
          for (z = 0; z < 7.99; z++)
            {
              double d, len;

              d = fabs (nx * x + ny * y + nz * z - a);

              switch (pos)
                {
                  case 0:
                    len = sqrt ((7.0 - x) * (7.0 - x) + (7.0 - z) * (7.0 - z));
                    break;
                  case 1:
                    len = sqrt ((7.0 - x) * (7.0 - x) + y * y);
                    break;
                  case 2:
                    len = sqrt (y * y + z * z);
                    break;
                  case 3:
                    len = sqrt (x * x + z * z);
                    break;
                  case 4:
                    len = sqrt (x * x + (7.0 - y) * (7.0 - y));
                    break;
                  case 5:
                    len = sqrt ((7.0 - y) * (7.0 - y) + (7.0 - z) * (7.0 - z));
                    break;
                  default:
                    len = 0;
                    break;
                }

              if (d < 0.7)
                {
                  len = 1.0 - CLAMP (len - 7.0, 0.0, 1.0);
                  render_pixel (fb, x, y, z, 1.0, 0.8, 0.0, len * (1.0 - d));
                }
            }
        }
    }
}


int
main (int   argc,
      char *argv[])
{
  double *framebuffer;
  double *effect1, *effect2;
  struct timeval tv;
  OpcClient *client;
  int mode = 0;
  int have_flip = 0;
  int input_fd = -1;
  struct pollfd pfd;
  double joy_x, joy_y;
  double last_js_test = 0;

  RenderFunc modeptrs_nojs[] =
    {
      // mode_astern,
      mode_2d_circles,
      mode_lava_balloon,
      mode_jumping_pixels,
      mode_random_blips,
      mode_rect_flip,
      mode_ball_wave,
    };

  int num_modes_nojs = sizeof (modeptrs_nojs) / sizeof (modeptrs_nojs[0]);

  RenderFunc modeptrs_js[] =
    {
      mode_pong,
    };

  int num_modes_js = sizeof (modeptrs_js) / sizeof (modeptrs_js[0]);

  RenderFunc *modeptrs;
  int num_modes;

  framebuffer = calloc (8 * 8 * 8 * 3, sizeof (double));
  effect1 = calloc (8 * 8 * 8 * 3, sizeof (double));
  effect2 = calloc (8 * 8 * 8 * 3, sizeof (double));

  client = opc_client_new ("127.0.0.1:7890", 7890,
                           8 * 8 * 8 * 3,
                           framebuffer);

  if (!client)
    {
      fprintf (stderr, "can't open client\n");
      exit (1);
    }
  opc_client_connect (client);

  srand48 (time (NULL));

  while (1)
    {
      double t, dt;
      gettimeofday (&tv, NULL);
      t = tv.tv_sec * 1.0 + tv.tv_usec / 1000000.0;

      if (input_fd < 0 && t - last_js_test > 5)
        {
          input_fd = open ("/dev/input/js0", O_RDONLY);

          last_js_test = t;
          pfd.events = POLLIN;
          pfd.fd = input_fd;
        }

      while (input_fd >= 0 && poll (&pfd, 1, 0) > 0)
        {
          struct js_event e;

          if (pfd.revents & POLLIN &&
              read (input_fd, &e, sizeof (e)) > 0)
            {
              switch ((e.type & 0x03) * 256 + e.number)
                {
                  case 0x0203:
                    /* Y */
                    joy_y = CLAMP (((float) e.value) / 23000.0, -1.0, 1.0);
                    break;
                  case 0x0204:
                    /* X */
                    joy_x = - CLAMP (((float) e.value) / 23000.0, -1.0, 1.0);
                    break;
                }
            }
          else
            {
              close (input_fd);
              last_js_test = t;
              input_fd = -1;
            }
        }

      if (input_fd < 0)
        {
          modeptrs = modeptrs_nojs;
          num_modes = num_modes_nojs;
        }
      else
        {
          modeptrs = modeptrs_js;
          num_modes = num_modes_js;
        }

      dt = fmod (t, EFFECT_TIME);

      if (num_modes > 1 && dt < 1.0)
        {
          if (have_flip == 1)
            {
              memset (effect2, 0, sizeof (double) * 8 * 8 * 8 * 3);
              have_flip = 0;
            }

          modeptrs[(mode + 0) % num_modes] (effect1, t, joy_x, joy_y);
          modeptrs[(mode + 1) % num_modes] (effect2, t, joy_x, joy_y);

          framebuffer_merge (framebuffer, effect1, effect2, dt);
        }
      else
        {
          if (have_flip == 0)
            {
              double *tmp;

              tmp = effect1;
              effect1 = effect2;
              effect2 = tmp;
              mode += 1;
              mode %= num_modes;

              have_flip = 1;
            }

          modeptrs[(mode + 0) % num_modes] (effect1, t, joy_x, joy_y);
          framebuffer_merge (framebuffer, effect1, effect2, 0.0);
        }

      opc_client_write (client, 0, 0);
      usleep (50 * 1000);  /* 50ms */
    }

  opc_client_shutdown (client);

  return 0;
}

