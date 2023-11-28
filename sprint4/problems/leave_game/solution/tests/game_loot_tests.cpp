#include <game.h>
#include <json_loader.h>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

using namespace std::literals;

SCENARIO("Game loot spawning") {
    model::Game game = json_loader::LoadGame("../data/config.json");
    GIVEN("Loaded game") {
        auto maps      = game.GetMaps();
        auto spSession = game.StartGame(maps[0], "TestSession");
        auto spDog1    = std::make_shared<Dog>("spike", 1);
        auto spDog2    = std::make_shared<Dog>("buddy", 2);
        WHEN("Game loot with/out dogs") {
            THEN("no dogs no loot") {
                CHECK(spSession->GetPlayers().size() == 0);
                REQUIRE(spSession->GetLoot().size() == 0);
                spSession->UpdateState(std::chrono::milliseconds(5000));
                REQUIRE(spSession->GetLoot().size() == 0);
                spSession->UpdateState(std::chrono::milliseconds(5000));
                REQUIRE(spSession->GetLoot().size() == 0);
            }
            THEN("1 dog, no tick = no loot") {
                CHECK(spSession->GetPlayers().size() == 0);
                REQUIRE(spSession->GetLoot().size() == 0);

                spSession->AddDog(spDog1);
                CHECK(spSession->GetPlayers().size() == 1);
                REQUIRE(spSession->GetLoot().size() == 0);
            }
            THEN("1 dog half a tick = no loot") {
                spSession->AddDog(spDog1);
                spSession->UpdateState(std::chrono::milliseconds(200));
                REQUIRE(spSession->GetLoot().size() == 0);
                spSession->UpdateState(std::chrono::milliseconds(800));
                REQUIRE(spSession->GetLoot().size() == 0);
            }
            THEN("1 dog full(1.5) tick = 1 loot") {
                spSession->AddDog(spDog1);
                spSession->UpdateState(std::chrono::milliseconds(8000));
                REQUIRE(spSession->GetLoot().size() == 1);
            }
            THEN("2 dogs full(1.5) tick = 1 loot") {
                spSession->AddDog(spDog1);
                spSession->AddDog(spDog2);
                spSession->UpdateState(std::chrono::milliseconds(8000));
                REQUIRE(spSession->GetLoot().size() == 1);
            }
            THEN("2 dogs 2 full(2.5) tick = 2 loot") {
                spSession->AddDog(spDog1);
                spSession->AddDog(spDog2);
                spSession->UpdateState(std::chrono::milliseconds(12000));
                REQUIRE(spSession->GetLoot().size() == 2);
            }
        }
    }
}
