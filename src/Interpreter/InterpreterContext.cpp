#include "caffeine/Interpreter/InterpreterContext.h"
#include "caffeine/Interpreter/Context.h"
#include "caffeine/Interpreter/FailureLogger.h"
#include "caffeine/Interpreter/Interpreter.h"
#include "caffeine/Interpreter/Options.h"
#include "caffeine/Interpreter/Policy.h"
#include "caffeine/Interpreter/Store.h"
#include "caffeine/Interpreter/TransformBuilder.h"
#include "caffeine/Memory/MemHeap.h"

namespace caffeine {

InterpreterContext::InterpreterContext(Interpreter* interpreter)
    : InterpreterContext(interpreter->ctx, interpreter->solver,
                         interpreter->logger, interpreter->policy,
                         interpreter->store, interpreter->options) {}
InterpreterContext::InterpreterContext(Context* ctx,
                                       const std::shared_ptr<Solver>& solver,
                                       FailureLogger* logger,
                                       ExecutionPolicy* policy,
                                       ExecutionContextStore* store,
                                       InterpreterOptions options)
    : ctx(ctx), solver(solver), logger(logger), policy(policy), store(store),
      options(options) {}

InterpreterContext InterpreterContext::with_other(Context* ctx) const {
  auto copy = *this;
  copy.ctx = ctx;
  return copy;
}

const llvm::DataLayout& InterpreterContext::layout() const {
  return module()->getDataLayout();
}
llvm::Module* InterpreterContext::module() const {
  return ctx->mod;
}
Context* InterpreterContext::context() {
  return ctx;
}

StackFrame& InterpreterContext::top_frame() {
  CAFFEINE_ASSERT(!ctx->stack.empty());
  return ctx->stack.back();
}
const StackFrame& InterpreterContext::top_frame() const {
  CAFFEINE_ASSERT(!ctx->stack.empty());
  return ctx->stack.back();
}

StackFrame& InterpreterContext::push_frame(llvm::Function* func) {
  ctx->stack.emplace_back(func);
  return ctx->stack.back();
}
void InterpreterContext::pop_frame() {
  ctx->pop();
}

LLVMValue InterpreterContext::lookup(llvm::Value* val) {
  return ctx->lookup(val);
}

void InterpreterContext::insert(llvm::Value* llvm, LLVMValue&& val) {
  StackFrame& top = top_frame();
  top.variables.insert_or_assign(llvm, std::move(val));
}
void InterpreterContext::insert(llvm::Value* llvm, const LLVMValue& val) {
  top_frame().insert(llvm, val);
}

void InterpreterContext::add(Assertion&& assertion) {
  ctx->add(std::move(assertion));
}
void InterpreterContext::add(const Assertion& assertion) {
  ctx->add(assertion);
}

SolverResult InterpreterContext::check(const Assertion& assertion) {
  return ctx->check(solver, assertion);
}
SolverResult InterpreterContext::resolve(const Assertion& assertion) {
  return ctx->resolve(solver, assertion);
}

void InterpreterContext::log_failure(const Assertion& assertion,
                                     std::string_view message) {
  auto result = resolve(assertion);
  if (result != SolverResult::SAT)
    return;

  logger->log_failure(result.model(), *ctx, Failure(assertion, message));
  policy->on_path_complete(*ctx, ExecutionPolicy::Fail, assertion);
}

Allocation& InterpreterContext::ptr_allocation(const Pointer& ptr) {
  return ctx->heaps.ptr_allocation(ptr);
}

llvm::SmallVector<Pointer, 1>
InterpreterContext::ptr_resolve(const Pointer& unresolved) {
  return ctx->heaps.resolve(solver, unresolved, *ctx);
}

} // namespace caffeine
