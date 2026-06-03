#include <catch2/catch_test_macros.hpp>
#include "../src/task_service.h"
#include "../src/task_store.h"
#include "../src/priority.h"
#include "../src/date_utils.h"

TEST_CASE("Placeholder test to verify build", "[build]") {
    TaskStore store;
    TaskService service(store);
    REQUIRE(true);
}
