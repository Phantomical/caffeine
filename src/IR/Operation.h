#pragma once

#include "caffeine/IR/Operation.h"

/**
 * This header has a bunch of utility methods for constant folding.
 */

namespace caffeine {

inline bool is_constant_int(const Operation& op, uint64_t value) {
  if (const auto* constant = llvm::dyn_cast<ConstantInt>(&op))
    return constant->value() == value;
  return false;
}

inline bool constant_int_compare(ICmpOpcode cmp, const llvm::APInt& lhs,
                                 const llvm::APInt& rhs) {
  switch (cmp) {
  case ICmpOpcode::EQ:
    return lhs == rhs;
  case ICmpOpcode::NE:
    return lhs != rhs;
  case ICmpOpcode::SGE:
    return lhs.sge(rhs);
  case ICmpOpcode::SGT:
    return lhs.sgt(rhs);
  case ICmpOpcode::SLE:
    return lhs.sle(rhs);
  case ICmpOpcode::SLT:
    return lhs.slt(rhs);
  case ICmpOpcode::UGE:
    return lhs.uge(rhs);
  case ICmpOpcode::UGT:
    return lhs.ugt(rhs);
  case ICmpOpcode::ULE:
    return lhs.ule(rhs);
  case ICmpOpcode::ULT:
    return lhs.ult(rhs);
  }
  CAFFEINE_UNREACHABLE("unknown ICmpOpcode");
}

} // namespace caffeine