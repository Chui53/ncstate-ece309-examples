#include "Graph.h"
#include "DList.h"
#include "EdgeList.h"
#include "IntegerSet.h"
#include "Queue.h"
#include <climits>
#include <stdio.h>

SparseGraph::SparseGraph(int n) : Graph(n) { nodes = new Node[numNodes]; }

void SparseGraph::addEdge(int v1, int v2) {
  nodes[v1].edge.append(v2);
  nodes[v2].edge.append(v1);
}

bool SparseGraph::isAdjacent(int v1, int v2) const {
  List::iterator it = nodes[v1].edge.begin();
  while (!it.end()) {
    if (it.getItem() == v2) {
      return true;
    }
    it.increment();
  }
  return false;
}

DirectedSparseGraph::DirectedSparseGraph(int n) : SparseGraph(n) {}
void DirectedSparseGraph::addEdge(int from, int to) {
  nodes[from].edge.append(to);
}

DenseGraph::DenseGraph(int n) : Graph(n) {
  edges = new bool[numNodes * numNodes];
  for (int i = 0; i < numNodes * numNodes; i++)
    edges[i] = false;
}
void DenseGraph::addEdge(int v1, int v2) {
  edges[v1 * numNodes + v2] = true;
  edges[v2 * numNodes + v1] = true;
}

bool DenseGraph::isAdjacent(int v1, int v2) const {
  return edges[v1 * numNodes + v2];
}

bool doesPathExist(Graph &g, int *path, int length) {
  for (int i = 0; i < length - 1; i++) {
    if (!g.isAdjacent(path[i], path[i + 1]))
      return false;
  }
  return true;
}

void visit(int node) { printf("%d ", node); }

void BreadthFirstSearch(Graph &graph, int start) {
  IntegerSetHT discovered(1000);
  Queue frontier;
  frontier.push(start);
  discovered.insert(start);
  while (!frontier.empty()) {
    int node = frontier.peek();
    visit(node);
    List adjList = graph.getAdjacencyList(node);
    List::iterator it = adjList.begin();
    while (!it.end()) {
      int j = it.getItem();
      if (!discovered.search(j)) {
        frontier.push(j);
        discovered.insert(j);
      }
      it.increment();
    }
    frontier.pop();
  }
}

void DepthFirstSearch_helper(Graph &g, IntegerSet &visitedSet, int node) {
  if (!visitedSet.search(node)) {
    visit(node); // take action upon visit to node
    visitedSet.insert(node);
    List adjList = g.getAdjacencyList(node);
    for (List::iterator it = adjList.begin(); !it.end(); it.increment())
      DepthFirstSearch_helper(g, visitedSet, it.getItem());
  }
}

void DepthFirstSearch(Graph &g, int node) {
  IntegerSetHT ht(1000);
  DepthFirstSearch_helper(g, ht, node);
}

class DFS {
protected:
  Graph &g;
  int node;
  IntegerSetHT ht;

  void dfs_helper(Graph &g, IntegerSet &visitedSet, int node) {
    if (!visitedSet.search(node)) {
      preOrderVisit(node); // take action upon visit to node
      visitedSet.insert(node);
      List adjList = g.getAdjacencyList(node);
      for (List::iterator it = adjList.begin(); !it.end(); it.increment())
        dfs_helper(g, visitedSet, it.getItem());

      postOrderVisit(node);
    }
  }

  void dfs(int n) { dfs_helper(g, ht, n); }

public:
  DFS(Graph &agraph, int n) : g(agraph), node(n), ht(1000) {}

  virtual void postOrderVisit(int node) { printf("%d", node); }
  virtual void preOrderVisit(int node) {}

  virtual void run() { dfs(node); }
};

// This function is horrible!!!!!
// There are better ways to do this.
List getNodesWithNoIncomingEdges(Graph &g) {
  bool array[g.getNumNodes()];
  // Assume the node has no incoming edges. We represent
  // that with a true value.
  for (int i = 0; i < g.getNumNodes(); i++)
    array[i] = true;
  // If we find an edge to a node, mark the entry as false
  for (int i = 0; i < g.getNumNodes(); i++) {
    // for all nodes, get their adjacency list and see what they
    // are connected to.  set to false anything adjacent
    List adjList = g.getAdjacencyList(i);
    for (List::iterator it = adjList.begin(); !it.end(); it.increment()) {
      array[it.getItem()] = false;
    }
  }
  // Anything in array still marked true has no incoming edge
  List l;
  for (int i = 0; i < g.getNumNodes(); i++)
    if (array[i]) {
      l.append(i);
    }
  return l;
}

class TopologicalSort : public DFS {
public:
  TopologicalSort(Graph &g, int node) : DFS(g, node) {}

  List sortedList;
  virtual void postOrderVisit(int node) {
    List::iterator it;
    sortedList.insertAfter(it, node);
  }

