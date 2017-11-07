#ifndef CONVERT_TREE_HH
#define CONVERT_TREE_HH

class TDirectoryFile;

namespace H5 {
  class Group;
}

#include <vector>
#include <regex>

void convert_tree(const TDirectoryFile& td, H5::Group& hg,
                  std::vector<std::regex> regexes = {});

#endif
