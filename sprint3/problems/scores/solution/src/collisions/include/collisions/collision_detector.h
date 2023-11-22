#pragma once
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "geom.h"

namespace collision_detector {
struct CollisionPrameters {
    double dogWidth;
    double officeWidth;
    double itemWidth;
};
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
class ItemGathererProvider : public IItemGathererProvider {
public:
    ItemGathererProvider(std::vector<Item> items, std::vector<Gatherer> daugs) : items_(items), dogs_(daugs) {}
    size_t   ItemsCount() const override { return items_.size(); }
    Item     GetItem(size_t idx) const override { return items_.at(idx); }
    size_t   GatherersCount() const override { return dogs_.size(); }
    Gatherer GetGatherer(size_t idx) const override { return dogs_.at(idx); }

private:
    std::vector<Item>     items_;
    std::vector<Gatherer> dogs_;
};

using Items       = std::unordered_map<uint32_t, Item>;
using Gatherers   = std::unordered_map<uint32_t, Gatherer>;
using DropOffice  = Item;
using DropOffices = std::unordered_map<uint32_t, DropOffice>;

class IItemsCollider {
protected:
    ~IItemsCollider() = default;

public:
    virtual void              AddGatherer(Gatherer gatherer)                                          = 0;
    virtual void              AddDropOffice(DropOffice dropoff)                                       = 0;
    virtual void              AddItem(Item item)                                                      = 0;
    virtual void              RemoveItem(uint32_t ingame_id)                                          = 0;
    virtual const Items       GetItems() const                                                        = 0;
    virtual const Gatherers   GetGatherers() const                                                    = 0;
    virtual const DropOffices GetDropOffices() const                                                  = 0;
    virtual void              UpdateNextTickPosition(uint32_t gatherer_id, geom::Point2D updated_pos) = 0;
};
class ItemsCollider : public IItemsCollider {
public:
    ~ItemsCollider() = default;
    void              AddGatherer(Gatherer gatherer) override { gatherers_[gatherer.ingame_id] = gatherer; };
    void              AddDropOffice(DropOffice dropoff) override { drops_[dropoff.ingame_id] = dropoff; }
    void              AddItem(Item item) override { items_[item.ingame_id] = item; }
    void              RemoveItem(uint32_t ingame_id) override { items_.erase(ingame_id); }
    const Items       GetItems() const override { return items_; }
    const Gatherers   GetGatherers() const override { return gatherers_; }
    const DropOffices GetDropOffices() const override { return drops_; }
    void              UpdateNextTickPosition(uint32_t gatherer_id, geom::Point2D updated_pos) override {
        auto& gatherer     = gatherers_[gatherer_id];
        gatherer.start_pos = gatherer.end_pos;
        gatherer.end_pos   = updated_pos;
    }

private:
    Items       items_;
    Gatherers   gatherers_;
    DropOffices drops_;
};

struct GatheringEvent {
    uint32_t           item_id;
    uint32_t           gatherer_id;
    double             sq_distance;
    double             time;
    CollisionEventType type = CollisionEventType::ITEM_PICK;
};

std::vector<GatheringEvent> FindGatherEvents(const IItemGathererProvider& provider);
std::vector<GatheringEvent> FindGatherEvents(const IItemsCollider& provider);

}  // namespace collision_detector