#include <game.h>
#include <json_loader.h>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

using namespace std::literals;

SCENARIO("Game loot spawning") {
    model::Game game = json_loader::LoadGame("../data/config.json");
    GIVEN("Loaded game") {
        auto maps = game.GetMaps();
        auto spSession = game.StartGame(maps[0], "TestSession");
        auto spDog1 = std::make_shared<Dog>("spike", 1);
        auto spDog2 = std::make_shared<Dog>("buddy", 2);
        WHEN("Game loot with/out dogs") {
            THEN("no dogs no loot") {
                CHECK(spSession->GetPlayingDogs().size() == 0);
                REQUIRE(spSession->GetLoot().size() == 0);
                spSession->UpdateState(5000);
                REQUIRE(spSession->GetLoot().size() == 0);
                spSession->UpdateState(5000);
                REQUIRE(spSession->GetLoot().size() == 0);
            }
            THEN("1 dog, no tick = no loot") {
                CHECK(spSession->GetPlayingDogs().size() == 0);
                REQUIRE(spSession->GetLoot().size() == 0);

                spSession->AddDog(spDog1);
                CHECK(spSession->GetPlayingDogs().size() == 1);
                REQUIRE(spSession->GetLoot().size() == 0);
            }
            THEN("1 dog half a tick = no loot") {
                spSession->AddDog(spDog1);
                spSession->UpdateState(200);
                REQUIRE(spSession->GetLoot().size() == 0);
                spSession->UpdateState(800);
                REQUIRE(spSession->GetLoot().size() == 0);
            }
            THEN("1 dog full(1.5) tick = 1 loot") {
                spSession->AddDog(spDog1);
                spSession->UpdateState(8000);
                REQUIRE(spSession->GetLoot().size() == 1);
            }
            THEN("2 dogs full(1.5) tick = 1 loot") {
                spSession->AddDog(spDog1);
                spSession->AddDog(spDog2);
                spSession->UpdateState(8000);
                REQUIRE(spSession->GetLoot().size() == 1);
            }
            THEN("2 dogs 2 full(2.5) tick = 2 loot") {
                spSession->AddDog(spDog1);
                spSession->AddDog(spDog2);
                spSession->UpdateState(12000);
                REQUIRE(spSession->GetLoot().size() == 2);
            }
        }
        // WHEN("Half a tick = no loot") {
        //     THEN("no loot till time tick") {

        //     }
        // }

        // WHEN("number of looters exceeds loot count") {
        //     THEN("number of loot is proportional to loot difference") {
        //         for (unsigned loot = 0; loot < 10; ++loot) {
        //             for (unsigned looters = loot; looters < loot + 10; ++looters) {
        //                 INFO("loot count: " << loot << ", looters: " << looters);
        //                 REQUIRE(gen.Generate(TIME_INTERVAL, loot, looters) == looters - loot);
        //             }
        //         }
        //     }
        // }
    }

    // GIVEN("a loot generator with some probability") {
    //     constexpr TimeInterval BASE_INTERVAL = 1s;
    //     LootGenerator gen{BASE_INTERVAL, 0.5};

    //     WHEN("time is greater than base interval") {
    //         THEN("number of generated loot is increased") {
    //             CHECK(gen.Generate(BASE_INTERVAL * 2, 0, 4) == 3);
    //         }
    //     }

    //     WHEN("time is less than base interval") {
    //         THEN("number of generated loot is decreased") {
    //             const auto time_interval = std::chrono::duration_cast<TimeInterval>(
    //                 std::chrono::duration<double>{1.0 / (std::log(1 - 0.5) / std::log(1.0 - 0.25))});
    //             CHECK(gen.Generate(time_interval, 0, 4) == 1);
    //         }
    //     }
    // }

    // GIVEN("a loot generator with custom random generator") {
    //     LootGenerator gen{1s, 0.5, [] {
    //                           return 0.5;
    //                       }};
    //     WHEN("loot is generated") {
    //         THEN("number of loot is proportional to random generated values") {
    //             const auto time_interval = std::chrono::duration_cast<TimeInterval>(
    //                 std::chrono::duration<double>{1.0 / (std::log(1 - 0.5) / std::log(1.0 - 0.25))});
    //             CHECK(gen.Generate(time_interval, 0, 4) == 0);
    //             CHECK(gen.Generate(time_interval, 0, 4) == 1);
    //         }
    //     }
    // }
}
