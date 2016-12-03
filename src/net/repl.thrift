typedef i64 NodeId;

enum ReplicationCheckpointResult {
  OUT_OF_SPACE = 1,
  CHECKPOINT_DISABLED = 2,
  SUCCESS = 3
}

service Replication {
  bool addNode(1: NodeId nodeId),
  bool removeNode(1: NodeId nodeId),
  bool addEdge(1: NodeId nodeAId, 2: NodeId nodeBId),
  bool removeEdge(1: NodeId nodeAId, 2: NodeId nodeBId),

  ReplicationCheckpointResult checkpoint()
}
