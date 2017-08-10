#ifndef CONVERT_TREE_HH
#define CONVERT_TREE_HH

class TDirectoryFile;

namespace H5 {
  class CommonFG;
}

void convert_tree(const TDirectoryFile& td, H5::CommonFG& hg);

#endif
