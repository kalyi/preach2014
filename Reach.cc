/*
 * Copyright (c) Florian Merz 2013.
 *
 * The software is provided on an as is basis for research purposes.
 * There is no additional support offered, nor are the author(s)
 * or their institutions liable under any circumstances.
 */

#include "Graph.hh"
#include "Reach.hh"
#include "util.hh"
#include <limits.h>

#define Q_NONE 0
#define Q_FWD_V 1
#define Q_BWD_V 2

#define BFS_FWD_TOGGLE 3
#define BFS_BWD_TOGGLE 2
#define BFS_FWD_ONLY   1
#define BFS_BWD_ONLY   0

namespace preach {
#include "spq/knheap.C"


  int Reach::NodePriority(const int i) {
    NodeContract_t node = g->ncl[i];
    int pri;
    if ((node.in == 0) || (node.out == 0)) {
      pri = node.changed + node.in + node.out;
    } else {
      pri += g->num_nodes;
    }
    return pri;
  }

  void Reach::InitQueue() {
    pq = new KNHeap<int,int>(INT_MAX,-INT_MAX);
    for (int i = 0; i < g->num_nodes; i++) {
      int prio = NodePriority(i);
      if (prio < g->num_nodes) {
        pq->insert(prio, i);
        g->ncl[i].con = 1;
      }
    }
  }

  void Reach::ContractSink(const int vid) {
    NodeContract_t *t;
    int prio;
    for (EdgeBuild_t& e: g->ebl_bwd[vid]) {
      if ((e.subs & E_E_) == 0) {
        continue;
      }
      t = &g->ncl[e.to];
      e.subs |= E_ECHBWD;
      t->out--;
      t->changed++;

      g->ebl_fwd[e.to][e.back].subs &= ~E_E;
      e.subs &= ~E_E_;

      prio = NodePriority(e.to);
      if (prio < g->num_nodes) {
        if (g->ncl[e.to].con > 0) {
          continue;
        }
        g->ncl[e.to].con = 1;
        pq->insert(prio, e.to);
      }
    }
  }

  void Reach::ContractSource(int vid) {
    NodeContract_t *t;
    int prio;
    for (EdgeBuild_t& e: g->ebl_fwd[vid]) {
      if ((e.subs & E_E) == 0) {
        continue;
      }
      t = &g->ncl[e.to];
      e.subs |= E_ECHFWD;
      t->in--;
      t->changed++;

      g->ebl_bwd[e.to][e.back].subs &= ~E_E_;
      e.subs &= ~E_E;

      prio = NodePriority(e.to);
      if (prio < g->num_nodes) {
        if (g->ncl[e.to].con > 0) {
          continue;
        }
        g->ncl[e.to].con = 1;
        pq->insert(prio, e.to);
      }
    }
  }

  void Reach::Contract(const int vid) {
    if (g->ncl[vid].in > 0) {
      ContractSink(vid);
    } else if (g->ncl[vid].out > 0) {
      ContractSource(vid);
    }
  }

  void Reach::MakeStatic() {
    g->el_fwd = (EdgeArray_t *)malloc(g->num_nodes * sizeof(EdgeArray_t));
    g->el_bwd = (EdgeArray_t *)malloc(g->num_nodes * sizeof(EdgeArray_t));
    g->MakeEdgeList(E_ECHFWD, g->el_fwd);
    g->MakeEdgeList(E_ECHBWD, g->el_bwd);
  }

  uint16_t Reach::Levels() {
    uint16_t max = 0;
    for (int i = 0; i < g->num_nodes; i++) {
      if (g->nl[i].level > max) {
        max = g->nl[i].level;
      }
    }
    return max + 1;
  }

