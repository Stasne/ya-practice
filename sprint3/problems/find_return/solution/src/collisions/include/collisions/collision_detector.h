#pragma once

#include <algorithm>
#include <unordered_map>
#include <vector>
#include "geom.h"

namespace collision_detector {
enum class CollisionEventType {
    ITEM_PICK,
    ITEM_DROP,
};
struct CollectionResult {
    bool IsCollected(double collect_radius) const {
        return proj_ratio >= 0 && proj_ratio <= 1 && sq_distance <= collect_radius * collect_radius;
    }
    // Квадрат расстояния до точки
    double sq_distance;
    // Доля пройденного отрезка
    double proj_ratio;
};

// Движемся из точки a в точку b и пытаемся подобрать точку c
CollectionResult TryCollectPoint(geom::Point2D a, geom::Point2D b, geom::Point2D c);

struct Item {
    uint32_t      ingame_id;
    geom::Point2D position;
    double        width;
};

struct Gatherer {
    uint32_t      ingame_id;
    geom::Point2D start_pos;
    geom::Point2D end_pos;
    double        width;
};

class IItemGathererProvider {
protected:
    ~IItemGathererProvider() = default;

public:
    virtual size_t   ItemsCount() const            = 0;
    virtual Item     GetItem(size_t num) const     = 0;
    virtual size_t   GatherersCount() const        = 0;
    virtual Gatherer GetGatherer(size_t num) const = 0;
};

using Items       = std::unordered_map<uint32_t, Item>;
using Gatherers   = std::vector<Gatherer>;
using DropOffice  = Item;
using DropOffices = std::unordered_map<uint32_t, DropOffice>;

class IItemsCollider {
protected:
    ~IItemsCollider() = default;

public:
    virtual void              AddGatherer(Gatherer gatherer) = 0;
    virtual void              AddOffice(DropOffice dropoff)  = 0;
    virtual void              AddItem(Item item)             = 0;
    virtual void              RemoveItem(size_t ingame_id)   = 0;
    virtual const Items       GetItems() const               = 0;
    virtual const Gatherers   GetGatherers() const           = 0;
    virtual const DropOffices GetDropOffices() const         = 0;
    virtual void              UpdateNextTickPosition()       = 0;
};
class ItemsCollider : public IItemsCollider {
protected:
    ~ItemsCollider() = default;

public:
    void              AddGatherer(Gatherer gatherer) { gatherers_.push_back(std::move(gatherer)); };
    void              AddDropOffice(DropOffice dropoff) { drops_[dropoff.ingame_id] = std::move(dropoff); }
    void              AddItem(Item item) { items_[item.ingame_id] = std::move(item); }
    void              RemoveItem(uint32_t ingame_id) { items_.erase(ingame_id); }
    const Items       GetItems() const { return items_; }
    const Gatherers   GetGatherers() const { return gatherers_; }
    const DropOffices GetDropOffices() const { return drops_; }
    void              UpdateNextTickPosition(uint32_t gatherer_id, geom::Point2D updated_pos) {
        auto& gatherer     = gatherers_[gatherer_id];
        gatherer.start_pos = gatherer.end_pos;
        gatherer.end_pos   = updated_pos;
    }

private:
    Items       items_;
    Gatherers   gatherers_;
    DropOffices drops_;
};

class ItemGathererProvider : public IItemGathererProvider {
public:
    ItemGathererProvider(std::vector<Item> items, std::vector<Gatherer> daugs)
        : items_(std::move(items)), dogs_(std::move(daugs)) {}
    size_t   ItemsCount() const override { return items_.size(); }
    Item     GetItem(size_t idx) const override { return items_.at(idx); }
    size_t   GatherersCount() const override { return dogs_.size(); }
    Gatherer GetGatherer(size_t idx) const override { return dogs_.at(idx); }

private:
    std::vector<Item>     items_;
    std::vector<Gatherer> dogs_;
};

struct GatheringEvent {
    size_t             item_id;
    size_t             gatherer_id;
    double             sq_distance;
    double             time;
    CollisionEventType type = CollisionEventType::ITEM_PICK;
};

std::vector<GatheringEvent> FindGatherEvents(const IItemGathererProvider& provider);
std::vector<GatheringEvent> FindGatherEvents(const IItemsCollider& provider);

}  // namespace collision_detector