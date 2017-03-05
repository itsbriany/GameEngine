#include <gtest/gtest.h>
#include "networking_tests.h"
#include "xmpp_tests.h"
#include "logging.h"

int main(int argc, char* argv[]) {
    Phyre::Logging::set_log_level(Phyre::Logging::kError);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}