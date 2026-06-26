#ifndef _MST2_H_
#define _MST2_H_

#include "global.h"

namespace fastSteiner {

void mst2_package_init(long n);
void mst2_package_done();
void mst2(long n, Point* pt, long* parent);
void reduced_mst2(long n_terminals, long* n_points, Point* pt,
                  Point* sorted_terms, long* parent);

bool std_compare_points(const Point& p1, const Point& p2);

}  // namespace fastSteiner

#endif
