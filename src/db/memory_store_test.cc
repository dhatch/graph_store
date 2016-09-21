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

int main() {
    MemoryStoreAddNode();
    MemoryStoreRemoveNode();
    MemoryStoreFindNode();
    MemoryStoreAddEdge();
    MemoryStoreRemoveEdge();
}
