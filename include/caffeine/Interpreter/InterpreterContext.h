#pragma once

#include "caffeine/Interpreter/Options.h"
#include "caffeine/Solver/Solver.h"

namespace caffeine {

class StackFrame;
class Context;
class FailureLogger;
class ExecutionPolicy;
class ExecutionContextStore;
class Allocation;
class TransformBuilder;
class Interpreter;

/**
 * Wrapper around the required state of an interpreter that provides convenience
 * methods for common things that an opcode/builtin implementation needs to do.
 */
class InterpreterContext {
public:
  Context* ctx;
  std::shared_ptr<Solver> solver;

  FailureLogger* logger;
  ExecutionPolicy* policy;
  ExecutionContextStore* store;
  InterpreterOptions options;

public:
  InterpreterContext(Interpreter* interpeter);
  InterpreterContext(Context* ctx, const std::shared_ptr<Solver>& solver,
                     FailureLogger* logger, ExecutionPolicy* policy,
                     ExecutionContextStore* store,
                     InterpreterOptions options = {});

  // Create a copy of this interpreter context with the inner context replaced
  // with ctx.
  InterpreterContext with_other(Context* ctx) const;

  const llvm::DataLayout& layout() const;
  llvm::Module* module() const;
  Context* context();

  // Get the top frame within the context stack.
  StackFrame& top_frame();
  const StackFrame& top_frame() const;

  // Push a new frame onto the context stack at the start of func.
  StackFrame& push_frame(llvm::Function* func);
  // Pop the top frame off of the context stack and return the frame.
  void pop_frame();

  LLVMValue lookup(llvm::Value* val);

  void insert(llvm::Value* llvm, LLVMValue&& val);
  void insert(llvm::Value* llvm, const LLVMValue& val);

  // Add a new assertion to the path condition of the current context.
  void add(Assertion&& assertion);
  void add(const Assertion& assertion);

  SolverResult check(const Assertion& extra = Assertion());
  SolverResult resolve(const Assertion& extra = Assertion());

  void log_failure(const Assertion& assertion, std::string_view message);

  // Heap Management Helpers

  Allocation& ptr_allocation(const Pointer& ptr);
  llvm::SmallVector<Pointer, 1> ptr_resolve(const Pointer& unresolved);
};

} // namespace caffeine
