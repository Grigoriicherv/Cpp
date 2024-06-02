//
// Created by grigorii on 04.09.23.
//
// b.cpp

#include

int compare(double* l, double* r) {
  if (*l > *r) return -1;
   else if (*l < *r) return 1;
   else         return 0;
}
void descending(double* beg, double* end) {
   sort(beg, end, &compare);
}

enum class AAA{
   cpp
};
enum class A{
   cpp
};


