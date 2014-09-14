/*
 * Copyright (c) Florian Merz 2013.
 *
 * The software is provided on an as is basis for research purposes.
 * There is no additional support offered, nor are the author(s)
 * or their institutions liable under any circumstances.
 *
 */

#ifndef PREACH_CONSTRUCTIONGRAPH_HH_
#define PREACH_CONSTRUCTIONGRAPH_HH_

#include <vector>
#include <queue>
#include <valarray>
#include <memory>
#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <sstream>

#include "QueryGraph.hh"

#define E_NONE 0
#define E_FWD 1
#define E_BWD 2

namespace preach {

template <class Valtype> struct Pair {
  int key;
  Valtype val;
  Pair(int k, Valtype v) : key(k), val(v) {};
  bool operator<(const Pair &other) const {
    return key < other.key;
  }
};

struct NodeCH {
  int in{0};
  int visited_in{0};
  int out{0};
  int visited_out{0};
  int deg{0};
  int rch_deg{0};
  bool con{false};
};

struct EdgeCH {
  int to;
  int rev_index;
  uint8_t direction;
  EdgeCH(int t, int r, uint8_t d) 
      : to(t), rev_index(r), direction(d) {};
};

using NodeCHList = std::valarray<NodeCH>;
using EdgeCHList = std::valarray<std::vector<EdgeCH> >;

class ConstructionGraph {
 private:
  std::priority_queue<Pair<int> > pq;
  inline void addEdge(const int sid, const int tid);
  int levelFwdDfs(const int vid, int tree_size);
  int levelBwdDfs(const int vid, int tree_size);
  int rangesFwdDfs(const int vid, int phi);
  int rangesBwdDfs(const int vid, int phi);
  void contractSource(const int vid);
  void contractSink(const int vid);
  void rCH();
  void divideEdges();
  inline int prio(const int vid);
 public:
  std::unique_ptr<QueryGraph> qg;
  int n;
  int m;
  NodeCHList node_chl;
  EdgeCHList fwd_edge_l;
  EdgeCHList bwd_edge_l;
  int fwd_m{0};
  int bwd_m{0};
  int num_sinks;
  int num_sources;

  void read(std::istream&);
  std::unique_ptr<QueryGraph> construct();
};

} // ns preach
#endif // PREACH_CONSTRUCTIONGRAPH_HH_
