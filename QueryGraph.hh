/*
 * Copyright (c) Florian Merz 2013.
 *
 * The software is provided on an as is basis for research purposes.
 * There is no additional support offered, nor are the author(s)
 * or their institutions liable under any circumstances.
 *
 */

#ifndef PREACH_QUERYGRAPH_HH_
#define PREACH_QUERYGRAPH_HH_
#include <valarray>
#include "Fifo.hh"

namespace preach {

struct Query {
  int s;
  int t;
  int r;
};

struct Ranges {
  int phi{-1};
  int phi_hat{-1};
  int phi_min{-1};
  int phi_ptree{-1};
  int phi_hat_ptree{-1};
  int phi_gap{-1};
};

struct Node {
  int fwd_level{0};
  int bwd_level{0};
  Ranges fwd_ranges;
  Ranges bwd_ranges;
};

struct Slice {
  const int* head;
  const int* tail;
  const int *begin() { return head; }
  const int *end() { return tail; }
}; 

using EdgeSlices = std::valarray<Slice>;
using EdgeList = std::valarray<int>;
using NodeList = std::valarray<Node>;

class QueryGraph {
 public:
  NodeList nodes;
  Fifo ff;
  std::valarray<int> runs;
 private:
  inline int bfsFwd(const int uid, const Node &t);
  inline int bfsBwd(const int uid, const Node &s);
  int run;
 public:
  EdgeSlices fwd_edge_slices;
  EdgeSlices bwd_edge_slices;
  EdgeList fwd_edges;
  EdgeList bwd_edges;
  QueryGraph(int n) : nodes(n), ff(n), runs(n) {};
  void setM(int fwd_m, int bwd_m);
  int query(std::vector<Query> &queries);
}; 

} // ns preach
#endif // PREACH_QUERYGRAPH_HH_
