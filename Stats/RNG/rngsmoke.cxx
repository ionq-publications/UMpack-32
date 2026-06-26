#define WANT_STREAM

#include <cmath>
#include <cstdlib>

#include "rng_include.h"
#include "newran.h"

#ifdef use_namespace
using namespace NEWRAN;
#endif

namespace {

void require(bool cond, const char* msg) {
  if (!cond) {
    std::cerr << "rngsmoke failed: " << msg << std::endl;
    std::exit(1);
  }
}

bool closeEnough(Real a, Real b, Real eps = 1e-12) {
  return std::fabs(a - b) <= eps;
}

}  // namespace

int main() {
  Random::Set(0.46875);
  Uniform u1;
  Real u1a = u1.Next();
  Real u1b = u1.Next();
  require(u1a >= 0.0 && u1a <= 1.0, "uniform range 1");
  require(u1b >= 0.0 && u1b <= 1.0, "uniform range 2");

  Random::Set(0.46875);
  Uniform u2;
  require(closeEnough(u1a, u2.Next()), "uniform reproducibility");
  require(closeEnough(u1b, u2.Next()), "uniform reproducibility 2");

  Constant c(5.0);
  require(closeEnough(c.Next(), 5.0), "constant generator");

  Random::Set(0.46875);
  Normal n1;
  Real n1a = n1.Next();
  Real n1b = n1.Next();
  Random::Set(0.46875);
  Normal n2;
  require(closeEnough(n1a, n2.Next(), 1e-10), "normal reproducibility");
  require(closeEnough(n1b, n2.Next(), 1e-10), "normal reproducibility 2");

  static Real probs[] = {0.1, 0.2, 0.3, 0.4};
  DiscreteGen dg(4, probs);
  require(closeEnough(dg.Mean().Value(), 2.0), "discrete mean");

  Real sample = dg.Next();
  require(sample >= 0.0 && sample <= 3.0, "discrete range");

  Poisson p(4.0);
  require(closeEnough(p.Mean().Value(), 4.0), "poisson mean");

  Binomial b(10, 0.25);
  require(closeEnough(b.Mean().Value(), 2.5, 1e-9), "binomial mean");

  return 0;
}
