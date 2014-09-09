#ifndef UTIL_H
#define UTIL_H

#include <sys/time.h>
#include <chrono>
#define TTIME(a) a = std::chrono::high_resolution_clock::now()
#define TDIFF(a, b)     std::chrono::duration_cast<std::chrono::nanoseconds>(b-a)

#define TPRECALL()
#define TPOSTCALL(label)

#endif
