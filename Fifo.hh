/*
 * Copyright (c) Florian Merz 2013.
 *
 * The software is provided on an as is basis for research purposes.
 * There is no additional support offered, nor are the author(s)
 * or their institutions liable under any circumstances.
 */

#ifndef FIFO_H
#define FIFO_H

#include <valarray>

namespace preach {

class Fifo {
 private:
  std::valarray<int> ff;
  int head_fwd;
  int tail_fwd;
  int head_bwd;
  int tail_bwd;
 public:
  void PushBwd(int);
  int PopBwd();
  bool EmptyBwd();
  int SizeBwd();
  void PushFwd(int);
  int PopFwd();
  bool EmptyFwd();
  int SizeFwd();
  void Clear();
  Fifo();
  explicit Fifo(int);
};

}
#endif // FIFO_H
