#ifndef _DB_REPLICATION_MANAGER_H_
#define _DB_REPLICATION_MANAGER_H_

#include "db/graph_store.h"
#include "net/replication_outbound.h"

#include "util/nocopy.h"

/**
 * The ReplicationManager coordinates the chain replication between nodes.
 */
class ReplicationManager {
    DISALLOW_COPY(ReplicationManager);
public:
    enum class NodeType {
        HEAD,
        MID,
        TAIL
    };

    /**
     * Construct a replication manager.
     *
     * type:  The type of this node.
     * successorIp: The IP Address of the successor, if the node type
     * is not TAIL.
     */
    ReplicationManager(NodeType type, GraphStore* store,
            ReplicationOutbound *successorReplicator);

    /**
     * Outbound replication operations.
     *
     * Calling these operations the replication manager causes the manager
     * to coordinate appropriate outbound replication actions futher down the
     * chain.
     *
     * These routines will not return until the action is durable further
     * down the chain.
     *
     * Preconditions:
     *
     * - The operation is valid on this node.
     * - The operation has not yet been committed on this node.
     * - The operation has not been acknowledged.
     *
     * Postconditions:
     *
     * - The operation has been committed on *all* nodes further down the chain.
     *
     * Returns:
     *
     * True on operation success.
     */

    bool replicateAddNode(NodeId nodeId);
    bool replicateRemoveNode(NodeId nodeId);

    bool replicateAddEdge(NodeId nodeAId, NodeId nodeBId);
    bool replicateRemoveEdge(NodeId ndeAId, NodeId nodeBId);

    enum class ReplicatedCheckpointStatus {
        REPL_FAIL,
        OUT_OF_SPACE,
        CHECKPOINT_DISABLED,
        SUCCESS
    };

    ReplicatedCheckpointStatus replicateCheckpoint();

    /**
     * Returns true if this node can accept writes.
     */
    bool writesAllowed() const {
        return _type == NodeType::HEAD;
    }

private:
    NodeType _type;
    GraphStore *_store = nullptr;
    ReplicationOutbound *_successorReplicator = nullptr;
};

#endif /** _DB_REPLICATION_MANAGER_H_ */
