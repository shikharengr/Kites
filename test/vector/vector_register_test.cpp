#include "processor/registers.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace
{

uint64_t GetElementMask(size_t width)
{
    if (width == 64)
        return ~uint64_t{0};

    return (uint64_t{1} << width) - 1;
}

void CheckElementRoundTrip(size_t element_width, size_t element_count)
{
    Kites::RegisterFile register_file;

    std::array<uint64_t, 16> expected{};
    const uint64_t mask = GetElementMask(element_width);

    // Write all elements first.
    for (size_t i = 0; i < element_count; ++i)
    {
        const uint64_t value =
            0xFEDCBA9876543210ULL ^
            (static_cast<uint64_t>(i) * 0x0102030405060708ULL);

        expected[i] = value & mask;

        register_file.WriteVrElement(
            3, i, element_width, value
        );
    }

    // Read all elements and verify their values.
    for (size_t i = 0; i < element_count; ++i)
    {
        EXPECT_EQ(
            register_file.ReadVrElement(3, i, element_width),
            expected[i]
        ) << "Element index: " << i
          << ", element width: " << element_width;
    }
}

} // namespace


TEST(VectorRegisterTest, WholeRegisterReadWrite)
{
    Kites::RegisterFile register_file;

    Kites::VectorRegister expected{};
    expected.words[0] = 0x0123456789ABCDEFULL;
    expected.words[1] = 0xFEDCBA9876543210ULL;

    register_file.WriteVr(7, expected);

    const Kites::VectorRegister actual = register_file.ReadVr(7);

    EXPECT_EQ(actual.words[0], expected.words[0]);
    EXPECT_EQ(actual.words[1], expected.words[1]);
}


TEST(VectorRegisterTest, Supports8BitElements)
{
    CheckElementRoundTrip(8, 16);
}

TEST(VectorRegisterTest, Supports16BitElements)
{
    CheckElementRoundTrip(16, 8);
}

TEST(VectorRegisterTest, Supports32BitElements)
{
    CheckElementRoundTrip(32, 4);
}

TEST(VectorRegisterTest, Supports64BitElements)
{
    CheckElementRoundTrip(64, 2);
}


TEST(VectorRegisterTest, MasksValuesToElementWidth)
{
    Kites::RegisterFile register_file;

    register_file.WriteVrElement(0, 0, 8, 0x1234);

    EXPECT_EQ(register_file.ReadVrElement(0, 0, 8), 0x34);
    EXPECT_EQ(register_file.ReadVrElement(0, 1, 8), 0x00);
}


TEST(VectorRegisterTest, RejectsInvalidRegisterIndex)
{
    Kites::RegisterFile register_file;
    Kites::VectorRegister value{};

    EXPECT_THROW(register_file.ReadVr(32), std::out_of_range);
    EXPECT_THROW(register_file.WriteVr(32, value), std::out_of_range);

    EXPECT_THROW(
        register_file.ReadVrElement(32, 0, 8),
        std::out_of_range
    );

    EXPECT_THROW(
        register_file.WriteVrElement(32, 0, 8, 10),
        std::out_of_range
    );
}

TEST(VectorRegisterTest, RejectsInvalidElementWidth)
{
    Kites::RegisterFile register_file;

    EXPECT_THROW(
        register_file.ReadVrElement(0, 0, 7),
        std::invalid_argument
    );

    EXPECT_THROW(
        register_file.WriteVrElement(0, 0, 128, 10),
        std::invalid_argument
    );
}

TEST(VectorRegisterTest, RejectsOutOfRangeElementIndex)
{
    Kites::RegisterFile register_file;

    EXPECT_THROW(
        register_file.ReadVrElement(0, 16, 8),
        std::out_of_range
    );

    EXPECT_THROW(
        register_file.WriteVrElement(0, 2, 64, 10),
        std::out_of_range
    );
}


TEST(VectorRegisterTest, ResetClearsAllVectorRegisters)
{
    Kites::RegisterFile register_file;

    Kites::VectorRegister value{};
    value.words[0] = 0xFFFFFFFFFFFFFFFFULL;
    value.words[1] = 0xFFFFFFFFFFFFFFFFULL;

    register_file.WriteVr(0, value);
    register_file.WriteVr(31, value);

    register_file.Reset();

    const auto registers = register_file.GetVrValues();

    ASSERT_EQ(registers.size(), 32u);

    for (const auto &reg : registers)
    {
        EXPECT_EQ(reg.words[0], 0u);
        EXPECT_EQ(reg.words[1], 0u);
    }
}

TEST(VectorRegisterTest, RecognizesVectorRegisterNames)
{
    EXPECT_TRUE(Kites::IsValidVectorRegister("v0"));
    EXPECT_TRUE(Kites::IsValidVectorRegister("v15"));
    EXPECT_TRUE(Kites::IsValidVectorRegister("v31"));

    EXPECT_FALSE(Kites::IsValidVectorRegister("v32"));
    EXPECT_FALSE(Kites::IsValidVectorRegister("x1"));
    EXPECT_FALSE(Kites::IsValidVectorRegister("invalid"));
}