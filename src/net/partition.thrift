typedef i64 NodeId;

service Partition {
  bool addEdgePart(1: NodeId nodeLocalId, 2: NodeId nodeRemoteId),
  bool removeEdgePart(1: NodeId nodeLocalId, 2: NodeId nodeRemoteId)
}
