/*
 * Copyright (c) Florian Merz 2013.
 *
 * The software is provided on an as is basis for research purposes.
 * There is no additional support offered, nor are the author(s)
 * or their institutions liable under any circumstances.
 */

#include "Fifo.hh"
#include <stdlib.h>

namespace preach {
  Fifo::Fifo(int n) {
    size = n;
    //ff = (int*)malloc(n * sizeof(int));
    ff = new int[size];
    head_fwd = 0;
    tail_fwd = 0;
    head_bwd = size - 1;
    tail_bwd = size - 1;
  }

  Fifo::Fifo() {
    ff = NULL;
  }

  void Fifo::PushBwd(int elem) {
    ff[tail_bwd--] = elem;
  }

  int Fifo::PopBackBwd() {
    if (head_bwd == tail_bwd) {
      return -1;
    } else {
      return ff[tail_bwd++];
    }
  }

  int Fifo::PopBwd() {
    if (head_bwd == tail_bwd) {
      return -1;
    } else {
      return ff[head_bwd--];
    }
  }

  bool Fifo::EmptyBwd() {
    return head_bwd == tail_bwd;
  }

  int Fifo::SizeBwd() {
    return head_fwd - tail_bwd;
  }

  void Fifo::PushFwd(int elem) {
    ff[tail_fwd++] = elem;
  }

  int Fifo::PopBackFwd() {
    if (head_fwd == tail_fwd) {
      return -1;
    } else {
      return ff[tail_fwd--];
    }
  }

  int Fifo::PopFwd() {
    if (head_fwd == tail_fwd) {
      return -1;
    } else {
      return ff[head_fwd++];
    }
  }

  bool Fifo::EmptyFwd() {
    return head_fwd == tail_fwd;
  }

  int Fifo::SizeFwd() {
    return tail_fwd - head_fwd;
  }


  void Fifo::Clear() {
    head_fwd = 0;
    tail_fwd = 0;
    head_bwd = size - 1;
    tail_bwd = size - 1;
  }

  Fifo::~Fifo() {
    /*
      if (ff != NULL) {
      free(ff);
      }
    */
  }
}
