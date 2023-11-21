#include "collision_detector.h"
#include <cassert>

namespace collision_detector {

CollectionResult TryCollectPoint(geom::Point2D a, geom::Point2D b, geom::Point2D c) {
    // Проверим, что перемещение ненулевое.
    // Тут приходится использовать строгое равенство, а не приближённое,
    // пскольку при сборе заказов придётся учитывать перемещение даже на небольшое
    // расстояние.
    assert(b.x != a.x || b.y != a.y);
    const double u_x = c.x - a.x;
    const double u_y = c.y - a.y;
    const double v_x = b.x - a.x;
    const double v_y = b.y - a.y;
    const double u_dot_v = u_x * v_x + u_y * v_y;
    const double u_len2 = u_x * u_x + u_y * u_y;
    const double v_len2 = v_x * v_x + v_y * v_y;
    const double proj_ratio = u_dot_v / v_len2;
    const double sq_distance = u_len2 - (u_dot_v * u_dot_v) / v_len2;

    return CollectionResult(sq_distance, proj_ratio);
}

std::vector<GatheringEvent> FindGatherEvents(const IItemGathererProvider& provider) {
    std::vector<GatheringEvent> events;

    for (auto i = 0; i < provider.GatherersCount(); ++i) {
        auto gatherer = provider.GetGatherer(i);
        if (gatherer.start_pos.x == gatherer.end_pos.x && gatherer.start_pos.y == gatherer.end_pos.y)
            continue;

        for (auto inum = 0; inum < provider.ItemsCount(); ++inum) {
            auto item = provider.GetItem(inum);
            auto collResult = TryCollectPoint(gatherer.start_pos, gatherer.end_pos, item.position);
            if (collResult.IsCollected(gatherer.width + item.width))
                events.emplace_back(inum, i, collResult.sq_distance, collResult.proj_ratio);
        }
    }

    std::sort(events.begin(), events.end(), [](const auto& e_l, const auto& e_r) { return e_l.time < e_r.time; });

    return events;
}
}  // namespace collision_detector