
// Shikhar: Unit tests for VectorUnit configuration.

#include "processor/vector_unit.h"

#include <gtest/gtest.h>

#include <cstdint>

namespace
{

// Shikhar: SEW encoding for e8, e16, e32 and e64.
constexpr uint64_t E8  = 0b000 << 3;
constexpr uint64_t E16 = 0b001 << 3;
constexpr uint64_t E32 = 0b010 << 3;
constexpr uint64_t E64 = 0b011 << 3;

} // namespace

TEST(VectorUnitTest, ResetUsesSimulatorDefaults)
{
    Kites::VectorUnit unit;

    EXPECT_EQ(unit.GetVl(), 0u);
    EXPECT_EQ(unit.GetVtype(), 0u);
    EXPECT_EQ(unit.GetVstart(), 0u);

    EXPECT_EQ(unit.GetSew(), 8u);
    EXPECT_EQ(unit.GetVlMax(), 16u);

    EXPECT_FALSE(unit.IsVill());
    EXPECT_FALSE(unit.IsTailAgnostic());
    EXPECT_FALSE(unit.IsMaskAgnostic());
}

TEST(VectorUnitTest, SupportsAllInitialElementWidths)
{
    Kites::VectorUnit unit;

    EXPECT_EQ(unit.Configure(100, E8), 16u);
    EXPECT_EQ(unit.GetSew(), 8u);
    EXPECT_EQ(unit.GetVlMax(), 16u);

    EXPECT_EQ(unit.Configure(100, E16), 8u);
    EXPECT_EQ(unit.GetSew(), 16u);
    EXPECT_EQ(unit.GetVlMax(), 8u);

    EXPECT_EQ(unit.Configure(100, E32), 4u);
    EXPECT_EQ(unit.GetSew(), 32u);
    EXPECT_EQ(unit.GetVlMax(), 4u);

    EXPECT_EQ(unit.Configure(100, E64), 2u);
    EXPECT_EQ(unit.GetSew(), 64u);
    EXPECT_EQ(unit.GetVlMax(), 2u);
}

TEST(VectorUnitTest, SetsVlWhenAvlIsSmallerThanVlMax)
{
    Kites::VectorUnit unit;

    EXPECT_EQ(unit.Configure(3, E32), 3u);
    EXPECT_EQ(unit.GetVl(), 3u);
}

TEST(VectorUnitTest, LimitsVlToVlMax)
{
    Kites::VectorUnit unit;

    EXPECT_EQ(unit.Configure(100, E32), 4u);
    EXPECT_EQ(unit.GetVl(), 4u);
}

TEST(VectorUnitTest, SupportsZeroAvl)
{
    Kites::VectorUnit unit;

    EXPECT_EQ(unit.Configure(0, E32), 0u);
    EXPECT_EQ(unit.GetVl(), 0u);
}

TEST(VectorUnitTest, StoresTailAndMaskPolicies)
{
    Kites::VectorUnit unit;

    // Shikhar: e32, m1, ta, ma.
    constexpr uint64_t vtype = E32 | (1u << 6) | (1u << 7);

    unit.Configure(4, vtype);

    EXPECT_TRUE(unit.IsTailAgnostic());
    EXPECT_TRUE(unit.IsMaskAgnostic());
    EXPECT_EQ(unit.GetVtype(), vtype);
}

TEST(VectorUnitTest, RejectsUnsupportedSew)
{
    Kites::VectorUnit unit;

    // Shikhar: vsew=100 requests e128.
    unit.Configure(4, 0b100 << 3);

    EXPECT_TRUE(unit.IsVill());
    EXPECT_EQ(unit.GetVl(), 0u);
    EXPECT_EQ(unit.GetVlMax(), 0u);
    EXPECT_EQ(unit.GetVtype(), uint64_t{1} << 63);
}

TEST(VectorUnitTest, RejectsUnsupportedLmul)
{
    Kites::VectorUnit unit;

    // Shikhar: vlmul=001 requests m2.
    unit.Configure(4, E32 | 0b001);

    EXPECT_TRUE(unit.IsVill());
    EXPECT_EQ(unit.GetVl(), 0u);
}

TEST(VectorUnitTest, RejectsReservedVtypeBits)
{
    Kites::VectorUnit unit;

    // Shikhar: Bit 8 is reserved.
    unit.Configure(4, E32 | (1u << 8));

    EXPECT_TRUE(unit.IsVill());
    EXPECT_EQ(unit.GetVl(), 0u);
}

TEST(VectorUnitTest, PreservesVlWhenRequestedAndVlMaxIsUnchanged)
{
    Kites::VectorUnit unit;

    ASSERT_EQ(unit.Configure(3, E32), 3u);

    // Shikhar: Change tail policy without changing VLMAX.
    const uint64_t new_vtype = E32 | (1u << 6);

    EXPECT_EQ(unit.Configure(100, new_vtype, true), 3u);
    EXPECT_EQ(unit.GetVl(), 3u);
    EXPECT_FALSE(unit.IsVill());
    EXPECT_TRUE(unit.IsTailAgnostic());
}

TEST(VectorUnitTest, RejectsPreserveVlWhenVlMaxChanges)
{
    Kites::VectorUnit unit;

    ASSERT_EQ(unit.Configure(3, E32), 3u);

    // Shikhar: Changing e32 to e16 changes VLMAX.
    unit.Configure(100, E16, true);

    EXPECT_TRUE(unit.IsVill());
    EXPECT_EQ(unit.GetVl(), 0u);
}

TEST(VectorUnitTest, ConfigurationResetsVstart)
{
    Kites::VectorUnit unit;

    unit.SetVstart(2);
    EXPECT_EQ(unit.GetVstart(), 2u);

    unit.Configure(4, E32);

    EXPECT_EQ(unit.GetVstart(), 0u);
}

TEST(VectorUnitTest, ResetClearsIllegalConfiguration)
{
    Kites::VectorUnit unit;

    unit.Configure(10, E32 | 1);

    ASSERT_TRUE(unit.IsVill());

    unit.Reset();

    EXPECT_FALSE(unit.IsVill());
    EXPECT_EQ(unit.GetVl(), 0u);
    EXPECT_EQ(unit.GetVtype(), 0u);
    EXPECT_EQ(unit.GetSew(), 8u);
}

