#include "function.hpp"
#include <iostream>
#include <string>

int main(int argc, char const *argv[])
{
   bool noData = false;
   if (argc == 1)
   {
      std::cout << "Enter task number to run: ";
      argc++;
      noData = true;
   }
   for (int i = 1; i < argc; i++)
   {
      int choice;
      if (noData)
      {
         std::cin >> choice;
         std::cin.ignore(32767, '\n');
      }
      else
      {
         choice = std::stoi(argv[i]);
      }

      switch (choice)
      {
      case 1:
         std::cout << "Task #1\n";
         task1();
         std::cout << "-----\n";
         break;
      default:
         std::cerr << "ERROR: Incorrect task number or task not exist\n";
         break;
      }
   }
   return 0;
}
