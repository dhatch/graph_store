#include "util/testing.h"

#include "io/buffer_manager.h"
#include "io/buffer.h"

/**
 * The tests in this file only work in the google cloud testing environment.
 */

TEST(BufferManagerReadWrite) {
    BufferManager manager("/dev/sdb");

    {
        auto status_with_buffer = manager.get(0);
        EXPECT_TRUE(status_with_buffer);
        Buffer buf = std::move(*status_with_buffer);
        auto data = static_cast<int*>(buf.getRaw());
        *data = 1234;

        manager.write(buf);
    }

    {
        auto status_with_buffer = manager.get(0);
        EXPECT_TRUE(status_with_buffer);
        Buffer buf = std::move(*status_with_buffer);
        auto data = static_cast<int*>(buf.getRaw());
        EXPECT_EQ(*data, 1234);

        manager.write(buf);
    }

    END;
}

TEST(BufferManagerSize) {
    BufferManager manager("/dev/sdb");

    EXPECT_TRUE(manager.get(2621439));
    EXPECT_FALSE(manager.get(2621440));

    END;
}

int main() {
    BufferManagerReadWrite();
    BufferManagerSize();
}
