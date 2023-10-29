#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include <filesystem>
#include <string>
#include <vector>

#include "lib_simplex_io/read_file.hpp"

class SimplexMethodSolver
{
  public:
   SimplexMethodSolver(const std::filesystem::path& tablePath);
   std::string getSimplexTableuStr();
   std::string getProccessedFunction();
   void doNextStep();
   void solve();
   ~SimplexMethodSolver() {}

  private:
   typedef std::vector<std::pair<std::string, float>> function_repr;
   typedef std::vector<
     std::pair<std::vector<float>, lib_simplex_io::ConstraintType>>
     constraints_repr;

   function_repr input_function_;
   function_repr processed_function_;
   constraints_repr constraints_;
   std::vector<std::vector<float>> simplex_tableu_;
   lib_simplex_io::MinMaxType input_function_type_;
};

#endif // FUNCTION_HPP
