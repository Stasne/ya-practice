#pragma once
#include <dog.h>

namespace model {

class GameSession {
public:
    // using Id = util::Tagged<std::string, GameSession>;
    GameSession(const Map& map) : map_(map) {}

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
}  // namespace model