#include "function.hpp"

#include <bitset>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <variant>
#include <vector>

struct IOChar
{
   char data = '\0';
   bool success = false;
};

class CharType
{
  public:
   typedef int enum_base_type;
   enum Types : enum_base_type
   {
      DIGIT = 0,
      MINUS,
      PLUS,
      DOT,
      EQUAL,
      LESS,
      GREATER,
      COMMA,
      UNDERSCORE,
      ALPHA,
      UNKNOWN,
   };
   CharType() {}
   CharType(char data)
   {
      if (isdigit(data))
         types = DIGIT;
      else if (data == '-')
         types = MINUS;
      else if (data == '+')
         types = PLUS;
      else if (data == '.')
         types = DOT;
      else if (data == '=')
         types = EQUAL;
      else if (data == '<')
         types = LESS;
      else if (data == '>')
         types = GREATER;
      else if (data == ',')
         types = COMMA;
      // else if (data == '_')
      //    types = UNDERSCORE;
      // else if (isalpha(data))
      //    types = ALPHA;
   }
   bool isSignChar() const { return types == MINUS || types == PLUS; }
   bool isRecognized() const { return types != UNKNOWN; }
   bool operator==(Types t) const { return types == t; }
   bool operator==(enum_base_type t) const { return types == t; }

  private:
   Types types = UNKNOWN;
};

IOChar try_get(std::istream& in)
{
   IOChar result;
   in.get(result.data);
   if (in.fail() || in.gcount() == 0)
      result.success = false;
   else
      result.success = true;
   return result;
}

std::string extractBufferContent(const std::string& buffer)
{
   std::stringstream stream;
   for (auto&& i : buffer) {
      if (i != '\0')
         stream << i;
   }
   return stream.str();
}

struct Coefficient
{
   double val;
   Coefficient(double value = 0) : val(value) {};
};
struct Variable
{
   std::string_view label;
};
enum ParserState
{
   READ_NULL,
   READ_COEFF = 1,
   READ_VARIABLE = (1 << 1),
   READ_MIN = (1 << 2),
   READ_MAX = (1 << 3),
   READ_CONSTRAINT = (1 << 4),
   READ_COMMENT = (1 << 5),
   READ_COMMA = (1 << 6),
};
typedef std::variant<Coefficient, Variable,
                     SimplexMethodSolver::MinMaxType,
                     SimplexMethodSolver::ConstraintType, ParserState>
  ParsedSymbol;

std::string_view min_str = "->min", max_str = "->max";
enum ReturnAction
{
   RA_SUCCESS,
   RA_ERROR_UNKNOWN_SYMBOL,
   RA_LOOP_CONTINUE,
   RA_LOOP_BREAK,
};

