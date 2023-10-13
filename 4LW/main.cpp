#include "function.hpp"
#include <filesystem>
#include <iostream>
#include <string>

static const std::filesystem::path tablePath = "char_table.txt";

int main(int argc, char const* argv[])
{
   bool noData = false;
   Tokenizer t(tablePath);

   if (argc == 1) {
      std::cout << "Menu:\n"
                << "\tPrint grammar table\n"
                << "\tCheck input sequence in language\n"
                << "Enter action number: ";
      argc++;
      noData = true;
   }
   for (int i = 1; i < argc; i++) {
      int choice;
      if (noData) {
         std::cin >> choice;
         std::cin.ignore(32767, '\n');
      } else {
         choice = std::stoi(argv[i]);
      }

      switch (choice) {
         case 1:
            std::cout << "Table:\n";
            std::cout << t.getTable() << '\n';
            std::cout << "-----\n";
            break;
         default:
            std::cerr << "ERROR: Incorrect action number (" << choice
                      << ")!\n";
            break;
      }
   }
   return 0;
}
