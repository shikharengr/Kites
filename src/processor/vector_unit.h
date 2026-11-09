
#pragma once
#include "processor/registers.h"

#include <cstddef>
#include <cstdint>

namespace Kites
{

// Manages the architectural state of the RISC-V Vector Extension.
class VectorUnit
{
  public:
    // VLEN is shared with the vector register storage implementation.
    static constexpr std::size_t VLEN_BITS = VECTOR_REGISTER_BITS;

    // Initial implementation supports elements up to 64 bits.
    static constexpr std::size_t ELEN_BITS = 64;

    // Constructor uses the simulator's default vector state.
    VectorUnit() = default;

    // Resets the vector architectural state.
    void Reset();

    // Configures vtype and vl using an AVL and encoded vtype value.
    // preserve_vl represents the special vsetvli rd=x0, rs1=x0 form.
    [[nodiscard]] uint64_t Configure(
        uint64_t avl,
        uint64_t requested_vtype,
        bool preserve_vl = false
    );

    // Reads the current vector length.
    [[nodiscard]] uint64_t GetVl() const;

    // Reads the encoded vector type.
    [[nodiscard]] uint64_t GetVtype() const;

    // Reads the current vector start index.
    [[nodiscard]] uint64_t GetVstart() const;

    // Updates the vector start index.
    void SetVstart(uint64_t value);

    // Reads the current selected element width.
    [[nodiscard]] std::size_t GetSew() const;

    // Calculates the maximum number of elements.
    [[nodiscard]] uint64_t GetVlMax() const;

    // Checks whether the current vector configuration is illegal.
    [[nodiscard]] bool IsVill() const;

    // Reads the tail-agnostic policy.
    [[nodiscard]] bool IsTailAgnostic() const;

    // Reads the mask-agnostic policy.
    [[nodiscard]] bool IsMaskAgnostic() const;

  private:
    //  vill occupies bit XLEN-1. Kites currently implements RV64.
    static constexpr uint64_t VILL_MASK = uint64_t{1} << 63;

    //  Bits 62:8 of the vtype CSR are reserved.
    static constexpr uint64_t VTYPE_RESERVED_MASK =
        ((uint64_t{1} << 55) - 1) << 8;

    //  Current vector architectural state.
    uint64_t vl_ = 0;
    uint64_t vtype_ = 0;
    uint64_t vstart_ = 0;

    // Decoded vector configuration.
    std::size_t sew_ = 8;
    bool vta_ = false;
    bool vma_ = false;
    bool vill_ = false;

    // Marks an unsupported vector configuration.
    void SetIllegalConfiguration();
};

} // namespace Kites
