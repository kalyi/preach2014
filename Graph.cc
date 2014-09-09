/*
 * Copyright (c) Florian Merz 2013.
 *
 * The software is provided on an as is basis for research purposes.
 * There is no additional support offered, nor are the author(s)
 * or their institutions liable under any circumstances.
 *
 */

#include "Graph.hh"
#include "util.hh"
#include <tuple>
#include <stdlib.h>

namespace preach {


typedef std::tuple<int, int> Pair_t;

bool PairLess(const Pair_t& l, const Pair_t& r) {
  return std::get<0>(l) < std::get<0>(r);
}

bool LexLess(const int& i, const int& j) {
  return std::to_string(i) < std::to_string(j);
}

void Graph::addNode(const int id) {
  Node_t v;
  v.order_fwd = -1;
  v.till_fwd = -1;
  v.peek_fwd = -1;
  v.peek_till_fwd = -1;
  v.peek_min_fwd = -1;
  v.peek_max_fwd = -1;
  v.order_bwd = -1;
  v.till_bwd = -1;
  v.peek_bwd = -1;
  v.peek_till_bwd = -1;
  v.peek_min_bwd = -1;
  v.peek_max_bwd = -1;
  v.level = 0;
  v.level_back = 0;
  nl[id] = v;
}

void Graph::addNodeBuild(const int id) {
  NodeBuild_t v;
  v.in = 0;
  v.out = 0;
  v.lvl_in = 0;
  v.lvl_out = 0;
  v.tail_edge_list_fwd = 0;
  v.tail_edge_list_bwd = 0;
  nbl[id] = v;
}

void Graph::strTrimRight(std::string& str) {
  std::string whitespaces(" \t");
  int index = str.find_last_not_of(whitespaces);
  if (index != std::string::npos)
    str.erase(index+1);
  else
    str.clear();
}

int Graph::addEdgeLine(std::string& buf, const int sid) {
  int deg;
  int idx;
  int tid;
  std::string sub;
  strTrimRight(buf);
  idx = buf.find(":");
  buf.erase(0, idx+2);
  deg = std::count(buf.begin(), buf.end(), ' ');
  std::vector<int> tmp_nl = std::vector<int>(deg);
  // check if unsorted edges break something TODO
  idx = 0;
  while (buf.find(" ") != std::string::npos) {
    sub = buf.substr(0, buf.find(" "));
    std::istringstream(sub) >> tid;
    buf.erase(0, buf.find(" ")+1);

    tmp_nl[idx] = tid;
    idx++;
  }

  //          for (std::vector<int>::iterator it = tmp_nl.begin(); it != tmp_nl.end(); it++) {
  for (int& it: tmp_nl) {
    if (nbl[sid].out == 0) {
      num_sinks--;
    }
    nbl[sid].out++;
    ncl[sid].out++;
    if (nbl[it].in == 0) {
      num_sources--;
    }
    nbl[it].in++;
    ncl[it].in++;
    addEdge(sid, it);
  }
  return tmp_nl.size();
}

void Graph::addEdge(const int sid, const int tid) {
  int back_fwd = nbl[tid].tail_edge_list_bwd++;
  int back_bwd = nbl[sid].tail_edge_list_fwd++;
  ebl_fwd[sid].push_back((EdgeBuild_t){tid, back_fwd, E_E});
  ebl_bwd[tid].push_back((EdgeBuild_t){sid, back_bwd, E_E_});
}


void Graph::readGraph(std::istream& in) {
  std::string line;
  int n, m;
  std::stringstream ss;
  std::getline(in, line);
  std::getline(in, line);
  ss << line;
  ss >> n;

  num_sinks = n;
  num_sources = n;

  nl = (NodeList)malloc(n * sizeof(Node_t));
  for (int i = 0; i < n; i++) {
    addNode(i);
  }
  nbl = NodeBuildList(n);
  for (int i = 0; i < n; i++) {
    addNodeBuild(i);
  }
  ncl = NodeContractList(n);
  for (int i = 0; i < n; i++) {
    ncl[i] = NodeContract_t{0,0,0,0};
  }

  ebl_fwd = EdgeBuildList(n);
  ebl_bwd = EdgeBuildList(n);
  for (std::vector<EdgeBuild_t>& ebv: ebl_fwd) {
    ebv = std::vector<EdgeBuild_t>();
  }
  for (std::vector<EdgeBuild_t>& ebv: ebl_bwd) {
    ebv = std::vector<EdgeBuild_t>();
  }

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
      if (nbl[sid].out == 0) {
        num_sinks--;
      }
      nbl[sid].out++;
      ncl[sid].out++;
      if (nbl[tid].in == 0) {
        num_sources--;
      }
      nbl[tid].in++;
      ncl[tid].in++;
      addEdge(sid, tid);
      m++;
    }
  }
  num_edges = m;
  num_nodes = n;
}

