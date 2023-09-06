#ifndef _UTILS_H_
#define _UTILS_H_
#include <array>
#include "audio.h"
namespace utils
{
bool portValid(uint32_t port)
{
    return (port > 0 && port < 65536);
}
}  // namespace utils
#endif  // _UTILS_H_