  virtual void run() {
    List nodes = getNodesWithNoIncomingEdges(g);
    List::iterator it = nodes.begin();
    while (!it.end()) {
      dfs(it.getItem());
      it.increment();
    }
  }
};

void WeightedDenseGraph::addWeight(int from, int to, int weight) {
  weights[from * numNodes + to] = weight;
  weights[to * numNodes + from] = weight;
}

void WeightedDenseGraph::addEdge(int from, int to, int weight) {
  addEdge(from, to);
  addWeight(from, to, weight);
}

class node_helper {
public:
  node_helper() {
    distance = INT_MAX;
    pred = -1;
  }
  int distance;
  int pred;
};

node_helper *ShortestPath(Graph &g, int startV) {

  int numNodes = g.getNumNodes();
  node_helper *nodesH = new node_helper[numNodes];

  nodesH[startV].distance = 0;
  DList unvisitedQueue;
  for (int i = 0; i < numNodes; i++)
    unvisitedQueue.append(i);

  while (!unvisitedQueue.empty()) {
    DList::iterator it = unvisitedQueue.begin();
    int min_node = it.getItem();
    DList::iterator min = it; // remember iterator to minimum
    int min_distance = nodesH[min_node].distance;

    while (!it.end()) {
      if (nodesH[it.getItem()].distance < min_distance) {
        min_node = it.getItem();
        min = it;
        min_distance = nodesH[min_node].distance;
      }
      it.increment();
    }

    int currentV = min_node;
    unvisitedQueue.erase(min); // remove min from queue

    List adjList = g.getAdjacencyList(currentV);
    for (List::iterator lit = adjList.begin(); !lit.end(); lit.increment()) {
      int toNode = lit.getItem();
      if (g.getWeight(currentV, toNode) + min_distance <
          nodesH[toNode].distance) {
        nodesH[toNode].distance = g.getWeight(currentV, toNode) + min_distance;
        nodesH[toNode].pred = currentV;
      }
    }
  }
  return nodesH;
}

EdgeList &getEdgeList(Graph &g) {
  EdgeList *elist = new EdgeList();
  for (int i = 0; i < g.getNumNodes(); i++) {
    List adjList = g.getAdjacencyList(i);
    for (List::iterator lit = adjList.begin(); !lit.end(); lit.increment()) {
      Edge e;
      e.from = i;
      e.to = lit.getItem();
      e.weight = g.getWeight(e.from, e.to);
      elist->insertInOrder(e);
    }
  }
  return *elist;
}

class mst_helper {
public:
  mst_helper() { vertexSet = new List; }
  List *vertexSet;
};

void MinimumSpanningTree(Graph &g, EdgeList &resultList) {
  mst_helper nodes[g.getNumNodes()];
  EdgeList &eList = getEdgeList(g);

  for (int i = 0; i < g.getNumNodes(); i++)
    nodes[i].vertexSet->append(i);

  EdgeList::iterator ei;
  int numNodes = g.getNumNodes();
  for (ei = eList.begin(); numNodes > 1 && !ei.end(); ei.increment()) {
    Edge e = ei.getEdge();
    if (nodes[e.from].vertexSet != nodes[e.to].vertexSet) {
      // not yet connected!
      resultList.append(e);
      List *to = nodes[e.to].vertexSet;
      List::iterator it = to->begin();
      while (!it.end()) {
        nodes[it.getItem()].vertexSet = nodes[e.from].vertexSet;
        it.increment();
      }
      nodes[e.from].vertexSet->append(*to);
      delete to;
      numNodes--;
    }
  }
}

int main() {

  DirectedSparseGraph dag(6);
  dag.addEdge(0, 1);
  dag.addEdge(1, 2);
  dag.addEdge(3, 5);
  dag.addEdge(5, 4);
  dag.addEdge(5, 1);
  dag.addEdge(4, 2);

  TopologicalSort dfs(dag, 0);
  dfs.run();
  printf("Topological sort: ");
  List::iterator it = dfs.sortedList.begin();
  while (!it.end()) {
    printf("%d ", it.getItem());
    it.increment();
  }
  printf("\n");

  WeightedDenseGraph wg(6);
  wg.addEdge(0, 1, 450);
  wg.addEdge(0, 3, 150);
  wg.addEdge(0, 2, 370);
  wg.addEdge(1, 3, 550);
  wg.addEdge(2, 4, 210);
  wg.addEdge(3, 4, 370);
  wg.addEdge(3, 5, 340);
  wg.addEdge(2, 5, 130);
  wg.addEdge(4, 5, 5);

  EdgeList res;
  MinimumSpanningTree(wg, res);
  EdgeList::iterator eit = res.begin();
  while (!eit.end()) {
    printf("From:%d To:%d Weight=%d\n", eit.getEdge().from, eit.getEdge().to,
           eit.getEdge().weight);
    eit.increment();
  }

  return 0;
}
