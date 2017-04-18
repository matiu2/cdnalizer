/// Makes an iterator derived from Abstract Block Iterator, and makes sure it is fast

#include "AbstractBlockIterator.hpp"
#include "../Config.hpp"
#include "../Rewriter.hpp"
#include "../Rewriter_impl.hpp"

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
  Block prev() {
    Block result(list, iter);
    --result.iter;
    return result;
  }
  std::string::iterator begin() const { return iter->begin(); }
  std::string::iterator end() const { return iter->end(); }
  Block operator++(int) { return next(); }
  Block operator--(int) { return prev(); }
  Block &operator++() {
    ++iter;
    return *this;
  }
  Block &operator--() {
    --iter;
    return *this;
  }
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
  constexpr size_t blockSize(2);
  std::string copied;
  while (i < input.size())  {
    Block out(blocks);
    auto inc = std::min(blockSize, input.size() - i);
    std::copy_n(in, inc, std::back_inserter(out.data()));
    std::copy_n(in, inc, std::back_inserter(copied));
    i += inc;
    in += inc;
  }

  // Now see if we can print it out
  std::string test_out;
  Iterator test_in(Block(blocks, blocks.begin()), blocks.front().begin());
  Iterator end_in(Block(blocks, blocks.end()));
  std::copy(test_in, end_in, std::back_inserter(test_out));

  using namespace std;
  cout << "Original data:\n    " << input << "\nCopied data:\n    " << copied
       << "\nRecreated data:\n    " << test_out << std::endl;

  assert(input == copied);
  assert(input == test_out);

  // Test random access
  auto start = test_in;
  cout << "start: " << *start << endl;
  assert(*start == 'j');
  auto A = start + 12;
  cout << "A: " << *A << endl;
  assert(*A == 'A');
  auto lt = A - 1;
  cout << "lt: " << *lt << endl;
  assert(*lt == '<');
  std::string bits;
  auto b = A - 6;
  std::copy_n(b, 4, std::back_inserter(bits));
  cout << "bits: " << bits << endl;
  assert(bits == "bits");
  auto s = std::make_reverse_iterator(A - 2);
  std::string stib;
  std::copy(s, make_reverse_iterator(b), std::back_inserter(stib));
  cout << "stib: " << stib << endl;

  // Now see if we can parse it
  /*
  std::string location("/blog");
  cdnalizer::Config cfg;
  auto unchanged = [&](
  cdnalizer::rewriteHTML(location, cfg, test_in, end_in, unchanged, newData,
                                isCSS);
  */
}

int main(int, char **) { 
  testSmallBlocks();
  return 0;
}