ReturnAction parseCoef(const IOChar& input, ParserState& s,
                       size_t& digit_count, size_t& dot_count)
{
   ReturnAction action = RA_SUCCESS;
   bool valid = true;
   CharType t = { input.data };

   if (s == READ_COEFF) {
      if (t == CharType::DIGIT)
         ++digit_count;
      else if (t == CharType::DOT) {
         if (dot_count == 0)
            ++dot_count;
         else
            valid = false;
      } else
         valid = false;
   } else {
      if (t == CharType::DIGIT) {
         s = READ_COEFF;
         ++digit_count;
      } else
         action = RA_ERROR_UNKNOWN_SYMBOL;
   }
   if (!valid)
      action = RA_LOOP_BREAK;
   return action;
}
ReturnAction parseConstraint(const IOChar& input, ParserState& s)
{
   // Read last part of "<=" or ">="
   if (input.data == '=')
      return RA_SUCCESS;
   else
      return RA_ERROR_UNKNOWN_SYMBOL;
}
ReturnAction parseMinmax(const IOChar& input, ParserState& s,
                         size_t& minmax_index)
{
   ReturnAction action = RA_SUCCESS;
   bool valid = true;
   CharType t = { input.data };

   if (s & (READ_MIN | READ_MAX) &&
       (s & (~(READ_MIN | READ_MAX))) == 0) {
      if (s == (READ_MIN | READ_MAX)) {
         if (input.data == min_str[minmax_index] &&
             input.data == max_str[minmax_index])
            s == READ_MIN | READ_MAX;
         else if (input.data == min_str[minmax_index])
            s = READ_MIN;
         else if (input.data == max_str[minmax_index])
            s = READ_MAX;
         else
            valid = false;
         if (valid)
            ++minmax_index;
      } else if (s == READ_MIN && input.data == min_str[minmax_index])
         ++minmax_index;
      else if (input.data == max_str[minmax_index]) {
         ++minmax_index;
      } else
         valid = false;
   } else {
      if (t == CharType::GREATER) {
         s = static_cast<ParserState>(READ_MIN | READ_MAX);
         // in next call we expect input.data = 'm', so set index = 2
         minmax_index = 2;
      } else
         action = RA_ERROR_UNKNOWN_SYMBOL;
   }
   if (!valid)
      action = RA_LOOP_BREAK;
   return action;
}
void filterSpaces(std::string& str)
{
   size_t size = str.size(), null_count = 0;
   for (size_t i = 0; i < size; ++i) {
      if (isspace(str[i])) {
         str.erase(i, 1);
         --i;
         --size;
      } else if (str[i] == '\0') {
         if (null_count == 0)
            ++null_count;
         else {
            str.resize(i - 1);
            break;
         }
      }
   }
}
ParsedSymbol construct(const ParserState& s, std::string& buffer)
{
   filterSpaces(buffer);
   if (s == READ_COEFF) {
      return Coefficient(std::stod(buffer));
   } else if (s == READ_MIN || s == READ_MAX) {
      if (buffer == min_str)
         return SimplexMethodSolver::MIN;
      else if (buffer == max_str)
         return SimplexMethodSolver::MAX;
   } else if (s == READ_CONSTRAINT) {
      if (buffer == ">=")
         return SimplexMethodSolver::GREATER_OR_EQUAL;
      else if (buffer == "<=")
         return SimplexMethodSolver::LESSER_OR_EQUAL;
      else if (buffer == "=")
         return SimplexMethodSolver::EQUAL;
   }
   return s;
}
ParsedSymbol next_symbol(std::iostream& in,
                         const size_t buffer_size = 255)
{
   std::string buffer, tmp;
   buffer.resize(buffer_size);
   tmp.resize(buffer_size);

   ParsedSymbol result;
   ParserState s = READ_NULL;
   bool valid = true, parsingCoefValue = false;
   ReturnAction action = RA_ERROR_UNKNOWN_SYMBOL;

   std::fill(buffer.begin(), buffer.end(), '\0');
   size_t minmax_index = 0, digit_count = 0, dot_count = 0;
   for (size_t i = 0; i < buffer_size; ++i) {
      bool isCharParsed = false;
      IOChar input = try_get(in);
      if (!input.success) {
         break;
      }
      if (input.data == '#') {
         s = READ_COMMENT;
         in.getline(tmp.data(), buffer_size);
         break;
      }

      if (isspace(input.data)) {
         isCharParsed = true;
         if (s == READ_COEFF && parsingCoefValue)
            break;
         if (s == READ_COMMA)
            break;
      }

      if (!isCharParsed && s == READ_NULL) {
         CharType t(input.data);
         if (t == CharType::DIGIT) {
            s = READ_COEFF;
            parsingCoefValue = true;
         }
         if (t == CharType::MINUS) {
            s = static_cast<ParserState>(READ_COEFF | READ_MIN |
                                         READ_MAX);
         } else if (t == CharType::PLUS) {
            s = READ_COEFF;
         } else if (t == CharType::GREATER || t == CharType::LESS ||
                    t == CharType::EQUAL) {
            s = READ_CONSTRAINT;
         } else if (t == CharType::COMMA) {
            s = READ_COMMA;
         }
         if (s != READ_NULL)
            isCharParsed = true;
      }
      if (!isCharParsed) {
         switch (s) {
            case READ_COEFF:
               action = parseCoef(input, s, digit_count, dot_count);
               if (action == RA_SUCCESS) {
                  parsingCoefValue = true;
               } else if (input.data == ',') {
                  in.unget();
                  action = RA_SUCCESS;
               }
               break;
            case READ_MIN | READ_MAX:
            case READ_MIN:
            case READ_MAX:
               action = parseMinmax(input, s, minmax_index);
               break;
            case READ_CONSTRAINT:
               action = parseConstraint(input, s);
               break;
            case READ_COEFF | READ_MIN | READ_MAX:
               action = parseCoef(input, s, digit_count, dot_count);
               if (action == RA_ERROR_UNKNOWN_SYMBOL) {
                  action = parseMinmax(input, s, minmax_index);
               } else if (action == RA_SUCCESS)
                  parsingCoefValue = true;
               break;
            default:
               break;
         }
         if (action != RA_SUCCESS)
            valid = false;
         if (action == RA_LOOP_BREAK)
            break;
      }
      if (s == READ_COEFF && input.data == ',')
         break;
      buffer[i] = input.data;
      if (s == READ_MIN && minmax_index >= min_str.size())
         break;
      else if (s == READ_MAX && minmax_index >= max_str.size())
         break;
      else if (s == READ_CONSTRAINT && input.data == '=')
         break;
      else if (s == READ_COMMA)
         break;
   }
   if (!valid) {
      std::string msg;
      buffer = extractBufferContent(buffer);
      msg = "'" + buffer + "',char index=" +
            std::to_string(in.tellg() % buffer.size()) +
            ". Syntax error.";

      throw std::runtime_error(msg);
   }
   result = construct(s, buffer);
   return result;
}

