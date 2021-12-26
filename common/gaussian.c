/*
 *  generate random draws from a N(0, 1) distribution using the box-mueller transform
 */

#include <stdlib.h>
#include <math.h>

#include "gaussian.h"

// constants
static const double twopi = 2.0 * M_PI;

// statics
static double R = 0.0;
static double T = 0.0;
static unsigned int state = 0;

double gengauss(void) {
     double res = 0.0;
     if (state == 0) {
          double u1 = 0.0;
          double u2 = 0.0;
          do {
               u1 = (double)rand() / (double)RAND_MAX;
               u2 = (double)rand() / (double)RAND_MAX;
          } while (u1 == 0.0);

          R = sqrt(-2.0 * log(u1));
          T = twopi * u2;
          res = R * cos(T);

          state = 1;
     } else {
          res = R * sin(T);
          state = 0;
     }
     return res;
}
