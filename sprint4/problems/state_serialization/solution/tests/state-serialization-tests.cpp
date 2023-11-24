#include <game_serialization.h>
#include <json_loader.h>
#include <players.h>
#include <token_machine.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <unordered_map>
using namespace game;
using namespace std::literals;
namespace {

using InputArchive  = boost::archive::text_iarchive;
using OutputArchive = boost::archive::text_oarchive;

struct Fixture {
    std::stringstream strm;
    OutputArchive     output_archive{strm};
};

}  // namespace

SCENARIO_METHOD(Fixture, "Dog Serialization") {
    GIVEN("a dog") {
        const auto dog = [] {
            Dog dog{"Pluto"s, 42};
            dog.SetPosition({2.3, -1.2});
            return dog;
        }();

        WHEN("dog is serialized") {
            {
                serialization::DogRepr repr{dog};
                output_archive << repr;
            }

            THEN("it can be deserialized") {
                InputArchive           input_archive{strm};
                serialization::DogRepr repr;
                input_archive >> repr;
                const auto restored = repr.Restore();

                CHECK(dog.Id() == restored->Id());
                CHECK(dog.GetName() == restored->GetName());
                CHECK(dog.Position() == restored->Position());
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Players Serialization") {
    GIVEN("Players") {
        const auto playersCollection = [] {
            game::Players players;
            auto          token1 = security::token::CreateToken();  //spToken
            auto          p1     = players.NewPlayer("Player 1 ", token1);
            auto          p2     = players.NewPlayer("Player 2", security::token::CreateToken());
            auto          p3     = players.NewPlayer("Player 3", security::token::CreateToken());
            p1->GetDog()->SetPosition({2.3, -1.2});
            p2->GetDog()->SetPosition({23, 2});
            p3->GetDog()->SetPosition({3, 2});
            return players;
        }();

        WHEN("Players handler serialized") {
            {
                serialization::PlayersHandlerRepr repr{playersCollection};
                output_archive << repr;
            }

            THEN("it can be deserialized. All players are correct") {
                InputArchive                      input_archive{strm};
                serialization::PlayersHandlerRepr repr;
                input_archive >> repr;
                const auto restored = repr.Restore();
                REQUIRE(playersCollection.PlayersCredits().size() == restored.PlayersCredits().size());
                for (const auto& [id, spPlayer] : playersCollection.PlayersMap()) {
                    CHECK(*restored.PlayersMap().at(id) == *spPlayer);
                }
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "game Serialization") {
    GIVEN("Game with 3 dogs and some loot") {
        model::Game restoredGame = json_loader::LoadGame("../data/config.json");
        auto        initGame     = [] {
            model::Game game = json_loader::LoadGame("../data/config.json");
            const auto& maps = game.GetMaps();
            auto        mapIt =
                std::find_if(maps.cbegin(), maps.cend(), [&](const auto& map) { return map.GetName() == "Town"; });
            if (mapIt == maps.cend())
                throw std::runtime_error("Restoring game error: unknown mapname");

            auto& mapTown  = *mapIt;
            auto  session1 = game.StartGame(mapTown, "session-1");
            auto  session2 = game.StartGame(mapTown, "session-2");
            auto& players  = game.PlayersHandler();
            auto  p1       = players.NewPlayer("Player 1 ", security::token::CreateToken());
            auto  p2       = players.NewPlayer("Player 2", security::token::CreateToken());
            auto  p3       = players.NewPlayer("Player 3", security::token::CreateToken());
            auto  dog1     = p1->GetDog();  //->SetPosition({2.3, -1.2});
            auto  dog2     = p2->GetDog();  //->SetPosition({2.3, -1.2});
            auto  dog3     = p3->GetDog();  //->SetPosition({2.3, -1.2});

            dog1->SetPosition({17.23, 82});
            dog2->SetPosition({23, 2});
            dog3->SetPosition({3, 2});
            session1->AddDog(dog1);
            session1->AddDog(dog2);
            session2->AddDog(dog3);
            session1->RestorePlayerLoot(p1->Id(), {{0, 1}, {1, 2}, {2, 3}}, 200);
            session2->RestorePlayerLoot(p2->Id(), {{9, 1}, {9, 2}, {2, 3}}, 999);

            game::GameSession::LootPositions mapLoot{
                           {0, {{12, 13}, 0}},
                           {1, {{22, 22}, 1}},
                           {2, {{33, 33}, 2}},
            };
            session1->RestoreMapLoot(mapLoot);
            session2->RestoreMapLoot(mapLoot);
            return game;
        }();

        serialization::GameRepr repr{initGame};
        output_archive << repr;
        WHEN("Game serialized") {

            THEN("Deserialization completed correctly ") {
                InputArchive            input_archive{strm};
                serialization::GameRepr repr;
                input_archive >> repr;
                repr.RestoreGame(restoredGame);
                CHECK(initGame.GetGameSessions().size() == restoredGame.GetGameSessions().size());
            }
        }
        WHEN("Game deserialized") {
            InputArchive            input_archive{strm};
            serialization::GameRepr repr;
            input_archive >> repr;
            repr.RestoreGame(restoredGame);
            THEN("sessions are same") {
                const auto& originalSessions = initGame.GetGameSessions();
                for (const auto& [id, spRSes] : restoredGame.GetGameSessions()) {
                    const auto& orig = *originalSessions.at(id);
                    const auto& rest = *spRSes;

                    REQUIRE(orig.GetName() == rest.GetName());
                    REQUIRE(orig.GetId() == rest.GetId());
                    REQUIRE(orig.GetMap() == rest.GetMap());
                    CHECK(orig == rest);
                }
            }
            THEN("Other game params are same") {
                REQUIRE(restoredGame.PlayersHandler() == initGame.PlayersHandler());
            }
        }
    }
}