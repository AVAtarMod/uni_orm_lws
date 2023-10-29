#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include <filesystem>
#include <string>
#include <vector>

class SimplexMethodSolver
{
  public:
   enum MinMaxType
   {
      MIN,
      MAX
   };
   enum ConstraintType
   {
      NONE,
      LESSER_OR_EQUAL,
      GREATER_OR_EQUAL,
      EQUAL,
   };

   SimplexMethodSolver(const std::filesystem::path& tablePath);
   std::string getSimplexTableuStr();
   std::string getProccessedFunction();
   void doNextStep();
   void solve();
   ~SimplexMethodSolver() {}

  private:
   typedef std::vector<std::pair<std::string, float>> function_repr;
   typedef std::vector<std::pair<std::vector<float>, ConstraintType>>
     constraints_repr;

   function_repr input_function_;
   function_repr processed_function_;
   constraints_repr constraints_;
   std::vector<std::vector<float>> simplex_tableu_;
   MinMaxType input_function_type_;
};

#endif // FUNCTION_HPP
