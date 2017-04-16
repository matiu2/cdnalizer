/// Makes an iterator derived from Abstract Block Iterator, and makes sure it is fast

#include "AbstractBlockIterator.hpp"

#include <list>
#include <string>
#include <iterator>
#include <algorithm>
#include <iostream>

using List = std::list<std::string>;

struct Block {
  List& list;
  List::iterator iter;
  Block(List &list, std::string data = {})
      : list(list), iter() {
    list.emplace_back(std::move(data));
    iter = list.end();
    --iter;
  }
  Block(List &list, List::iterator iter) : list(list), iter(iter) {}
  std::string& data() { return *iter; }
  Block next() {
    Block result(list, iter);
    ++result.iter;
    return result;
  }
  std::string::iterator begin() const { return iter->begin(); }
  std::string::iterator end() const { return iter->end(); }
  Block operator++(int) { return next(); }
  Block& operator++() { ++iter; return *this; }
  bool isSentinel() const { return iter == list.end(); }
  bool operator==(const Block& other) const {
    return (other.list == list) && (other.iter == iter);
  }
};


using cdnalizer::apache::AbstractBlockIterator;

struct Iterator : AbstractBlockIterator<std::string::iterator, Block> {
  Iterator(Block block, std::string::iterator location)
      : AbstractBlockIterator(block, location) {}
  Iterator(Block block) : AbstractBlockIterator(block, block.data().begin()) {}
};

void testSmallBlocks() {
  // Copy the string into 3 character blocks
  const std::string input(
      R"--(junky bits <A boolean style="background-image: url('/images/happy.jpg'); filter: url('/images/filter.css');" check_something_else>click here</a>)--");
  List blocks;
  auto in = input.begin();
  size_t i = 0;
  constexpr size_t blockSize(3);
  std::string written;
  while (i < input.size())  {
    Block out(blocks);
    auto inc = std::min(blockSize, input.size() - i);
    std::copy_n(in, inc, std::back_inserter(out.data()));
    std::copy_n(in, inc, std::back_inserter(written));
    i += inc;
    in += inc;
  }
  // Now see if we can print it out
  std::string test_out;
  Iterator test_in(Block(blocks, blocks.begin()), blocks.front().begin());
  Iterator end_in(Block(blocks, blocks.end()));
  std::copy(test_in, end_in, std::back_inserter(test_out));

  using namespace std;
  cout << "Original data:\n    " << input << "\nCopied data:\n    " << written
       << "\nRecreated data:\n    " << test_out << std::endl;
}

int main(int, char **) { 
  testSmallBlocks();
  return 0;
}
