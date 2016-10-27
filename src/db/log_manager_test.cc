#include "util/testing.h"

#include "db/log_manager.h"
#include "io/buffer_manager.h"

TEST(LogManagerFormatCheckpoint) {
    BufferManager manager("/dev/rdisk2");
    LogManager logManager(manager, {0, 10});

    logManager.format();
    logManager.init();

    LogManager::Reader &reader = logManager.readLog();
    EXPECT_FALSE(reader.hasNext());
    reader.close();

    END;
}

TEST(LogManagerIncrementCheckpoint) {
    BufferManager manager("/dev/rdisk2");
    LogManager logManager(manager, {0, 10});

    logManager.format();
    logManager.init();
    uint64_t generation = logManager.getGeneration();
    EXPECT_EQ(logManager.increaseGeneration(), generation + 1);
    EXPECT_EQ(logManager.getGeneration(), generation + 1);


    END;
}

TEST(LogManagerReadWrite) {
    BufferManager manager("/dev/rdisk2");
    LogManager logManager(manager, {0, 10});

    logManager.format();
    LogManager::Entry entry(LogManager::OpCode::ADD_NODE, 1, 2);
    EXPECT_TRUE(logManager.logOperation(entry));

    LogManager::Reader &reader = logManager.readLog();
    EXPECT_TRUE(reader.hasNext());
    EXPECT_TRUE(reader.getNext() == entry);
    reader.close();

    END;
}

TEST(LogManagerReadWriteAcrossBoundaries) {
    BufferManager manager("/dev/rdisk2");
    LogManager logManager(manager, {0, 10});

    logManager.format();
    LogManager::Entry entry(LogManager::OpCode::ADD_NODE, 1, 2);
    for (int i = 0; i < 230; i++) {
        EXPECT_TRUE(logManager.logOperation(entry));
    }
    LogManager::Entry entryLast(LogManager::OpCode::REMOVE_NODE, 1, 2);
    EXPECT_TRUE(logManager.logOperation(entryLast));

    LogManager::Reader &reader = logManager.readLog();

    for (int i = 0; i < 230; i++) {
        EXPECT_TRUE(reader.hasNext());
        EXPECT_TRUE(reader.getNext() == entry);
    }

    EXPECT_TRUE(reader.hasNext());
    EXPECT_TRUE(reader.getNext() == entryLast);
    reader.close();

    END;
}

TEST(LogManagerIncreasesGeneration) {
    BufferManager manager("/dev/rdisk2");
    LogManager logManager(manager, {0, 10});

    logManager.format();
    LogManager::Entry entry(LogManager::OpCode::ADD_NODE, 1, 2);
    for (int i = 0; i < 230; i++) {
        EXPECT_TRUE(logManager.logOperation(entry));
    }
    LogManager::Entry entryLast(LogManager::OpCode::REMOVE_NODE, 1, 2);
    EXPECT_TRUE(logManager.logOperation(entryLast));

    LogManager::Reader &reader = logManager.readLog();

    for (int i = 0; i < 230; i++) {
        EXPECT_TRUE(reader.hasNext());
        EXPECT_TRUE(reader.getNext() == entry);
    }

    EXPECT_TRUE(reader.hasNext());
    EXPECT_TRUE(reader.getNext() == entryLast);
    reader.close();

    LogManager::Entry entryNew(LogManager::OpCode::ADD_EDGE, 1, 2);
    logManager.increaseGeneration();
    EXPECT_TRUE(logManager.logOperation(entryNew));
    EXPECT_TRUE(logManager.logOperation(entryLast));

    for (int i = 0; i < 210; i++) {
        EXPECT_TRUE(logManager.logOperation(entryNew));
    }

    LogManager::Reader &newReader = logManager.readLog();
    EXPECT_TRUE(newReader.hasNext());
    EXPECT_TRUE(newReader.getNext() == entryNew);
    EXPECT_TRUE(newReader.hasNext());
    EXPECT_TRUE(newReader.getNext() == entryLast);

    for (int i = 0; i < 210; i++) {
        EXPECT_TRUE(newReader.hasNext());
        EXPECT_TRUE(newReader.hasNext());
        EXPECT_TRUE(newReader.getNext() == entryNew);
    }

    END;
}

int main() {
    LogManagerFormatCheckpoint();
    LogManagerIncrementCheckpoint();
    LogManagerReadWrite();
    LogManagerReadWriteAcrossBoundaries();
    LogManagerIncreasesGeneration();
}
