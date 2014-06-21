/*
 * Copyright (c) Florian Merz 2013.
 *
 * The software is provided on an as is basis for research purposes.
 * There is no additional support offered, nor are the author(s)
 * or their institutions liable under any circumstances.
 */

#include <iostream>
#include "Reach.hh"
#include <unistd.h>
#include <sys/time.h>

int main(int argc, char **argv)
{
  float query_time;
  struct timeval after_time, before_time;
  if (argc != 3) {
    return 0;
  }
  preach::Graph *g = new preach::Graph();
  std::cout << "reading " << argv[1] << std::endl;
  std::ifstream infile(argv[1]);
  if (!infile) {
    std::cout << "Error: cannot open " << argv[1] << std::endl;
    return -1;
  }
  g->readGraph(infile);
  std::cout << "#vertex size:" << g->num_nodes << "\t#edges size:" << g->num_edges << std::endl;
  std::cout << "#DAG vertex size:" << g->num_nodes << "\t#DAG edges size:" << g->num_edges << std::endl;

  // read queries
  std::cout << "reading queries from " << argv[2] << std::endl;
  std::vector<preach::Query_t> *queries = new std::vector<preach::Query_t>;
  std::ifstream query_str(argv[2]);
  int s, t, r;
  while (!query_str.eof()) {
    query_str >> s >> t >> r;
    queries->push_back({s,t,r});
  }
  queries->pop_back();
  int num_queries = queries->size();

  std::cout << "building reach ..." << std::endl;
  gettimeofday(&before_time, NULL);
  preach::Reach *reach = new preach::Reach(g);
  gettimeofday(&after_time, NULL);
  query_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 +
    (after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
  std::cout << "levels: " << reach->Levels() << std::endl;
  std::cout << "#construction time:" << query_time << " (ms)" << std::endl;
  std::cout << "#index size: " << g->num_nodes * sizeof(preach::Node_t)/sizeof(int) << std::endl;
#ifdef CLEANUP
  g->CleanUp();
  sleep(1);
#endif
  int reached;
  gettimeofday(&before_time, NULL);
  //for (int i = 0; i < 20; i++) {
  reached = reach->Query(queries);
  //}
  gettimeofday(&after_time, NULL);
  query_time = (after_time.tv_sec - before_time.tv_sec)*1000.0 +
    (after_time.tv_usec - before_time.tv_usec)*1.0/1000.0;
  std::cout << "bfscount: " << reach->BfsCount() << " comp: " << reach->CompleteRuns() << std::endl;
  std::cout << "reached: " << reached << std::endl;
  std::cout << "#total query running time:" << query_time << " (ms)" << std::endl;
  /*
    std::cout << "writing " << argv[2] << std::endl;
    std::ofstream outfile(argv[2]);
    if (!outfile) {
    std::cout << "Error: cannot open " << argv[2] << std::endl;
    return -1;
    }
    g.writeGraph(outfile);
  */
  return 0;
}
