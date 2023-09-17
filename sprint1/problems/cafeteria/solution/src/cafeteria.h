#pragma once
#ifdef _WIN32
#include <sdkddkver.h>
#endif
#include <boost/asio.hpp>
#include <boost/asio/execution/executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <memory>

#include "hotdog.h"
#include "result.h"

namespace net = boost::asio;
using namespace std::chrono;
using namespace std::literals;
// Функция-обработчик операции приготовления хот-дога
using HotDogHandler = std::function<void(Result<HotDog> hot_dog)>;
using CoockingReadyHandler = std::function<void(std::shared_ptr<Bread> bread, std::shared_ptr<Sausage> saus)>;

// Класс "Кафетерий". Готовит хот-доги
class Cafeteria
{
   public:
    explicit Cafeteria(net::io_context& io) : io_{io} {}

    // Асинхронно готовит хот-дог и вызывает handler, как только хот-дог будет готов.
    // Этот метод может быть вызван из произвольного потока
    void OrderHotDog(HotDogHandler handler) { CookHotDog(handler); }

   private:
    void CookHotDog(HotDogHandler handler)
    {
        static std::atomic<size_t> hdid{1};
        net::defer(
            io_,
            [this, handler]()
            {
                auto bread = store_.GetBread();
                auto saus = store_.GetSausage();

                size_t id = hdid++;
                bread->StartBake(*gas_cooker_,
                                 [id, bread, saus, handler, this]
                                 {
                                     auto t_bake =
                                         std::make_shared<net::steady_timer>(strand_, HotDog::MIN_BREAD_COOK_DURATION);
                                     t_bake->async_wait(net::bind_executor(
                                         strand_, [t_bake, bread](sys::error_code ec) { bread->StopBaking(); }));
                                 });

                saus->StartFry(
                    *gas_cooker_,
                    [id, bread, saus, handler, this]
                    {
                        auto t_fry = std::make_shared<net::steady_timer>(strand_, HotDog::MIN_SAUSAGE_COOK_DURATION);
                        t_fry->async_wait(net::bind_executor(strand_,
                                                             [t_fry, id, bread, saus, handler](sys::error_code ec)
                                                             {
                                                                 saus->StopFry();
                                                                 CheckHotDog(id, bread, saus, handler);
                                                             }));
                    });
            });
    }

    static void CheckHotDog(size_t id, std::shared_ptr<Bread> bread, std::shared_ptr<Sausage> saus, HotDogHandler h)
    {
        if (bread->IsCooked() && saus->IsCooked())
        {
            HotDog hd(id, saus, bread);
            h(hd);
        }
    }

   private:
    net::io_context& io_;
    using Strand = net::strand<net::io_context::executor_type>;
    Strand strand_{net::make_strand(io_)};
    // Используется для создания ингредиентов хот-дога
    Store store_;
    // Газовая плита. По условию задачи в кафетерии есть только одна газовая плита на 8 горелок
    // Используйте её для приготовления ингредиентов хот-дога.
    // Плита создаётся с помощью make_shared, так как GasCooker унаследован от
    // enable_shared_from_this.
    std::shared_ptr<GasCooker> gas_cooker_ = std::make_shared<GasCooker>(io_);
};
