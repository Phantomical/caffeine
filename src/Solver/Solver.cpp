#include "caffeine/Solver/Solver.h"
#include "caffeine/IR/Assertion.h"
#include "caffeine/IR/Value.h"
#include "caffeine/IR/Visitor.h"
#include "caffeine/Interpreter/Context.h"
#include "caffeine/Solver/ModelEval.h"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <magic_enum.hpp>

namespace caffeine {

SolverResult::SolverResult(Kind kind, std::unique_ptr<Model> model)
    : kind_(kind), model_(std::move(model)) {
  CAFFEINE_ASSERT(
      kind == SAT || model_ == nullptr,
      "It doesn't make sense to construct a non-SAT SolverResult with a model");
}

bool SolverResult::operator==(Kind kind) const {
  return this->kind() == kind;
}
bool SolverResult::operator!=(Kind kind) const {
  return !(*this == kind);
}

SolverResult::Kind SolverResult::kind() const {
  return kind_;
}
const Model* SolverResult::model() const {
  return model_.get();
}

Value SolverResult::evaluate(const Operation& expr) const {
  return model()->evaluate(expr);
}
Value SolverResult::evaluate(const LLVMScalar& expr, Context& ctx) const {
  return model()->evaluate(expr, ctx);
}
Value SolverResult::evaluate(const LLVMValue& expr, Context& ctx) const {
  return model()->evaluate(expr, ctx);
}

Value Model::evaluate(const Operation& expr) const {
  return ModelEvaluator(this).visit(expr);
}

Value Model::evaluate(const LLVMScalar& scalar, Context& ctx) const {
  ModelEvaluator evaluator{this};

  if (scalar.is_pointer())
    return evaluator.visit(*scalar.pointer().value(ctx.heaps));
  return evaluator.visit(*scalar.expr());
}

Value Model::evaluate(const LLVMValue& expr, Context& ctx) const {
  ModelEvaluator evaluator{this};

  if (expr.is_scalar())
    return evaluate(expr.scalar(), ctx);

  if (expr.is_vector()) {
    std::vector<Value> nested;
    nested.reserve(expr.num_elements());

    auto elems = expr.elements();
    std::transform(elems.begin(), elems.end(), std::back_inserter(nested),
                   [&](const auto& e) { return evaluate(e, ctx); });

    return Value(std::move(nested));
  }

  if (expr.is_aggregate()) {
    std::vector<Value> nested;
    nested.reserve(expr.num_members());

    auto elems = expr.members();
    std::transform(elems.begin(), elems.end(), std::back_inserter(nested),
                   [&](const auto& e) { return evaluate(e, ctx); });

    return Value(std::move(nested));
  }

  CAFFEINE_UNREACHABLE();
}

SolverResult Solver::check(AssertionList& assertions) {
  return check(assertions, Assertion());
}

SolverResult Solver::check(AssertionList& assertions, const Assertion& extra) {
  return SolverResult(resolve(assertions, extra).kind());
}

SolverResult Solver::resolve(AssertionList& assertions) {
  return resolve(assertions, Assertion());
}

std::ostream& operator<<(std::ostream& os, const SolverResult& res) {
  return os << magic_enum::enum_name(res.kind());
}

} // namespace caffeine
