// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <gtest/gtest.h>

#include <type_traits>

namespace DsVeosCoSim::TestDetail {

template <typename Lhs, typename Rhs>
[[nodiscard]] constexpr bool HasSafeIntegerSigns()
{
    using Left = std::remove_cv_t<std::remove_reference_t<Lhs>>;
    using Right = std::remove_cv_t<std::remove_reference_t<Rhs>>;
    if constexpr (std::is_integral_v<Left> && std::is_integral_v<Right> && std::is_signed_v<Left> != std::is_signed_v<Right>) {
        return std::is_signed_v<decltype(Left{} + Right{})>;
    }
    return true;
}

}  // namespace DsVeosCoSim::TestDetail

// Keep the type check at the call site while letting GoogleTest evaluate and print the original operands.
// The switch keeps an enclosing else from binding to the macro's if.
#define DSVEOSCOSIM_CHECKED_COMPARISON(assertion, lhs, rhs)                                                                 \
    switch (0)                                                                                                              \
    case 0:                                                                                                                 \
    default:                                                                                                                \
        if constexpr ([&] {                                                                                                 \
                          static_assert(::DsVeosCoSim::TestDetail::HasSafeIntegerSigns<decltype((lhs)), decltype((rhs))>(), \
                                        "mixed signed/unsigned operands in test comparison");                               \
                          return true;                                                                                      \
                      }())                                                                                                  \
        assertion(lhs, rhs)

#define ASSERT_CHECKED_EQ(lhs, rhs) DSVEOSCOSIM_CHECKED_COMPARISON(ASSERT_EQ, lhs, rhs)
#define ASSERT_CHECKED_NE(lhs, rhs) DSVEOSCOSIM_CHECKED_COMPARISON(ASSERT_NE, lhs, rhs)
#define EXPECT_CHECKED_EQ(lhs, rhs) DSVEOSCOSIM_CHECKED_COMPARISON(EXPECT_EQ, lhs, rhs)
#define EXPECT_CHECKED_NE(lhs, rhs) DSVEOSCOSIM_CHECKED_COMPARISON(EXPECT_NE, lhs, rhs)
#define EXPECT_CHECKED_LT(lhs, rhs) DSVEOSCOSIM_CHECKED_COMPARISON(EXPECT_LT, lhs, rhs)
#define EXPECT_CHECKED_GT(lhs, rhs) DSVEOSCOSIM_CHECKED_COMPARISON(EXPECT_GT, lhs, rhs)