void Graph::writeGraph(std::ostream& out) {
  std::cout << "Nodes: " << num_nodes << " Edges: " << num_edges << std::endl;
  out << "graph_for_greach" << std::endl;
  out << num_nodes << std::endl;

  EdgeBuildList::iterator ebl_fwd_it;
  std::vector<EdgeBuild_t>::iterator eb_it;
  int i = 0;
  for (ebl_fwd_it = ebl_fwd.begin(); ebl_fwd_it != ebl_fwd.end(); ebl_fwd_it++) {
    out << i << ": ";
    for (eb_it = (*ebl_fwd_it).begin(); eb_it != (*ebl_fwd_it).end(); eb_it++) {
      if ((*eb_it).subs & E_E) {
        out << (*eb_it).to << " ";
      }
    }
    out << "#" << std::endl;
    i++;
  }
}
/* Refinemnet */
int Graph::LevelBackDfs(const int vid, int tree_size) {
  int level_back = nl[vid].level_back;
  NodeBuild_t *tb;
  for (EdgeBuild_t& e: ebl_bwd[vid]) {
    if (nl[e.to].level_back <= level_back) {
      nl[e.to].level_back = level_back + 1;
    }
  }
  for (EdgeBuild_t& e: ebl_bwd[vid]) {
    tb = &nbl[e.to];
    tb->lvl_out++;
    if (tb->out == tb->lvl_out) {
      tree_size = LevelBackDfs(e.to, tree_size + 1);
    }
  }
  return tree_size;
}

int Graph::LevelDfs(const int vid, int tree_size) {
  int level = nl[vid].level;
  NodeBuild_t *tb;
  for (EdgeBuild_t& e: ebl_fwd[vid]) {
    if (nl[e.to].level <= level) {
      nl[e.to].level = level + 1;
    }
  }
  for (EdgeBuild_t& e: ebl_fwd[vid]) {
    tb = &nbl[e.to];
    tb->lvl_in++;
    if (tb->in == tb->lvl_in) {
      tree_size = LevelDfs(e.to, tree_size + 1);
    }
  }
  return tree_size;
}

int Graph::OrderBackDfs(const int vid, int order) {
  Node_t *v = &nl[vid];

  int max_peek = 1;
  v->order_bwd = order;
  order++;

  v->peek_bwd = v->order_bwd;
  v->peek_min_bwd = v->order_bwd;
  v->peek_till_bwd = -2;
  Node_t *t;
  for (EdgeBuild_t& e: ebl_bwd[vid]) {
    t = &nl[e.to];
    if (t->peek_till_bwd == -1) {
      order = OrderBackDfs(e.to, order);
    }
    if (t->order_bwd < v->order_bwd) {
      if ((t->till_bwd - t->order_bwd) > max_peek) {
        max_peek = t->till_bwd - t->order_bwd;
        v->peek_bwd = t->order_bwd;
        v->peek_till_bwd = t->till_bwd;
      }
      if (t->till_bwd > v->peek_max_bwd) {
        v->peek_max_bwd = t->till_bwd;
      }
    }
    if (t->peek_bwd < v->order_bwd) {
      if ((t->peek_till_bwd - t->peek_bwd) > max_peek) {
        max_peek = t->peek_till_bwd - t->peek_bwd;
        v->peek_bwd = t->peek_bwd;
        v->peek_till_bwd = t->peek_till_bwd;
      }
    }
    if (t->peek_max_bwd > v->peek_max_bwd) {
      v->peek_max_bwd = t->peek_max_bwd;
    }
    if ((t->peek_min_bwd > -1) && (t->peek_min_bwd < v-> peek_min_bwd)) {
      v->peek_min_bwd = t->peek_min_bwd;
    }
  }
  if (v->peek_till_bwd == -2) {
    v->peek_till_bwd = order;
  }
  v->till_bwd = order;
  return order;
}

