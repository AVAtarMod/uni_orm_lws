#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include <filesystem>
#include <string>
#include <vector>

#include "simplex_io/read_data.hpp"
#include "simplex_io/write_data.hpp"
#include "simplex_io/conversion.hpp"

struct SimplexMethodStatus
{
	bool is_end, is_infinity;
};
struct SimplexMethodAnswer
{
	float value;
	std::vector<float> point;
	bool is_valid, is_infinity;
};

class SimplexMethodSolver
{
public:
	SimplexMethodSolver(const std::filesystem::path& tablePath);
	std::string getSimplexTableuStr();
	std::string getProccessedFunction();
	SimplexMethodStatus doNextStep();
	SimplexMethodAnswer solve();
	~SimplexMethodSolver() {}

private:
	simplex_io::FunctionBase target_function_;
	simplex_io::Problem p;
	simplex_io::SimplexTableau simplex_tableu_;

	void createSimplexTableu(const std::vector<size_t>& basis_var_ind);
};

#endif // FUNCTION_HPP