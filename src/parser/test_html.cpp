/// Tests the html parser
#include "html.hpp"
#include <fstream>

int main(int argc, char *argv[]) {
  std::ifstream in_file("index.html");
  std::ifstream expected_file("index.html.expected");

  using cdnalizer::parser::parseHTML;


  
  return 0;
}