int Graph::OrderDfs(const int vid, int order) {
  Node_t *v = &nl[vid];

  int max_peek = 1;
  v->order_fwd = order;
  order++;

  v->peek_fwd = v->order_fwd;
  v->peek_min_fwd = v->order_fwd;
  v->peek_till_fwd = -2;
  Node_t *t;
  for (EdgeBuild_t& e: ebl_fwd[vid]) {
    t = &nl[e.to];
    if (t->peek_till_fwd == -1) {
      order = OrderDfs(e.to, order);
    }
    if (t->order_fwd < v->order_fwd) {
      if ((t->till_fwd - t->order_fwd) > max_peek) {
        max_peek = t->till_fwd - t->order_fwd;
        v->peek_fwd = t->order_fwd;
        v->peek_till_fwd = t->till_fwd;
      }
      if (t->till_fwd > v->peek_max_fwd) {
        v->peek_max_fwd = t->till_fwd;
      }
    }
    if (t->peek_fwd < v->order_fwd) {
      if ((t->peek_till_fwd - t->peek_fwd) > max_peek) {
        max_peek = t->peek_till_fwd - t->peek_fwd;
        v->peek_fwd = t->peek_fwd;
        v->peek_till_fwd = t->peek_till_fwd;
      }
    }
    if (t->peek_max_fwd > v->peek_max_fwd) {
      v->peek_max_fwd = t->peek_max_fwd;
    }
    if ((t->peek_min_fwd > -1) && (t->peek_min_fwd < v-> peek_min_fwd)) {
      v->peek_min_fwd = t->peek_min_fwd;
    }
  }
  if (v->peek_till_fwd == -2) {
    v->peek_till_fwd = order;
  }
  v->till_fwd = order;
  return order;
}

void Graph::SetLevels() {
  std::vector<Pair_t> l_fwd = std::vector<Pair_t>();
  std::vector<Pair_t> l_bwd = std::vector<Pair_t>();

  int tree_size = 0;
  int id = 0;
  TPRECALL();
  for (NodeBuild_t& nb: nbl) {
    if (nb.in == 0) {
      tree_size = LevelDfs(id, 0);
      l_fwd.push_back(std::make_tuple(num_nodes - tree_size, id));
    }
    if (nb.out == 0) {
      tree_size = LevelBackDfs(id, 0);
      l_bwd.push_back(std::make_tuple(num_nodes - tree_size, id));
    }
    id++;
  }
  TPOSTCALL("MakeLevels");
  std::sort(l_fwd.begin(), l_fwd.end(), PairLess);
  std::sort(l_bwd.begin(), l_bwd.end(), PairLess);
  TPOSTCALL("SortTrees");

  std::cout << "sinks: " << num_sinks << " source: " << num_sources << std::endl;

  int order_fwd = 0;
  int order_bwd = 0;

  TPRECALL();
  for (Pair_t& p: l_fwd) {
    order_fwd = OrderDfs(std::get<1>(p), order_fwd);
  }
  TPOSTCALL("OrderDFS");
  for (Pair_t& p: l_bwd) {
    order_bwd = OrderBackDfs(std::get<1>(p), order_bwd);
  }
  TPOSTCALL("OrderBackDFS");
}

// fill edges array used for queries
void Graph::MakeEdgeList(uint8_t filter, EdgeList el, int *edges) {
  int offset = 0;
  EdgeBuildList::iterator ebl_it;
  EdgeBuildList::iterator ebl_end;
  std::vector<EdgeBuild_t>::iterator ebv_it;
  int i = 0;
  if (filter == E_ECHFWD) {
    ebl_it = ebl_fwd.begin();
    ebl_end = ebl_fwd.end();
  } else {
    ebl_it = ebl_bwd.begin();
    ebl_end = ebl_bwd.end();
  }

  for (; ebl_it != ebl_end; ebl_it++) {
    int m = 0;
    for (ebv_it = ebl_it->begin(); ebv_it != ebl_it->end(); ebv_it++) {
      if (((*ebv_it).subs & filter) > 0) {
        m++;
      }
    }
    el[i].a = edges + offset;
    el[i].len = m;
    offset += m;
    ebv_it = ebl_it->begin();
    int j = 0;
    while (j < m) {
      if (((*ebv_it).subs & filter) > 0) {
        el[i].a[j] = (*ebv_it).to;
        j++;
      }
      ebv_it++;
    }
    i++;
  }
}

void Graph::CleanUp() {
  for (std::vector<EdgeBuild_t>& ebl: ebl_fwd) {
    std::vector<EdgeBuild_t>().swap(ebl);
  }
  EdgeBuildList().swap(ebl_fwd);
  for (std::vector<EdgeBuild_t>& ebl: ebl_bwd) {
    std::vector<EdgeBuild_t>().swap(ebl);
  }
  EdgeBuildList().swap(ebl_bwd);
  NodeBuildList().swap(nbl);
  NodeContractList().swap(ncl);
}

Graph::Graph() {

}

Graph::~Graph() {
  if (nl) {
    free(nl);
    nl = nullptr;
  }
  if (el_fwd) {
    free(el_fwd);
    el_fwd = nullptr;
  }
  if (el_bwd) {
    free(el_bwd);
    el_bwd = nullptr;
  }
}


} // namespace p2reach
