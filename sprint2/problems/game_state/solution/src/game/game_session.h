#pragma once
#include <dog.h>
#include <iostream>
namespace model {

class GameSession {
public:
    // using Id = util::Tagged<std::string, GameSession>;
    GameSession(const Map& map, std::string_view name = "") : map_(map), name_(name) {}

    // const Id& GetId() const noexcept { return id_; }
    std::string_view Name() const noexcept { return name_; }
    void AddDog(const game::Dog& doge) { dogs_.push_back(doge); };  //by vals? wtf?
    using Dogs = std::vector<game::Dog>;
    const Dogs& PlayingDogs() const { return dogs_; }

private:
    Dogs dogs_;
    std::string name_;
    const Map& map_;
};
using spGameSession = std::shared_ptr<GameSession>;
}  // namespace model