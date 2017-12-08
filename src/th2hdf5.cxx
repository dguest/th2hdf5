#include "convert_tree.hh"
#include "unshittify.hh"

#include "H5Cpp.h"
#include "TFile.h"

#include <vector>
#include <regex>

void usage(const char* prog) {
  printf("usage: %s [-h] <root-file> <output-file-name> [regex...]\n", prog);
}
void help(const char* prog) {
  usage(prog);
  puts("\n"
       "Convert a root file containing histograms into an HDF5 file\n"
       "\n"
       "Subdirectories can be selected with regular expressions: the first\n"
       "regex is applied to the top level of root directories within the\n"
       "file, the second to the second level of directories, and so on");
}

int main(int argc, char* argv[]) {
  for (int argn = 0; argn < argc; argn++) {
    if (strcmp("-h", argv[argn]) == 0) {
      help(argv[0]);
      return 1;
    }
  }
  if (argc < 3) {
    usage(argv[0]);
    return 1;
  }
  std::vector<std::regex> regexes;
  for (int argn = 3; argn < argc; argn++) {
    regexes.emplace_back(argv[argn]);
  }
  unshittify();
  TFile* root_file = TFile::Open(argv[1], "read");
  H5::H5File hdf_file(argv[2], H5F_ACC_TRUNC);
  convert_tree(*root_file, hdf_file, regexes);
  return 0;
}
