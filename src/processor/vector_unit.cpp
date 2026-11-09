
//  Implementation of vector architectural state management.

#include "processor/vector_unit.h"

#include <algorithm>

namespace Kites
{

void VectorUnit::Reset()
{
    vl_ = 0;
    vtype_ = 0;
    vstart_ = 0;

    sew_ = 8;
    vta_ = false;
    vma_ = false;
    vill_ = false;
}

void VectorUnit::SetIllegalConfiguration()
{
    vl_ = 0;
    vtype_ = VILL_MASK;

    sew_ = 0;
    vta_ = false;
    vma_ = false;
    vill_ = true;
}

uint64_t VectorUnit::Configure(
    uint64_t avl,
    uint64_t requested_vtype,
    bool preserve_vl
)
{
    vstart_ = 0;

    const uint8_t lmul = requested_vtype & 0b111;
    const uint8_t vsew = (requested_vtype >> 3) & 0b111;

    const bool invalid =
        (requested_vtype & (VILL_MASK | VTYPE_RESERVED_MASK)) != 0 ||
        lmul != 0 ||
        vsew > 3;

    if (invalid)
    {
        SetIllegalConfiguration();
        return vl_;
    }

    const std::size_t new_sew = std::size_t{8} << vsew;

    //  With LMUL=1, VLMAX = VLEN / SEW.
    const uint64_t new_vlmax = VLEN_BITS / new_sew;

    uint64_t new_vl = 0;

    if (preserve_vl)
    {
        //  The rd=x0, rs1=x0 form may preserve vl only if VLMAX
        // remains unchanged and the previous configuration is legal.
        if (vill_ || GetVlMax() != new_vlmax)
        {
            SetIllegalConfiguration();
            return vl_;
        }

        new_vl = vl_;
    }
    else
    {
        //  Choosing min(AVL, VLMAX) is a permitted vl selection.
        new_vl = std::min(avl, new_vlmax);
    }

    //  Commit the new vector configuration.
    vl_ = new_vl;
    vtype_ = requested_vtype;
    sew_ = new_sew;

    vta_ = (requested_vtype >> 6) & 1;
    vma_ = (requested_vtype >> 7) & 1;

    vill_ = false;

    return vl_;
}

//  Vector state accessors.

uint64_t VectorUnit::GetVl() const
{
    return vl_;
}

uint64_t VectorUnit::GetVtype() const
{
    return vtype_;
}

uint64_t VectorUnit::GetVstart() const
{
    return vstart_;
}

void VectorUnit::SetVstart(uint64_t value)
{
    vstart_ = value;
}

std::size_t VectorUnit::GetSew() const
{
    return sew_;
}

uint64_t VectorUnit::GetVlMax() const
{
    if (vill_ || sew_ == 0)
        return 0;

    return VLEN_BITS / sew_;
}

bool VectorUnit::IsVill() const
{
    return vill_;
}

bool VectorUnit::IsTailAgnostic() const
{
    return vta_;
}

bool VectorUnit::IsMaskAgnostic() const
{
    return vma_;
}

} // namespace Kites
