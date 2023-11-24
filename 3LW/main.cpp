#include "function.hpp"
#include <iostream>
#include <string>

std::filesystem::path table_path = "table1.txt";

int main(int argc, char const* argv[])
{
   bool noData = false;
   SimplexMethodSolver s(table_path);
   auto a = s.solve();
   std::cout << a.value << std::endl;
   for (int i = 0; i < a.point.size(); i++)
       std::cout << a.point[i] << ' ';
   std::cout << std::endl;

   std::string desc_1 =
     "1. Construct II phase function and first simplex tableau";
   if (argc == 1) {
      std::cout << desc_1 << "\nEnter task number to run: ";
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
            std::cout << desc_1 << "\n";
            std::cout << "function: " << s.getProccessedFunction()
                      << '\n';
            std::cout << "table:\n"
                      << s.getSimplexTableuStr() << '\n';
            std::cout << "-----\n";
            break;
         default:
            std::cerr
              << "ERROR: Incorrect task number or task not exist\n";
            break;
      }
   }
   return 0;
}
