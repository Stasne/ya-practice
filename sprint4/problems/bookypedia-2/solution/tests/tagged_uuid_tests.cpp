#include <catch2/catch_test_macros.hpp>

#include <bookypedia/util/tagged_uuid.h>

using util::TaggedUUID;

namespace {
struct TestTag {};
using TestUUID = TaggedUUID<TestTag>;
}  // namespace

TEST_CASE("UUID-String conversion") {
    auto uuid = TestUUID::New();
    auto s    = uuid.ToString();
    CHECK(TestUUID::FromString(s) == uuid);
}