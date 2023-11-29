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
   // static_assert(
   //  std::is_same<decltype(ParsedFunction::function),
   //               function_repr>::value,
   // "ParsedFunction::function must match to function_repr type");

    std::ifstream in(tablePath.string());
    if (in) {
        {
            auto f = readFunction(in);
            if (f.success) {
                p.function = f;
                target_function_ = f;
            }
        }
        {
            ParsedConstraint c;
            while (c.success) {
                c = readConstraint(in);
                if (c.success) {
                    if (!c.is_last)
                        p.constraints.push_back(
                            { c.constraint, c.constraint_type });
                    else if (!isValidLastConstraint(c)) {
                        p.constraints = {};
                        break;
                    }
                }
            }
        }

        size_t size = p.function.function.size(), size_i;
        for (int i = 0; i < p.constraints.size(); i++)
            if (p.constraints[i].constraint.size() > size)
                size = p.constraints[i].constraint.size();
        for (int i = 0; i < p.constraints.size(); i++) {
            size_i = p.constraints[i].constraint.size();
            p.constraints[i].constraint.resize(size, 0.);
            std::swap(p.constraints[i].constraint[size - 1],
                p.constraints[i].constraint[size_i - 1]);
        }

        p = convertToCanonical(p);
        if (isCanonicalProblem(p))
            createSimplexTableu(getBasisVariablesVector(p));
    }
    else {
        std::cerr << "[ERROR] Cannot open file " << tablePath.string()
            << '\n';
    }
}

void SimplexMethodSolver::createSimplexTableu(
  const std::vector<size_t>& basis_var_ind)
{
    simplex_tableu_.basis_variables_indexes = basis_var_ind;
    size_t size = p.constraints[0].constraint.size();

    simplex_tableu_.table.resize(p.constraints.size() + 1);
    for (int i = 0; i < simplex_tableu_.table.size() - 1; i++) {
        simplex_tableu_.table[i].resize(size);
        for (int j = 0; j < size; j++)
            simplex_tableu_.table[i][j] = p.constraints[i].constraint[j];
    }
    simplex_tableu_.table[simplex_tableu_.table.size() - 1].resize(
        size, 0.);

    for (int j = 0; j < p.function.function.size(); j++)
        simplex_tableu_.table[simplex_tableu_.table.size() - 1][j] =
        p.function.function[j].second;

   simplex_tableu_
     .table[simplex_tableu_.table.size() - 1][size - 1] *= -1;

   simplex_tableu_.variable_count = size - 1;

   auto& c = simplex_tableu_.basis_variables_indexes; //
   float coef1, coef2;
   for (int ii = 0; ii < c.size(); ii++) {
      coef1 = simplex_tableu_.table[ii][c[ii]];
      for (int j = 0; j < size; j++)
         simplex_tableu_.table[ii][j] /= coef1;

      coef2 = simplex_tableu_.table[c.size()][c[ii]];
      for (int j = 0; j < size; j++)
         simplex_tableu_.table[c.size()][j] -=
           simplex_tableu_.table[ii][j] * coef2 / coef1;
   }
}

std::string SimplexMethodSolver::getSimplexTableuStr()
{
   return std::string();
}
std::string SimplexMethodSolver::getProccessedFunction()
{
   return std::string();
}

SimplexMethodStatus SimplexMethodSolver::doNextStep()
{
   SimplexMethodStatus st { false, false };
   size_t last_line = simplex_tableu_.table.size() - 1;
   size_t last_col = simplex_tableu_.table[0].size() - 1;
   float coef = 0.f;
   int i, j, colomn = -1, line = -1;
   for (i = 0; i < last_col; i++)
      if (simplex_tableu_.table[last_line][i] < coef) {
         coef = simplex_tableu_.table[last_line][i];
         colomn = i;
      }
   if (colomn != -1) {
      float buf;
      i = 0;
      while (line == -1 && i < last_line) {
         if (simplex_tableu_.table[i][colomn] > 1e-6f)
            line = i;
         i++;
      }
      if (line != -1) {
         coef = simplex_tableu_.table[line][last_col] /
                simplex_tableu_.table[line][colomn];
         for (; i < last_line; i++) {
            if (simplex_tableu_.table[i][colomn] > 1e-6f) {
               buf = simplex_tableu_.table[i][last_col] /
                     simplex_tableu_.table[i][colomn];
               if (buf < coef) {
                  line = i;
                  coef = buf;
               }
            }
         }

         simplex_tableu_.basis_variables_indexes[line] = colomn;

         for (i = 0; i < simplex_tableu_.table.size(); i++) {
            if (i != line) {
               coef = simplex_tableu_.table[i][colomn] /
                      simplex_tableu_.table[line][colomn];
               for (j = 0; j < simplex_tableu_.table[i].size(); j++)
                  simplex_tableu_.table[i][j] -=
                    simplex_tableu_.table[line][j] * coef;
            } else {
               coef = simplex_tableu_.table[line][colomn];
               for (j = 0; j < simplex_tableu_.table[i].size(); j++)
                  simplex_tableu_.table[i][j] /= coef;
            }
         }
      } else {
         st.is_infinity = true;
         st.is_end = true;
      }
   } else
      st.is_end = true;
   return st;
}

SimplexMethodAnswer SimplexMethodSolver::solve()
{
    SimplexMethodStatus st{ false, false };
    while (!st.is_end) {
        std::cout << simplex_tableu_ << std::endl;
        st = doNextStep();
    }
    if (st.is_infinity)
        return { 0.f, std::vector<float>(), true, true };
    SimplexMethodAnswer ans{
       0.f, std::vector<float>(simplex_tableu_.table[0].size() - 1, 0.f), true, false
    };
    int i;
    for (i = 0; i < simplex_tableu_.basis_variables_indexes.size(); i++) {
        ans.point[simplex_tableu_.basis_variables_indexes[i]] =
            simplex_tableu_.table[i][ans.point.size()];
    }
    for (i = 0; i < target_function_.function.size(); i++)
        ans.value += ans.point[i] * target_function_.function[i].second;
    return ans;
}