  void Reach::BfsBwd(const int vid) {
    count++;

    if (runs[vid] == run + Q_FWD_V) {
      meet = vid;
      return;
    }

    int e;
    Node_t *en;
    EdgeArray_t *a = &g->el_bwd[vid];
    for (int i = 0;i < a->len; i++) {
      e = a->a[i];
      if (runs[e] == run + Q_BWD_V) {
        continue;
      }
      if (runs[e] == run + Q_FWD_V) {
        meet = e;
        return;
      }
      en = &g->nl[e];
      if (en->level_back >= fwd_lvl_back) {
        continue;
      }
      if (en->level <= fwd_lvl) {
        continue;
      }
      if (order_bwd >= en->till_bwd) {
        continue;
      } else {
        if (order_bwd < en->peek_min_bwd) {
          continue;
        }
        if ((order_bwd >= en->peek_bwd) &&
            ((order_bwd > en->order_bwd) ||
             (order_bwd < en->peek_till_bwd))) {
          meet = src;
          return;
        }
      }
      if ((order_bwd < en->order_bwd) && (order_bwd >= en->peek_max_bwd)) {
        continue;
      }
#ifdef DOUBLECHECK
      if (en->order_fwd >= g->nl[src].till_fwd) {
        continue;
      } else {
        if (en->order_fwd < g->nl[src].peek_min_fwd) {
          continue;
        }
        if ((en->order_fwd >= g->nl[src].peek_fwd) &&
            ((en->order_fwd > g->nl[src].order_fwd) ||
             (en->order_fwd < g->nl[src].peek_till_fwd))) {
          meet = src;
          return;
        }
      }
      if ((en->order_fwd < g->nl[src].order_fwd) && (en->order_fwd >= g->nl[src].peek_max_fwd)) {
        continue;
      }
#endif
      runs[e] = run + Q_BWD_V;
      ff->PushBwd(e);
    }
  }

  void Reach::BfsFwd(const int vid) {
    count++;

    if (runs[vid] == run + Q_BWD_V) {
      meet = vid;
      return;
    }

    int e;
    Node_t *en;
    EdgeArray_t *a = &g->el_fwd[vid];
    for (int i = 0;i < a->len; i++) {
      e = a->a[i];
      if (runs[e] == run + Q_FWD_V) {
        continue;
      }
      if (runs[e] == run + Q_BWD_V) {
        meet = e;
        return;
      }
      en = &g->nl[e];
      if (en->level_back <= bwd_lvl_back) {
        continue;
      }
      if (en->level >= bwd_lvl) {
        continue;
      }
      if (order_fwd >= en->till_fwd) {
        continue;
      } else {
        if (order_fwd < en->peek_min_fwd) {
          continue;
        }
        if ((order_fwd >= en->peek_fwd) &&
            ((order_fwd > en->order_fwd) ||
             (order_fwd < en->peek_till_fwd))) {
          meet = tgt;
          return;
        }
      }
      if ((order_fwd < en->order_fwd) && (order_fwd >= en->peek_max_fwd)) {
        continue;
      }
#ifdef DOUBLECHECK
      if (en->order_bwd >= g->nl[tgt].till_bwd) {
        continue;
      } else {
        if (en->order_bwd < g->nl[tgt].peek_min_bwd) {
          continue;
        }
        if ((en->order_bwd >= g->nl[tgt].peek_bwd) &&
            ((en->order_bwd > g->nl[tgt].order_bwd) ||
             (en->order_bwd < g->nl[tgt].peek_till_bwd))) {
          meet = tgt;
          return;
        }
      }
      if ((en->order_bwd < g->nl[tgt].order_bwd) && (en->order_bwd >= g->nl[tgt].peek_max_bwd)) {
        continue;
      }
#endif
      runs[e] = run + Q_FWD_V;
      ff->PushFwd(e);
    }
  }

