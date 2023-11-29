#pragma once

#include <memory>

#include <bookypedia/domain/domain_fwd.h>

namespace app {

struct UnitOfWork {
    virtual void                       Commit()  = 0;
    virtual domain::IAuthorRepository& Authors() = 0;
    virtual domain::IBookRepository&   Books()   = 0;
    virtual ~UnitOfWork()                        = default;
};

using UnitOfWorkUnit = std::unique_ptr<UnitOfWork>;

class UnitOfWorkFactory {
public:
    virtual UnitOfWorkUnit CreateUnit() = 0;

protected:
    ~UnitOfWorkFactory() = default;
};

}  // namespace app