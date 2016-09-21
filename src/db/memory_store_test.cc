#include "util/testing.h"

#include "db/memory_store.h"
#include "db/types.h"
#include "util/status.h"

TEST(MemoryStoreAddNode) {
    MemoryStore store;

    EXPECT_TRUE(store.addNode(1) == StatusCode::SUCCESS);
    EXPECT_TRUE(store.addNode(2) == StatusCode::SUCCESS);

    EXPECT_TRUE(store.addNode(1) == StatusCode::NO_ACTION);

    END;
}

TEST(MemoryStoreRemoveNode) {
    MemoryStore store;

    EXPECT_TRUE(store.removeNode(1) == StatusCode::DOES_NOT_EXIST);

    EXPECT_TRUE(store.addNode(1) == StatusCode::SUCCESS);
    EXPECT_TRUE(store.addNode(2) == StatusCode::SUCCESS);

    EXPECT_TRUE(store.removeNode(1) == StatusCode::SUCCESS);

    EXPECT_FALSE(store.findNode(1));
    EXPECT_TRUE(store.findNode(2));

    END;
}

TEST(MemoryStoreFindNode) {
    MemoryStore store;

    EXPECT_TRUE(store.addNode(1) == StatusCode::SUCCESS);

    auto status = store.findNode(1);
    EXPECT_TRUE(status);

    const Node* node = *status;
    EXPECT_EQ(node->getId(), 1);

    END;
}

TEST(MemoryStoreAddEdge) {
    MemoryStore store;

    EXPECT_TRUE(store.addNode(1));
    EXPECT_TRUE(store.addNode(2));

    EXPECT_TRUE(store.addEdge(1, 2));
    EXPECT_TRUE(store.getEdge(1, 2));

    EXPECT_TRUE(store.addEdge(1, 1) == StatusCode::INVALID);
    EXPECT_TRUE(store.addEdge(1, 2) == StatusCode::NO_ACTION);

    EXPECT_TRUE(store.addEdge(1, 3) == StatusCode::DOES_NOT_EXIST);
    EXPECT_TRUE(store.addEdge(3, 2) == StatusCode::DOES_NOT_EXIST);

    END;
}

TEST(MemoryStoreRemoveEdge) {
    MemoryStore store;

    EXPECT_TRUE(store.addNode(1));
    EXPECT_TRUE(store.addNode(2));
    EXPECT_TRUE(store.addNode(3));

    EXPECT_TRUE(store.addEdge(1, 2));
    EXPECT_TRUE(store.addEdge(1, 3));

    EXPECT_TRUE(store.removeEdge(1, 2));
    EXPECT_FALSE(store.getEdge(1, 2));
    EXPECT_TRUE(store.getEdge(1, 3));

    EXPECT_TRUE(store.removeEdge(4, 1) == StatusCode::DOES_NOT_EXIST);
    EXPECT_TRUE(store.removeEdge(1, 4) == StatusCode::DOES_NOT_EXIST);

    EXPECT_TRUE(store.removeEdge(1, 1) == StatusCode::INVALID);

    END;
}

TEST(MemoryStoreGetNeighbors) {
    MemoryStore store;

    EXPECT_TRUE(store.addNode(1));
    EXPECT_TRUE(store.addNode(2));
    EXPECT_TRUE(store.addNode(3));

    EXPECT_TRUE(store.addEdge(1, 2));
    EXPECT_TRUE(store.addEdge(2, 3));

    auto status_with_edges = store.getNeighbors(1);
    EXPECT_TRUE(status_with_edges);
    NodeIdList edges = *status_with_edges;
    EXPECT_TRUE(std::find(edges.begin(), edges.end(), 2) != edges.end());
    EXPECT_EQ(edges.size(), 1);

    status_with_edges = store.getNeighbors(2);
    EXPECT_TRUE(status_with_edges);
    edges = *status_with_edges;
    EXPECT_TRUE(std::find(edges.begin(), edges.end(), 1) != edges.end());
    EXPECT_TRUE(std::find(edges.begin(), edges.end(), 3) != edges.end());
    EXPECT_EQ(edges.size(), 2);

    EXPECT_FALSE(store.getNeighbors(5));

    END;
}

TEST(MemoryStoreShortestPath) {
    MemoryStore store;

    EXPECT_TRUE(store.addNode(1));
    EXPECT_TRUE(store.addNode(2));
    EXPECT_TRUE(store.addNode(3));
    EXPECT_TRUE(store.addNode(4));
    EXPECT_TRUE(store.addNode(5));

    EXPECT_TRUE(store.addEdge(1, 2));
    EXPECT_TRUE(store.addEdge(2, 3));

    auto status_with_len = store.shortestPath(1, 3);
    EXPECT_TRUE(status_with_len);
    EXPECT_EQ(*status_with_len, 2);

    EXPECT_TRUE(store.addEdge(3, 1));
    EXPECT_TRUE(store.addEdge(2, 4));
    EXPECT_TRUE(store.addEdge(4, 5));

    status_with_len = store.shortestPath(1, 3);
    EXPECT_TRUE(status_with_len);
    EXPECT_EQ(*status_with_len, 1);

    status_with_len = store.shortestPath(1, 4);
    EXPECT_TRUE(status_with_len);
    EXPECT_EQ(*status_with_len, 2);

    status_with_len = store.shortestPath(1, 5);
    EXPECT_TRUE(status_with_len);
    EXPECT_EQ(*status_with_len, 3);

    EXPECT_TRUE(store.addNode(6));
    EXPECT_FALSE(store.shortestPath(1, 6));

    EXPECT_FALSE(store.shortestPath(1, 7));
    EXPECT_FALSE(store.shortestPath(7, 1));
    EXPECT_FALSE(store.shortestPath(1, 1));

    END;
}

int main() {
    MemoryStoreAddNode();
    MemoryStoreRemoveNode();
    MemoryStoreFindNode();
    MemoryStoreAddEdge();
    MemoryStoreRemoveEdge();
    MemoryStoreGetNeighbors();
    MemoryStoreShortestPath();
}
