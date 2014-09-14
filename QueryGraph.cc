/*
 * Copyright (c) Florian Merz 2013.
 *
 * The software is provided on an as is basis for research purposes.
 * There is no additional support offered, nor are the author(s)
 * or their institutions liable under any circumstances.
 *
 */

#include "QueryGraph.hh"

#define Q_NONE 0
#define Q_FWD_V 1
#define Q_BWD_V 2

#define BFS_FWD_TOGGLE 3
#define BFS_BWD_TOGGLE 2
#define BFS_FWD_ONLY   1
#define BFS_BWD_ONLY   0

namespace preach {

inline bool check_level_prune(const Node &u, const Node &v) {
  return ((u.fwd_level >= v.fwd_level) || (u.bwd_level <= v.bwd_level));
}

inline bool check_phi_minhat_prune(const Ranges &u, const Ranges &v) {
  return ((v.phi < u.phi_min) || (v.phi >= u.phi_hat));
}

inline bool check_phi_hat_shortcut(const Ranges &u, const Ranges &v) {
  return ((v.phi >= u.phi) && (v.phi < u.phi_hat));
}

inline bool check_phi_ptree_shortcut(const Ranges &u, const Ranges &v) {
  return ((v.phi >= u.phi_ptree) && (v.phi < u.phi_hat_ptree));
}

inline bool check_phi_gap_prune(const Ranges &u, const Ranges &v) {
  return ((v.phi < u.phi) && (v.phi >= u.phi_gap));
}

void QueryGraph::setM(const int fwd_m, const int bwd_m) {
  fwd_edge_slices = EdgeSlices(nodes.size());
  bwd_edge_slices = EdgeSlices(nodes.size());
  fwd_edges = EdgeList(fwd_m);
  bwd_edges = EdgeList(bwd_m);
}

inline int QueryGraph::bfsFwd(const int uid, const Node &t) {
  if (runs[uid] == run + Q_BWD_V) {
    return uid;
  }
  for (int e: fwd_edge_slices[uid]) {
    if (runs[e] == run + Q_FWD_V) {
      continue;
    }
    if (runs[e] == run + Q_BWD_V) {
      return e;
    }
    
    Node &v = nodes[e];

    if (check_level_prune(v, t)) {
      continue;
    }
    
    if (check_phi_minhat_prune(v.fwd_ranges, t.fwd_ranges)) {
      continue;
    }
    if (check_phi_hat_shortcut(v.fwd_ranges, t.fwd_ranges)) {
      return uid;
    }
    if (check_phi_ptree_shortcut(v.fwd_ranges, t.fwd_ranges)) {
      return uid;
    }
    if (check_phi_minhat_prune(v.fwd_ranges, t.fwd_ranges)) {
      continue;
    }
    if (check_phi_minhat_prune(t.bwd_ranges, v.bwd_ranges)) {
      continue;
    }
    if (check_phi_hat_shortcut(t.bwd_ranges, v.bwd_ranges)) {
      return uid;
    }
    if (check_phi_ptree_shortcut(t.bwd_ranges, v.bwd_ranges)) {
      return uid;
    }
    if (check_phi_minhat_prune(t.bwd_ranges, v.bwd_ranges)) {
      continue;
    }
    
    runs[e] = run + Q_FWD_V;
    ff.PushFwd(e);
  }
  return -1;
}

inline int QueryGraph::bfsBwd(const int uid, const Node &s) {
  if (runs[uid] == run + Q_FWD_V) {
    return uid;
  }
  for (int e: bwd_edge_slices[uid]) {
    if (runs[e] == run + Q_BWD_V) {
      continue;
    }
    if (runs[e] == run + Q_FWD_V) {
      return e;
    }
    
    Node &v = nodes[e];

    if (check_level_prune(s, v)) {
      continue;
    }
    
    if (check_phi_minhat_prune(s.fwd_ranges, v.fwd_ranges)) {
      continue;
    }
    if (check_phi_hat_shortcut(s.fwd_ranges, v.fwd_ranges)) {
      return uid;
    }
    if (check_phi_ptree_shortcut(s.fwd_ranges, v.fwd_ranges)) {
      return uid;
    }
    if (check_phi_minhat_prune(s.fwd_ranges, v.fwd_ranges)) {
      continue;
    }
    if (check_phi_minhat_prune(v.bwd_ranges, s.bwd_ranges)) {
      continue;
    }
    if (check_phi_hat_shortcut(v.bwd_ranges, s.bwd_ranges)) {
      return uid;
    }
    if (check_phi_ptree_shortcut(v.bwd_ranges, s.bwd_ranges)) {
      return uid;
    }
    if (check_phi_minhat_prune(v.bwd_ranges, s.bwd_ranges)) {
      continue;
    }
    
    runs[e] = run + Q_BWD_V;
    ff.PushBwd(e);
  }
  return -1;
}


int QueryGraph::query(std::vector<Query> &queries) {
  run = 0;
  int reach = 0;
  for (Query &q: queries) {
    if (q.s == q.t) {
      q.r = true;
      reach++;
      continue;
    }
    Node &s = nodes[q.s];
    Node &t = nodes[q.t];
    
    if (check_level_prune(s, t)) {
      q.r = 0;
      continue;
    }
    
    if (check_phi_minhat_prune(s.fwd_ranges, t.fwd_ranges)) {
      q.r = 0;
      continue;
    }
    if (check_phi_hat_shortcut(s.fwd_ranges, t.fwd_ranges)) {
      q.r = 1;
      reach++;
      continue;
    }
    if (check_phi_ptree_shortcut(s.fwd_ranges, t.fwd_ranges)) {
      q.r = 1;
      reach++;
      continue;
    }
    if (check_phi_minhat_prune(s.fwd_ranges, t.fwd_ranges)) {
      q.r = 0;
      continue;
    }
    if (check_phi_minhat_prune(t.bwd_ranges, s.bwd_ranges)) {
      q.r = 0;
      continue;
    }
    if (check_phi_hat_shortcut(t.bwd_ranges, s.bwd_ranges)) {
      q.r = 1;
      reach++;
      continue;
    }
    if (check_phi_ptree_shortcut(t.bwd_ranges, s.bwd_ranges)) {
      q.r = 1;
      reach++;
      continue;
    }
    if (check_phi_minhat_prune(t.bwd_ranges, s.bwd_ranges)) {
      q.r = 0;
      continue;
    }
    
    runs[q.s] = run + Q_FWD_V;
    runs[q.t] = run + Q_BWD_V;
    ff.Clear();
    
    int meet = -1;
    int uid = q.s;
    int vid = q.t;
    int state = BFS_FWD_TOGGLE;

    while ((uid > -1 || vid > -1) && (meet == -1)) {
      switch (state) {
        case BFS_FWD_TOGGLE:
          meet = bfsFwd(uid, t);
          uid = ff.PopFwd();
          state = (vid == -1) ? BFS_FWD_ONLY : BFS_BWD_TOGGLE;
          break;
        case BFS_BWD_TOGGLE:
          meet = bfsBwd(vid, s);
          vid = ff.PopBwd();
          state = (uid == -1) ? BFS_BWD_ONLY : BFS_FWD_TOGGLE;
          break;
        case BFS_FWD_ONLY:
          meet = bfsFwd(uid, t);
          uid = ff.PopFwd();
          break;
        case BFS_BWD_ONLY:
          meet = bfsBwd(vid, s);
          vid = ff.PopBwd();
          break;
        default:
          break;
      }
    }
    run += 3;
    q.r = meet > -1 ? 1 : 0;
    reach += q.r;
  }
  return reach;
}

} // ns preach
