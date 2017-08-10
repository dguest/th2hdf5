#include "convert_tree.hh"

#include "H5Cpp.h"
#include "TFile.h"
#include "TKey.h"
#include "TH1.h"

#include <iostream>

namespace {
  void h5_hist_from_th1(const TH1*, H5::CommonFG& fg);
}

void convert_tree(const TDirectoryFile& td, H5::CommonFG& fg) {
   TIter next(td.GetListOfKeys());
   TKey *key;
   while ((key = dynamic_cast<TKey*>(next()))) {
      // TClass *cl = gROOT->GetClass(key->GetClassName());
      // if (!cl->InheritsFrom("TH1")) continue;
     std::string key_name = key->GetName();
     const TObject* obj = key->ReadObj();

     const TDirectoryFile* dir = dynamic_cast<const TDirectoryFile*>(obj);
     if (dir) {
       auto group = fg.createGroup(dir->GetName());
       convert_tree(*dir, group);
       continue;
     }

      const TH1 *hist = dynamic_cast<const TH1*>(obj);
      if (!hist) continue;
      h5_hist_from_th1(hist, fg);
   }
}


namespace {

  struct bin_t {
    double value;
    double error;
    double upper_edge;
    double lower_edge;
  };

  H5::CompType get_bin_type() {
    const auto dt = H5::PredType::NATIVE_DOUBLE;
    H5::CompType type(sizeof(bin_t));
#define INSERT(name) type.insertMember(#name, offsetof(bin_t, name), dt)
    INSERT(value);
    INSERT(error);
    INSERT(upper_edge);
    INSERT(lower_edge);
#undef INSERT
    return type;
  }

  void h5_hist_from_th1(const TH1* hist, H5::CommonFG& fg) {
    // build bins vector
    const hsize_t n_bins = hist->GetNbinsX() + 2; // 2 extra for overflow
    std::vector<bin_t> bins;
    for (int bin_number = 0; bin_number < n_bins; bin_number++) {
      bin_t bin;
      bin.value = hist->GetBinContent(bin_number);
      bin.error = hist->GetBinError(bin_number);
      bin.upper_edge = hist->GetXaxis()->GetBinUpEdge(bin_number);
      bin.lower_edge = hist->GetXaxis()->GetBinLowEdge(bin_number);
      bins.push_back(bin);
    }

    // make data type
    H5::CompType type = get_bin_type();

    // make dataset
    std::string hist_name = hist->GetName();
    hsize_t extent[1] = {n_bins};
    H5::DataSpace dspace(1, extent);
    H5::DSetCreatPropList params;
    params.setChunk(1, extent);
    params.setDeflate(7);
    auto dset = fg.createDataSet(hist_name, type, dspace, params);
    dset.write(bins.data(), type);
  }
}
