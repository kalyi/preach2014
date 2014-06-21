/*
 * Copyright (c) Florian Merz 2013.
 *
 * The software is provided on an as is basis for research purposes.
 * There is no additional support offered, nor are the author(s)
 * or their institutions liable under any circumstances.
 */

#ifndef FIFO_H
#define FIFO_H

#include <vector>

namespace preach {

  class Fifo {
  private:
    int head_fwd;
    int tail_fwd;
    int head_bwd;
    int tail_bwd;
    int size;
    int *ff;
  public:
    void PushBwd(int);
    int PopBackBwd();
    int PopBwd();
    bool EmptyBwd();
    int SizeBwd();
    void PushFwd(int);
    int PopBackFwd();
    int PopFwd();
    bool EmptyFwd();
    int SizeFwd();
    void Clear();
    Fifo();
    explicit Fifo(int);
    ~Fifo();
  };

}
#endif // FIFO_H
