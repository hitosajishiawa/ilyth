#ifndef ILYTH_TYPES_H
#define ILYTH_TYPES_H

#include <cstdint>
#include <limits>

namespace ilyth
{

namespace types
{

using u8  = uint8_t;
using s8  = int8_t;
using u16 = uint16_t;
using s16 = int16_t;
using u32 = uint32_t;
using s32 = int32_t;
using u64 = uint64_t;
using s64 = int64_t;

constexpr auto min_u8  = std::numeric_limits<u8>::min();
constexpr auto max_u8  = std::numeric_limits<u8>::max();
constexpr auto min_s8  = std::numeric_limits<s8>::min();
constexpr auto max_s8  = std::numeric_limits<s8>::max();
constexpr auto min_u16 = std::numeric_limits<u16>::min();
constexpr auto max_u16 = std::numeric_limits<u16>::max();
constexpr auto min_s16 = std::numeric_limits<s16>::min();
constexpr auto max_s16 = std::numeric_limits<s16>::max();
constexpr auto min_u32 = std::numeric_limits<u32>::min();
constexpr auto max_u32 = std::numeric_limits<u32>::max();
constexpr auto min_s32 = std::numeric_limits<s32>::min();
constexpr auto max_s32 = std::numeric_limits<s32>::max();
constexpr auto min_u64 = std::numeric_limits<u64>::min();
constexpr auto max_u64 = std::numeric_limits<u64>::max();
constexpr auto min_s64 = std::numeric_limits<s64>::min();
constexpr auto max_s64 = std::numeric_limits<s64>::max();

static_assert(min_u8 == 0);
static_assert(min_u16 == 0);
static_assert(min_u32 == 0);
static_assert(min_u64 == 0);

}  // namespace types

}  // namespace ilyth

#endif
