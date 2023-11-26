#pragma once
#include <bookypedia/domain/domain_fwd.h>
#include "use_cases.h"

namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(domain::IAuthorRepository& authors) : authors_{authors} {}

    void AddAuthor(const std::string& name) override;

private:
    domain::IAuthorRepository& authors_;
};

}  // namespace app
