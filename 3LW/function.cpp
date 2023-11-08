#include "function.hpp"

#include <fstream>
#include <iostream>
#include <variant>

using namespace lib_simplex_io;

bool isValidLastConstraint(const ParsedConstraint& c)
{
   if (!c.is_last)
      return false;
   const size_t size = c.constraint.size();
   for (size_t i = 0; i < size - 1; ++i) {
      if (c.constraint[i] != 1)
         return false;
   }
   if (c.constraint[size - 1] != 0)
      return false;
   if (c.constraint_type != GREATER_OR_EQUAL)
      return false;
   return true;
}

SimplexMethodSolver::SimplexMethodSolver(
  const std::filesystem::path& tablePath)
{
   static_assert(
     std::is_same<decltype(ParsedFunction::function),
                  function_repr>::value,
     "ParsedFunction::function must match to function_repr type");

   std::ifstream in(tablePath.string());
   if (in) {
      {
         auto f = readFunction(in);
         if (f.success) {
            target_function_ = f.function;
            input_function_type_ = f.function_type;
         }
      }
      {
         ParsedConstraint c;
         while (c.success) {
            c = readConstraint(in);
            if (c.success) {
               if (!c.is_last)
                  constraints_.push_back(
                    { c.constraint, c.constraint_type });
               else if (!isValidLastConstraint(c)) {
                  constraints_ = {};
                  break;
               }
            }
         }
      }
   } else {
      std::cerr << "[ERROR] Cannot open file " << tablePath.string()
                << '\n';
   }
}

std::string SimplexMethodSolver::getSimplexTableuStr() {}
std::string SimplexMethodSolver::getProccessedFunction() {}