  int Reach::Query(std::vector<Query_t>  *queries) {
    int reach = 0;
    count = 0;
    comp = 0;
    for (Query_t& q: *queries) {
      src = q.s;
      tgt = q.t;


      if (src == tgt) {
        q.r = 1;
        reach++;
        continue;
      }
      if ((g->nl[src].level_back <= g->nl[tgt].level_back) ||
          (g->nl[src].level >= g->nl[tgt].level)) {
        q.r = 0;
        continue;
      }
      if (g->nl[tgt].order_fwd < g->nl[src].peek_min_fwd) {
        q.r = 0;
        continue;
      }
      if ((g->nl[tgt].order_fwd <  g->nl[src].till_fwd) &&
          (g->nl[tgt].order_fwd >= g->nl[src].peek_fwd) &&
          ((g->nl[tgt].order_fwd > g->nl[src].order_fwd) ||
           (g->nl[tgt].order_fwd < g->nl[src].peek_till_fwd))) {
        q.r = 1;
        reach++;
        continue;
      }
      if (g->nl[src].order_bwd < g->nl[tgt].peek_min_bwd) {
        q.r = 0;
        continue;
      }
      if ((g->nl[src].order_bwd <  g->nl[tgt].till_bwd) &&
          (g->nl[src].order_bwd >= g->nl[tgt].peek_bwd) &&
          ((g->nl[src].order_bwd > g->nl[tgt].order_bwd) ||
           (g->nl[src].order_bwd < g->nl[tgt].peek_till_bwd))) {
        q.r = 1;
        reach++;
        continue;
      }
      if ((g->nl[src].order_bwd < g->nl[tgt].order_bwd) &&
          (g->nl[src].order_bwd >= g->nl[tgt].peek_max_bwd)) {
        q.r = 0;
        continue;
      }
      if ((g->nl[tgt].order_fwd < g->nl[src].order_fwd) &&
          (g->nl[tgt].order_fwd >= g->nl[src].peek_max_fwd)) {
        q.r = 0;
        continue;
      }
      fwd_lvl = g->nl[src].level;
      fwd_lvl_back = g->nl[src].level_back;
      order_bwd = g->nl[src].order_bwd;
      order_fwd = g->nl[tgt].order_fwd;
      bwd_lvl = g->nl[tgt].level;
      bwd_lvl_back = g->nl[tgt].level_back;
      meet = -1;
      v_fwd = src;
      v_bwd = tgt;

      runs[src] = run + Q_FWD_V;
      runs[tgt] = run + Q_BWD_V;
      ff->Clear();

      forward = true;

      int state = BFS_FWD_TOGGLE;
      while ((v_fwd > -1 || v_bwd > -1) && (meet == -1)) {
        switch (state) {
        case BFS_FWD_TOGGLE:
          BfsFwd(v_fwd);
          v_fwd = ff->PopFwd();
          state = (v_bwd == -1) ? BFS_FWD_ONLY : BFS_BWD_TOGGLE;
          break;
        case BFS_BWD_TOGGLE:
          BfsBwd(v_bwd);
          v_bwd = ff->PopBwd();
          state = (v_fwd == -1) ? BFS_BWD_ONLY : BFS_FWD_TOGGLE;
          break;
        case BFS_FWD_ONLY:
          BfsFwd(v_fwd);
          v_fwd = ff->PopFwd();
          break;
        case BFS_BWD_ONLY:
          BfsBwd(v_bwd);
          v_bwd = ff->PopBwd();
          break;
        default:
          break;
        }
      }
      run += 3;
      q.r = meet > -1 ? 1 : 0;
      reach += q.r;
      comp++;
    }
    return reach;
  }

  int Reach::BfsCount() {
    return count;
  }

  int Reach::CompleteRuns() {
    return comp;
  }

  Reach::Reach() {

  }

  Reach::Reach(Graph *graph) {
    g = graph;
    TPRECALL()
      g->SetLevels();
    TPOSTCALL("SetLevels")
      InitQueue();
    TPOSTCALL("InitQueue")
      int key, val;
    while (pq->getSize() > 1) {
      pq->deleteMin(&key, &val);
      Contract(val);
    }
    TPOSTCALL("Contraction")
      MakeStatic();
    TPOSTCALL("Conversion")
      ff = new Fifo(g->num_nodes);
    run = 0;
    runs = new uint64_t[g->num_nodes];
    TPOSTCALL("Runs and Fifo")

      //std::cout << "size fwd: " << g->size_fwd << " size bwd: " << g->size_bwd << std::endl;

      }

  Reach::~Reach() {
    delete pq;
    delete ff;
  }

}
