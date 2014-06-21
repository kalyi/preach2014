/*
 * Copyright (c) Florian Merz 2013.
 *
 * The software is provided on an as is basis for research purposes.
 * There is no additional support offered, nor are the author(s)
 * or their institutions liable under any circumstances.
 */

#ifndef REACH_H
#define REACH_H

#include "Graph.hh"
#include "Fifo.hh"

namespace preach {
#include "spq/knheap.h"

  typedef struct Query {
    int s;
    int t;
    int r;
  } Query_t;

  class Reach {
  private:
    Graph *g;
    Fifo *ff;
    uint64_t run;
    uint64_t con_run;
    int meet;
    bool forward;
    uint16_t fwd_lvl_back;
    uint16_t bwd_lvl_back;
    uint16_t fwd_lvl;
    uint16_t bwd_lvl;
    int order_fwd;
    int order_bwd;
    KNHeap<int,int> *pq;
    int src;
    int tgt;
    int v_fwd;
    int v_bwd;
    Node_t *src_node;
    Node_t *tgt_node;
    uint64_t *runs;

    int comp;
    int count;

    int NodePriority(int);
    void InitQueue();
    void ContractSink(int);
    void ContractSource(int);
    void Contract(int);
    void MakeStatic();
    void BfsBwd(int);
    void BfsFwd(int);
  public:

    uint16_t Levels();
    int Query(std::vector<Query_t> *);
    explicit Reach(Graph*);
    int BfsCount();
    int CompleteRuns();
    Reach();
    ~Reach();

  };
}

#endif // REACH_H
