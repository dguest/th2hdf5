#include "convert_tree.hh"
#include "unshittify.hh"

#include "H5Cpp.h"
#include "TFile.h"

int main(int argc, char* argv[]) {
  if (argc <= 2) return 1;
  unshittify();
  TFile root_file(argv[1], "read");
  H5::H5File hdf_file(argv[2], H5F_ACC_TRUNC);
  convert_tree(root_file, hdf_file);
  return 0;
}
