Root to HDF5 Histogram Converter
================================

This is an easy way to get your histograms into numpy arrays without
having to mess around with pyroot.

Usage
-----

```bash
th2hdf5 root_file.root new_hdf5_file.h5
```

Creates one new "group" for each histogram:

 - Group attribute: `hist_type`
 - Bin values are stored in `values` dataset, including under and overflow
 - Bin edges are stored in `edges` dataset

