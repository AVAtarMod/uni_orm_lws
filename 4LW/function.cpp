#include "function.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

struct IOChar
{
   char data = '\0';
   bool success = false;
};

class CharType
{
  public:
   bool digit = false;
   bool minus = false;
   bool plus = false;
   bool dot = false;

   CharType() {}
   CharType(char data)
   {
      if (data == '.')
         dot = true;
      if (data == '-')
         minus = true;
      if (data == '+')
         plus = true;
      if (isdigit(data))
         digit = true;
   }
   bool isSignChar() const { return minus || plus; }
   bool isRecognized() const { return digit || minus || plus || dot; }
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

double next_number(std::istream& in)
{
   const size_t BUFFER_SIZE = 255;
   std::string tmp;
   tmp.resize(BUFFER_SIZE);

   double result;
   while (true) {
      IOChar input = try_get(in);
      if (!input.success)
         throw std::runtime_error("File is empty or unknown error");
      if (input.data == '#') {
         in.getline(tmp.data(), BUFFER_SIZE, '\n');
         continue;
      }

      CharType t;
      size_t dot_count = 0, sign_char_count = 0, digit_count = 0;
      std::fill(tmp.begin(), tmp.end(), '\0');
      for (size_t i = 0; i < BUFFER_SIZE; ++i) {
         if (!input.success)
            break;

         t = { input.data };
         if (!t.isRecognized())
            break;

         if (t.isSignChar())
            ++sign_char_count;
         else if (t.digit)
            ++digit_count;
         else if (t.dot)
            ++dot_count;

         if (dot_count > 1 || sign_char_count > 1)
            break;

         tmp[i] = input.data;
         input = try_get(in);
      }
      if (digit_count == 0) {
         tmp = extractBufferContent(tmp);
         std::string msg =
           "'" + tmp + "',pos=" +
           std::to_string(std::abs(in.tellg()) - tmp.size()) +
           ". End of file reached before end of number.";
         throw std::runtime_error(msg);
      }
      result = std::stod(tmp);
      break;
   }
   return result;
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
      if (result.find_first_of('#') == result.npos &&
          result != "\n" && result != "")
         break;
   }

   return result;
};

std::map<std::string, size_t> readTable(
  const std::filesystem::path& p)
{
   std::ifstream in(p);
   std::map<std::string, size_t> result;
   std::stringstream buffer;
   constexpr size_t BUFFER_SIZE = 255;

   if (in) {
      while (in) {
         buffer.clear();
         try {
            buffer.str(next_line(in, BUFFER_SIZE));
         } catch (const std::exception& e) {
            break;
         }
         std::string token, cb = buffer.str();
         size_t code;
         buffer >> token >> code;
         result[token] = code;
      }
   } else {
      std::cerr << "[ERROR] Cannot open file " << p.string() << '\n';
   }
   return result;
}

Tokenizer::Tokenizer(const std::filesystem::path& tablePath) :
  table(readTable(tablePath))
{
}

std::string Tokenizer::getTable()
{
   std::string result = "token | code\n";
   std::vector<std::pair<std::string, size_t>> sorted_table;
   sorted_table.reserve(table.size());
   for (auto&& i : table) {
      sorted_table.push_back(i);
   }

   for (auto&& i : sorted_table) {
      result += (i.first + " | " + std::to_string(i.second)) + '\n';
   }
   return result;
}

Tokenizer::~Tokenizer() {}
