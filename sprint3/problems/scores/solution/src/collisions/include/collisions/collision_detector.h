#pragma once
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "geom.h"

namespace collision_detector {

enum class CollisionEventType {
    ITEM_PICK,
    ITEM_DROP,
};

struct CollisionPrameters {
    double dogWidth;
    double officeWidth;
    double itemWidth;
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

struct GatheringEvent {
    uint32_t           item_id;
    uint32_t           gatherer_id;
    double             sq_distance;
    double             time;
    CollisionEventType type = CollisionEventType::ITEM_PICK;
};

using Items       = std::unordered_map<uint32_t, Item>;
using Gatherers   = std::unordered_map<uint32_t, Gatherer>;
using DropOffice  = Item;
using DropOffices = std::unordered_map<uint32_t, DropOffice>;

class ItemsCollider {
public:
    ~ItemsCollider() = default;
    void              AddGatherer(Gatherer gatherer) { gatherers_[gatherer.ingame_id] = gatherer; };
    void              AddDropOffice(DropOffice dropoff) { drops_[dropoff.ingame_id] = dropoff; }
    void              AddItem(Item item) { items_[item.ingame_id] = item; }
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

// Движемся из точки a в точку b и пытаемся подобрать точку c
CollectionResult            TryCollectPoint(geom::Point2D a, geom::Point2D b, geom::Point2D c);
std::vector<GatheringEvent> FindGatherEvents(const ItemsCollider& provider);

// код предложенный авторами курса как стартовый. Оставлен, т.к. на нём выполнялось задание по тестированию
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

std::vector<GatheringEvent> FindGatherEvents(const IItemGathererProvider& provider);

}  // namespace collision_detector