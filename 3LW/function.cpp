#include "function.hpp"

#include <fstream>
#include <iostream>
#include <variant>

using namespace simplex_io;

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

void SimplexMethodSolver::createSimplexTableu()
{
    size_t size = target_function_.size();
    for (int i = 0; i < constraints_.size(); i++)
        if (constraints_[i].first.size() > size)
            size = constraints_[i].first.size();
    simplex_tableu_.resize(constraints_.size() + 1);
    for (int i = 0; i < simplex_tableu_.size() - 1; i++) {
        simplex_tableu_[i].resize(size, 0.f);
        for (int j = 0; j < constraints_[i].first.size() - 1; j++)
            simplex_tableu_[i][j] = constraints_[i].first[j];
        simplex_tableu_[i][size - 1] =
            *constraints_[i].first.rbegin();
    }
    simplex_tableu_[simplex_tableu_.size() - 1].resize(size);
    for (int j = 0; j < target_function_.size(); j++)
        simplex_tableu_[simplex_tableu_.size() - 1][j] =
        target_function_[j].second;
    simplex_tableu_[simplex_tableu_.size() - 1][size - 1] *= -1;
}

std::string SimplexMethodSolver::getSimplexTableuStr() {
    return std::string();
}
std::string SimplexMethodSolver::getProccessedFunction() {
    return std::string();
}

void SimplexMethodSolver::doNextStep()
{
    float coef = 0.f;
    int colomn, line;
    for (int i = 0; i < simplex_tableu_[0].size(); i++)
        if (simplex_tableu_[simplex_tableu_.size() - 1][i] < coef)
            colomn = i;
    for (int i = 0; i < simplex_tableu_[0].size(); i++)
        if (simplex_tableu_[simplex_tableu_.size() - 1][i] < coef)
            colomn = i;
}
