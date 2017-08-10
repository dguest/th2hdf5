#include "convert_tree.hh"

#include "H5Cpp.h"
#include "TFile.h"
#include "TKey.h"
#include "TH1.h"

#include <set>

#include <iostream>

namespace {
  void h5_hist_from_th1(const TH1*, H5::CommonFG& fg);
  H5::DataType packed(H5::DataType);

  // type of group that represents a histogram
  enum HistType {HISTOGRAM};
}

void convert_tree(const TDirectoryFile& td, H5::CommonFG& fg) {
  // build a list of keys (this is to avoid asking for the same key
  // with differn cycle numbers)
  std::set<std::string> keys;
   TIter next(td.GetListOfKeys());
   TKey *key;
   while ((key = dynamic_cast<TKey*>(next()))) {
     keys.insert(key->GetName());
   }
   for (const auto key_name: keys) {
     key = td.GetKey(key_name.c_str());
     const TObject* obj = key->ReadObj();

     const TDirectoryFile* dir = dynamic_cast<const TDirectoryFile*>(obj);
     if (dir) {
       auto group = fg.createGroup(dir->GetName());
       convert_tree(*dir, group);
       continue;
     }

      const TH1 *hist = dynamic_cast<const TH1*>(obj);
      if (hist) h5_hist_from_th1(hist, fg);
   }
}


namespace {

  struct bin_t {
    double value;
    double error;
    double lower_edge;
    double upper_edge;
  };

  H5::CompType get_bin_type() {
    const auto dt = H5::PredType::NATIVE_DOUBLE;
    H5::CompType type(sizeof(bin_t));
#define INSERT(name) type.insertMember(#name, offsetof(bin_t, name), dt)
    INSERT(value);
    INSERT(error);
    INSERT(lower_edge);
    INSERT(upper_edge);
#undef INSERT
    return type;
  }

  H5::EnumType get_group_type() {
    H5::EnumType btype(sizeof(HistType));

    HistType histogram = HISTOGRAM;
    btype.insert("HISTOGRAM", &histogram);

    return btype;
  }
  void write_group_type(H5::Group& group, HistType type) {
    auto gtype = get_group_type();
    auto attr = group.createAttribute("hist_type", gtype, H5S_SCALAR);
    attr.write(gtype, &type);
  }

  void add_bins(H5::Group& group, const std::vector<bin_t>& bins) {
    hsize_t extent[] = {bins.size()};
    H5::DataSpace dspace(1, extent);
    H5::CompType type = get_bin_type();
    H5::DSetCreatPropList params;
    params.setChunk(1, extent);
    params.setDeflate(9);
    auto dset = group.createDataSet("bins", packed(type), dspace, params);
    dset.write(bins.data(), type);
  }
  void add_dvector(H5::Group& group, const std::vector<double>& vec,
                   const std::string& name) {
    hsize_t extent[] = {vec.size()};
    H5::DataSpace space(1, extent);
    auto dtype = H5::PredType::NATIVE_DOUBLE;
    H5::DSetCreatPropList params;
    params.setChunk(1, extent);
    params.setDeflate(9);
    auto dset = group.createDataSet(name, dtype, space, params);
    dset.write(vec.data(), dtype);
  }

  void h5_hist_from_th1(const TH1* hist, H5::CommonFG& fg) {
    // check that name isn't already taken
    std::string hist_name = hist->GetName();
    if (H5Lexists(fg.getLocId(), hist_name.c_str(), H5P_DEFAULT)) {
      throw std::logic_error(hist_name + " inserted twice");
      return;
    }
    if (hist->GetNbinsY() != 1 || hist->GetNbinsZ() != 1) {
      throw std::logic_error(hist_name + " has y or z bins");
    }

    // build bins vector
    const hsize_t n_bins = hist->GetNbinsX() + 2; // 2 extra for overflow
    std::vector<bin_t> bins;
    for (int bin_number = 0; bin_number < n_bins; bin_number++) {
      bin_t bin;
      bin.value = hist->GetBinContent(bin_number);
      bin.error = hist->GetBinError(bin_number);
      bin.lower_edge = hist->GetXaxis()->GetBinLowEdge(bin_number);
      bin.upper_edge = hist->GetXaxis()->GetBinUpEdge(bin_number);
      bins.push_back(bin);
    }

    std::vector<double> edges;
    std::vector<double> values;
    for (int bin_number = 0; bin_number < n_bins; bin_number++) {
      const auto& bin = bins.at(bin_number);
      if (bin_number != 0) edges.push_back(bin.lower_edge);
      values.push_back(bin.value);
    }

    // build group
    auto group = fg.createGroup(hist_name);
    write_group_type(group, HISTOGRAM);

    // fill bin values
    add_dvector(group, values, "values");
    // fill the edges
    add_dvector(group, edges, "edges");
  }

  H5::DataType packed(H5::DataType in) {
    // TODO: Figure out why a normal copy constructor doesn't work here.
    //       The normal one seems to create shallow copies.
    auto out = H5::CompType(H5Tcopy(in.getId()));
    out.pack();
    return out;
  }

}
