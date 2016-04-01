#ifndef NEIGHBOR_NODE_H
#define NEIGHBOR_NODE_H

using namespace std;

class NeighborNode
{
private:
    bool mTwoHopNeighbor;
public:
  NeighborNode();
  ~NeighborNode();
  bool isTwoHopNeighbor();
};

#endif // NEIGHBOR_NODE_H
