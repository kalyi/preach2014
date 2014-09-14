/*
 * Copyright (c) Florian Merz 2013.
 *
 * The software is provided on an as is basis for research purposes.
 * There is no additional support offered, nor are the author(s)
 * or their institutions liable under any circumstances.
 *
 */

#include <algorithm>
#include "ConstructionGraph.hh"

namespace preach {

bool LexLess(const int& i, const int& j) {
  return std::to_string(i) < std::to_string(j);
}

void strTrimRight(std::string& str) {
  std::string whitespaces(" \t");
  int index = str.find_last_not_of(whitespaces);
  if (index != std::string::npos) {
    str.erase(index+1);
  } else {
    str.clear();
  }
}

inline void ConstructionGraph::addEdge(const int sid, const int tid) {
  int back_fwd = bwd_edge_l[tid].size();
  int back_bwd = fwd_edge_l[sid].size();
  fwd_edge_l[sid].push_back(EdgeCH(tid, back_fwd, E_NONE));
  bwd_edge_l[tid].push_back(EdgeCH(sid, back_bwd, E_NONE));
}

void ConstructionGraph::read(std::istream& in) {
  std::string line;
  std::stringstream ss;
  std::getline(in, line);
  std::getline(in, line);
  ss << line;
  ss >> n;

  num_sinks = n;
  num_sources = n;

  node_chl = NodeCHList(n);

  fwd_edge_l = EdgeCHList(n);
  bwd_edge_l = EdgeCHList(n);
  m = 0;
  for (int i = 0; i < n; i++) {
    std::stringstream ss;
    int sid, tid;
    char c;
    std::getline(in, line);
    ss << line;

    ss >> sid;
    sid = i; // stupid bug in go-uniprot
    ss >> c;
    while(ss >> tid) {
      if (node_chl[sid].out == 0) {
        num_sinks--;
      }
      node_chl[sid].out++;
      if (node_chl[tid].in == 0) {
        num_sources--;
      }
      node_chl[tid].in++;
      addEdge(sid, tid);
      m++;
    }
  }
  qg = std::unique_ptr<QueryGraph>(new QueryGraph(n));
}

int ConstructionGraph::levelFwdDfs(const int vid, int tree_size) {
  int level = qg->nodes[vid].fwd_level;
  NodeCH *tb;
  for (auto& e: fwd_edge_l[vid]) {
    if (qg->nodes[e.to].fwd_level <= level) {
      qg->nodes[e.to].fwd_level = level + 1;
    }
  }
  for (auto& e: fwd_edge_l[vid]) {
    tb = &node_chl[e.to];
    tb->visited_in++;
    if (tb->in == tb->visited_in) {
      tree_size = levelFwdDfs(e.to, tree_size + 1);
    }
  }
  return tree_size;
}

int ConstructionGraph::levelBwdDfs(const int vid, int tree_size) {
  int level = qg->nodes[vid].bwd_level;
  NodeCH *tb;
  for (auto& e: bwd_edge_l[vid]) {
    if (qg->nodes[e.to].bwd_level <= level) {
      qg->nodes[e.to].bwd_level = level + 1;
    }
  }
  for (auto& e: bwd_edge_l[vid]) {
    tb = &node_chl[e.to];
    tb->visited_out++;
    if (tb->out == tb->visited_out) {
      tree_size = levelBwdDfs(e.to, tree_size + 1);
    }
  }
  return tree_size;
}

inline void dfsCheckRecurse(Ranges *r, Ranges *t, int &gap) {
  if (t->phi < r->phi) {
    if ((t->phi_hat - t->phi) > gap) {
      gap = t->phi_hat - t->phi;
      r->phi_ptree = t->phi;
      r->phi_hat_ptree = t->phi_hat;
    }
    if (t->phi_hat > r->phi_gap) {
      r->phi_gap = t->phi_hat;
    }
  }
  if (t->phi_ptree < r->phi) {
    if ((t->phi_hat_ptree - t->phi_ptree) > gap) {
      gap = t->phi_hat_ptree - t->phi_ptree;
      r->phi_ptree = t->phi_ptree;
      r->phi_hat_ptree = t->phi_hat_ptree;
    }
  }
  if (t->phi_gap > r->phi_gap) {
    r->phi_gap = t->phi_gap;
  }
  if ((t->phi_min > -1) && (t->phi_min < r-> phi_min)) {
    r->phi_min = t->phi_min;
  }
}

int ConstructionGraph::rangesFwdDfs(const int vid, int phi) {
  Ranges *r = &qg->nodes[vid].fwd_ranges;

  int gap = 1;
  r->phi = phi;
  phi++;

  r->phi_ptree = r->phi;
  r->phi_min = r->phi;
  r->phi_hat_ptree = -2;
  Ranges *t;
  for (auto& e: fwd_edge_l[vid]) {
    t = &qg->nodes[e.to].fwd_ranges;
    if (t->phi_hat_ptree == -1) {
      phi = rangesFwdDfs(e.to, phi);
    }
    dfsCheckRecurse(r, t, gap);
  }
  if (r->phi_hat_ptree == -2) {
    r->phi_hat_ptree = phi;
  }
  r->phi_hat = phi;
  return phi;
}

int ConstructionGraph::rangesBwdDfs(const int vid, int phi) {
  Ranges *r = &qg->nodes[vid].bwd_ranges;

  int gap = 1;
  r->phi = phi;
  phi++;

  r->phi_ptree = r->phi;
  r->phi_min = r->phi;
  r->phi_hat_ptree = -2;
  Ranges *t;
  for (auto& e: bwd_edge_l[vid]) {
    t = &qg->nodes[e.to].bwd_ranges;
    if (t->phi_hat_ptree == -1) {
      phi = rangesBwdDfs(e.to, phi);
    }
    dfsCheckRecurse(r, t, gap);
  }
  if (r->phi_hat_ptree == -2) {
    r->phi_hat_ptree = phi;
  }
  r->phi_hat = phi;
  return phi;
}

inline int ConstructionGraph::prio(const int vid) {
  int pri = n;
  if (node_chl[vid].in == 0 || node_chl[vid].out == 0) {
    pri = node_chl[vid].deg;
  }
  return pri;
}

inline void ConstructionGraph::contractSource(const int vid) {
  NodeCH *t;
  for (auto &e: fwd_edge_l[vid]) {
    if (e.direction != E_NONE) {
      continue;
    }
    t = &node_chl[e.to];
    node_chl[vid].rch_deg++;
    e.direction = E_FWD;
    fwd_m++;
    t->in--;
    bwd_edge_l[e.to][e.rev_index].direction = E_FWD;
    
    int pri = prio(e.to);
    if ((pri < n) && !node_chl[e.to].con) {
      pq.emplace(pri, e.to);
      node_chl[e.to].con = true;
    }
  }
}

inline void ConstructionGraph::contractSink(const int vid) {
  NodeCH *t;
  for (auto &e: bwd_edge_l[vid]) {
    if (e.direction != E_NONE) {
      continue;
    }
    t = &node_chl[e.to];
    node_chl[vid].rch_deg++;
    e.direction = E_BWD;
    bwd_m++;
    t->out--;
    fwd_edge_l[e.to][e.rev_index].direction = E_BWD;
    
    int pri = prio(e.to);
    if ((pri < n) && !node_chl[e.to].con) {
      pq.emplace(pri, e.to);
      node_chl[e.to].con = true;
    }
  }
}

void ConstructionGraph::rCH() {
  for (int i = 0; i < n; ++i) {
    int pri = prio(i);
    if (pri < n) {
      pq.emplace(pri, i);
      node_chl[i].con = true;
    }
  }
  while(!pq.empty()) {
    int vid = pq.top().val;
    pq.pop();
    if (node_chl[vid].in == 0) {
      contractSource(vid);
    } else if (node_chl[vid].out == 0) {
      contractSink(vid);
    }
  }
}

void ConstructionGraph::divideEdges() {
  qg->setM(fwd_m, bwd_m);
  int *it = &qg->fwd_edges[0];
  for (int vid = 0; vid < n; ++vid) {
    qg->fwd_edge_slices[vid].head = it;
    for (EdgeCH &e: fwd_edge_l[vid]) {
      if (e.direction == E_FWD) {
        *it = e.to;
        it++;
      }
    }
    qg->fwd_edge_slices[vid].tail = it;
  }

  it = &qg->bwd_edges[0];
  for (int vid = 0; vid < n; ++vid) {
    qg->bwd_edge_slices[vid].head = it;
    for (EdgeCH &e: bwd_edge_l[vid]) {
      if (e.direction == E_BWD) {
        *it = e.to;
        it++;
      }
    }
    qg->bwd_edge_slices[vid].tail = it;
  }
}

std::unique_ptr<QueryGraph> ConstructionGraph::construct() {
  std::vector<Pair<int> > l_fwd;
  std::vector<Pair<int> > l_bwd;

  int tree_size = 0;
  int id = 0;
  for (auto &v: node_chl) {
    if (v.in == 0) {
      tree_size = levelFwdDfs(id, 0);
      l_fwd.push_back(Pair<int>(n - tree_size, id));
    }
    if (v.out == 0) {
      tree_size = levelBwdDfs(id, 0);
      l_bwd.push_back(Pair<int>(n - tree_size, id));
    }
    id++;
  }
  std::sort(l_fwd.begin(), l_fwd.end());
  std::sort(l_bwd.begin(), l_bwd.end());

  int phi = 0;
  for (auto& p: l_fwd) {
    phi = rangesFwdDfs(p.val, phi);
  }
  phi = 0;
  for (auto& p: l_bwd) {
    phi = rangesBwdDfs(p.val, phi);
  }
  rCH();
  divideEdges();
  
  return std::move(qg);
}

} // ns preach