constexpr bool isCommentLine(std::string_view line)
{
   auto pos = line.find_first_of('#');
   if (pos == line.npos)
      return false;
   else {
      for (size_t i = 0; i < pos; ++i) {
         if (!isspace(line[i]))
            return false;
      }
   }
   return true;
}

constexpr bool isEmptyLine(std::string_view line)
{
   for (size_t i = 0; i < line.size(); ++i) {
      if (!isspace(line[i]))
         return false;
   }
   return true;
}

std::string next_line(std::istream& in, const size_t line_length)
{
   std::string result;
   while (true) {
      result.resize(line_length);
      std::fill(result.begin(), result.end(), '\0');
      in.getline(result.data(), line_length, '\n');
      if (in.fail() || in.gcount() == 0)
         throw std::runtime_error(
           "Cannot read line, EOF reached or IO error happened");
      result = extractBufferContent(result);
      if (!isCommentLine(result) && !isEmptyLine(result))
         break;
   }

   return result;
};

struct ParsedFunction
{
   std::vector<std::pair<std::string, float>> function;
   SimplexMethodSolver::MinMaxType function_type;
   bool success = true;
};

struct ParsedConstraint
{
   typedef std::vector<float> SingleConstraint;
   SingleConstraint constraint;
   SimplexMethodSolver::ConstraintType constraint_type =
     SimplexMethodSolver::NONE;
   bool success = true;
   bool is_last = false;
};
constexpr std::string_view label_prefix = "x_";
std::string getLabelByIndex(const size_t index)
{
   return label_prefix.data() + std::to_string(index);
}

ParsedFunction readFunction(std::istream& in,
                            const size_t buffer_size = 255)
{
   ParsedFunction result;
   std::stringstream buffer;
   constexpr size_t expected_label_length = 3;
   size_t var_index = 0;
   std::string label;
   label.reserve(expected_label_length);
   try {
      buffer.str(next_line(in, buffer_size));
   } catch (const std::exception& e) {
      result.success = false;
      return result;
   }
   do {
      ParsedSymbol s = next_symbol(buffer, buffer_size);
      switch (s.index()) {
         case 0:
            label = getLabelByIndex(var_index);
            result.function.push_back({ label, std::get<0>(s).val });
            ++var_index;
            break;
         case 1:
            result.function[var_index].first = std::get<1>(s).label;
            break;
         case 2:
            result.function_type = std::get<2>(s);
            if (buffer)
               buffer.setstate(std::ios::eofbit);
            break;
         case 4:
            if (std::get<4>(s) == READ_COMMENT) {
               continue;
            } else {
               if (buffer)
                  buffer.setstate(std::ios::eofbit);
            }
            break;
         default:
            break;
      }
   } while (buffer);

   return result;
}

size_t indexFromLabel(std::string_view label)
{
   return std::atoi(label.begin() + label_prefix.size());
}

ParsedConstraint readConstraint(std::istream& in,
                                const size_t buffer_size = 255)
{
   ParsedConstraint result;
   std::stringstream buffer;
   constexpr size_t expected_label_length = 3;
   size_t var_index = 0, comma_count = 0;
   std::string label;
   label.reserve(expected_label_length);
   bool isTypeParsed = false, isParsingCompleted = false;
   double current_coef_val = 0;

   try {
      buffer.str(next_line(in, buffer_size));
   } catch (const std::exception& e) {
      result.success = false;
      return result;
   }
   do {
      size_t tmp_index;
      ParsedSymbol s = next_symbol(buffer, buffer_size);
      switch (s.index()) {
         case 0:
            current_coef_val = std::get<0>(s).val;
            result.constraint.push_back(current_coef_val);
            if (isTypeParsed)
               isParsingCompleted = true;
            ++var_index;
            break;
         case 1:
            tmp_index = indexFromLabel(std::get<1>(s).label);
            if (tmp_index >= result.constraint.size())
               result.constraint.resize(tmp_index);
            result.constraint[tmp_index] = current_coef_val;
            break;
         case 3:
            result.constraint_type = std::get<3>(s);
            isTypeParsed = true;
            break;
         case 4:
            if (std::get<4>(s) == READ_COMMENT) {
               if (!isParsingCompleted) {
                  result.success = false;
                  buffer.setstate(std::ios::eofbit);
               }
            }
            if (std::get<4>(s) == READ_COMMA) {
               result.is_last = true;
               ++comma_count;
            } else {
               buffer.setstate(std::ios::eofbit);
            }
            break;
         default:
            break;
      }
      auto count_diff =
        static_cast<long>(var_index) - static_cast<long>(comma_count);
      if (result.is_last && (count_diff > 1 || count_diff < 0)) {
         result.success = false;
         buffer.setstate(std::ios::eofbit);
      }
   } while (buffer);

   return result;
}

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
   if (c.constraint_type != SimplexMethodSolver::GREATER_OR_EQUAL)
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
            input_function_ = f.function;
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
