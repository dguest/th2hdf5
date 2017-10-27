#include "convert_tree.hh"
#include "unshittify.hh"

#include "H5Cpp.h"
#include "TFile.h"

void usage(const char* prog) {
  printf("usage: %s <root-file> <output-file-name>\n", prog);
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    usage(argv[0]);
    return 1;
  }
  unshittify();
  TFile* root_file = TFile::Open(argv[1], "read");
  H5::H5File hdf_file(argv[2], H5F_ACC_TRUNC);
  convert_tree(*root_file, hdf_file);
  return 0;
}
