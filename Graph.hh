/*
 * Copyright (c) Florian Merz 2013.
 *
 * The software is provided on an as is basis for research purposes.
 * There is no additional support offered, nor are the author(s)
 * or their institutions liable under any circumstances.
 *
 */

#ifndef GRAPH_H
#define GRAPH_H
#include <vector>
#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdint.h>
#include <array>

namespace preach {
#define E_E 1
#define E_E_ 2
#define E_ECHFWD 4
#define E_ECHBWD 8

  // node type used for queries
  typedef struct Node {
    int level;
    int level_back;
    int order_fwd;
    int till_fwd;
    int peek_fwd;
    int peek_till_fwd;
    int peek_min_fwd;
    int peek_max_fwd;
    int order_bwd;
    int till_bwd;
    int peek_bwd;
    int peek_till_bwd;
    int peek_min_bwd;
    int peek_max_bwd;
  } Node_t;

  // node type used for refinement
  typedef struct NodeBuild {
    int in;
    int out;
    int lvl_in;
    int lvl_out;
    int tail_edge_list_fwd;
    int tail_edge_list_bwd;
  } NodeBuild_t;

  // node type used when constructin search spaces
  typedef struct NodeContract {
    int in;
    int out;
    int changed;
    int con;
  } NodeContract_t;

  typedef struct EdgeBuild {
    int to;
    int back;
    uint8_t subs;
  } EdgeBuild_t;

  typedef Node_t *NodeList;
  typedef std::vector<NodeBuild_t> NodeBuildList;
  typedef std::vector<NodeContract_t> NodeContractList;
  typedef struct EdgeArray {
    int *a;
    int len;
  } EdgeArray_t;
  typedef EdgeArray_t *EdgeList;
  typedef std::vector<std::vector<EdgeBuild_t> > EdgeBuildList;

  class Graph {
  private:
    void addNode(int);
    void addEdge(int, int);
    void strTrimRight(std::string&);
    int addEdgeLine(std::string&, int);
    void addNodeBuild(int);

    // Refinement
    int LevelBackDfs(int, int);
    int LevelDfs(int, int);
    int OrderBackDfs(int, int);
    int OrderDfs(int, int);
  public:
    int *all_edges;
    int num_nodes;
    int num_edges;
    NodeList nl;
    NodeBuildList nbl;
    NodeContractList ncl;
    EdgeList el_fwd;
    EdgeList el_bwd;
    EdgeBuildList ebl_fwd;
    EdgeBuildList ebl_bwd;
    uint16_t num_levels;
    int num_sinks;
    int num_sources;

    int size_bwd;
    int size_fwd;


    void readGraph(std::istream&);
    void writeGraph(std::ostream&);
    void SetLevels();
    void MakeEdgeList(uint8_t, EdgeList);
    void CleanUp();
    Graph();
    ~Graph();

  };

}
#endif // GRAPH_H
