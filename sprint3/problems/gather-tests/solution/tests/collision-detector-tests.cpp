#define _USE_MATH_DEFINES

#include <cmath>
#include <functional>
#include <sstream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

#include "../src/collision_detector.h"

// Напишите здесь тесты для функции collision_detector::FindGatherEvents

namespace Catch {
template <>
struct StringMaker<collision_detector::GatheringEvent> {
    static std::string convert(collision_detector::GatheringEvent const& value) {
        std::ostringstream tmp;
        tmp << "(" << value.gatherer_id << value.item_id << value.sq_distance << value.time << ")";

        return tmp.str();
    }
};
}  // namespace Catch
using namespace collision_detector;
class ItemsCollider : public ItemGathererProvider {
public:
    ItemsCollider(std::vector<Item> items, std::vector<Gatherer> daugs)
        : items_(std::move(items)), dogs_(std::move(daugs)) {}
    size_t ItemsCount() const override { return items_.size(); }
    Item GetItem(size_t idx) const override { return items_.at(idx); }
    size_t GatherersCount() const override { return dogs_.size(); }
    Gatherer GetGatherer(size_t idx) const override { return dogs_.at(idx); }

private:
    std::vector<Item> items_;
    std::vector<Gatherer> dogs_;
};

SCENARIO("Gather tests") {
    WHEN("No items on map") {
        // std::vector<Item> items{{1,0}, 0.4};
        std::vector<Item> items{};
        std::vector<Gatherer> dogs{
            {{3, 0}, {4, 0}, 0.4},
            {{0, 0}, {2, 0}, 0.4},
            {{0, 0}, {2, 0}, 0.4},
        };
        ItemsCollider provider(items, dogs);

        THEN("No gathering events'd be found") {
            auto events = FindGatherEvents(provider);
            CHECK(events.size() == 0);
        }
    }
    WHEN("1 item gathered ") {
        std::vector<Item> items{{{1, 0}, 0.4}};
        std::vector<Gatherer> dogs{
            {{3, 0}, {4, 0}, 0.4},
            {{0, 0}, {2, 0}, 0.4},
            {{0, 3}, {0, 5}, 0.4},
        };
        ItemsCollider provider(items, dogs);

        THEN("Only 1 dog'd gather the item") {
            auto events = FindGatherEvents(provider);
            CHECK(events.size() == 1);
            CHECK(events.front().gatherer_id == 1);
        }
    }
    WHEN("Two dogs about to gather the item") {
        std::vector<Item> items{{{2, 0}, 0.4}};
        std::vector<Gatherer> dogs{
            {{3, 0}, {2, 0}, 0.4},
            {{0, 0}, {2, 0}, 0.4},
            {{0, 3}, {0, 5}, 0.4},
        };
        ItemsCollider provider(items, dogs);

        THEN("Item gathered by closest dog") {
            auto events = FindGatherEvents(provider);
            CHECK(events.size() == 2);
            CHECK(events.front().gatherer_id == 0);
        }
    }
    WHEN("Dog has many items on the way") {
        std::vector<Item> items{
            {{2, 0}, 0.4},    // 0 *
            {{3, 0.6}, 0.1},  // 1 miss
            {{3, 0}, 0.4},    // 2 *
            {{4, 0}, 0.1},    // 3 *
            {{5, 1}, 0.2},    // 4 miss
            {{5, 0}, 0.4},    // 5 *
            {{5, 0}, 0.6},    // 6 *
        };
        std::vector<Gatherer> dogs{
            {{1, 0}, {6, 0}, 0.2},
        };
        ItemsCollider provider(items, dogs);

        THEN("Dog gathering items in order from closests item edge") {
            auto events = FindGatherEvents(provider);
            REQUIRE(events.size() == 5);
            events[0].item_id = 0;
            events[1].item_id = 2;
            events[2].item_id = 3;
            events[3].item_id = 6;
            events[4].item_id = 5;
        }
    }
}