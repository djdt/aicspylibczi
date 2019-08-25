//
// Created by Jamie Sherman on 7/11/19.
//

#ifndef PYLIBCZI_AICS_ADDED_HPP
#define PYLIBCZI_AICS_ADDED_HPP

#include <cstdio>
#include <iostream>
#include <typeinfo>
#include <exception>
#include <functional>
#include <vector>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
//#include <numpy/ndarraytypes.h>

#include "inc_libCZI.h"
#include "Python.h"
#include "IndexMap.h"

using namespace std;

namespace py = pybind11;

// extern PyObject *PylibcziError; add back in when I split out into a cpp and hpp file
namespace pylibczi {

/// <summary>	A wrapper that takes a FILE * and creates an libCZI::IStream object out of it
class CSimpleStreamImplFromFP : public libCZI::IStream {
private:
  FILE *fp;
public:
  CSimpleStreamImplFromFP() = delete;
  explicit CSimpleStreamImplFromFP(FILE *file_pointer) : fp(file_pointer) {}
  ~CSimpleStreamImplFromFP() override { fclose(this->fp); };
  void Read(std::uint64_t offset, void *pv, std::uint64_t size, std::uint64_t *ptrBytesRead) override;
};

class Reader {
  unique_ptr<CCZIReader> m_czireader;
  libCZI::SubBlockStatistics m_statistics;
public:
  explicit Reader(FILE *f_in);

  typedef std::map<libCZI::DimensionIndex, std::pair<int, int> > mapDiP;

  bool isMosaicFile();

  Reader::mapDiP get_shape();

  typedef std::tuple<py::list, py::array_t<int32_t>, std::vector<IndexMap> > tuple_ans;
  // typedef py::list tuple_ans;

  std::string cziread_meta();

  tuple_ans cziread_selected(libCZI::CDimCoordinate &planeCoord, int mIndex = -1);

private:
  // int convertDictToPlaneCoords(PyObject *obj, void *dim_p);
  py::array copy_bitmap_to_numpy_array(std::shared_ptr<libCZI::IBitmapData> pBitmap);

  bool dimsMatch(const libCZI::CDimCoordinate &targetDims, const libCZI::CDimCoordinate &cziDims);

  void add_sort_order_index(vector<IndexMap> &vec);

  static bool isPyramid0(const libCZI::SubBlockInfo &info) {
      return (info.logicalRect.w == info.physicalSize.w && info.logicalRect.h == info.physicalSize.h);
  }
};

/*

static PyObject *cziread_allsubblocks_from_istream(PyObject *self, PyObject *pyfp) {
  using namespace std::placeholders; // enable _1 _2 _3 type placeholders
  PyObject *in_file = nullptr;
  // parse arguments
  try {
    if (!PyArg_ParseTuple(pyfp, "O", &in_file)) {
      PyErr_SetString(PylibcziError, "Error: conversion of arguments failed. Check arguments.");
      return nullptr;
    }
    auto stream = cziread_io_buffered_reader_to_istream(self, in_file);
    auto cziReader = open_czireader_from_istream(stream);
    // count all the subblocks

    npy_intp subblock_count = 0;
    auto count_blocks([&subblock_count](int idx, const libCZI::SubBlockInfo &info) -> bool {
      subblock_count++;
      return true;
    });

    // assignment warning is a CLION error it should be fine.
    std::function<bool(int, const libCZI::SubBlockInfo &)> countLambdaAsFunc =
        static_cast< std::function<bool(int, const libCZI::SubBlockInfo &)> >(count_blocks);

    cziReader->EnumerateSubBlocks(countLambdaAsFunc);  // f_count_blocks);
    std::cout << "Enumerated " << subblock_count << std::endl;

    // meh - this seems to be not useful, what is an M-index? someone read the spec...
    //auto stats = cziReader->GetStatistics();
    //cout << stats.subBlockCount << " " << stats.maxMindex << endl;
    //int subblock_count = stats.subBlockCount;

    // copy the image data and coordinates into numpy arrays, return images as python list of numpy arrays
    PyObject *images = PyList_New(subblock_count);
    npy_intp eshp[2];
    eshp[0] = subblock_count;
    eshp[1] = 2;
    PyArrayObject *coordinates = (PyArrayObject *) PyArray_Empty(2, eshp, PyArray_DescrFromType(NPY_INT32), 0);
    npy_int32 *coords = (npy_int32 *) PyArray_DATA(coordinates);

    npy_intp cnt = 0;
    cziReader->EnumerateSubBlocks(
        [&cziReader, &subblock_count, &cnt, images, coords](int idx, const libCZI::SubBlockInfo &info) {
          //std::cout << "Index " << idx << ": " << libCZI::Utils::DimCoordinateToString(&info.coordinate)
          //  << " Rect=" << info.logicalRect << " M-index " << info.mIndex << std::endl;

          // add the sub-block image
          PyList_SetItem(images, cnt,
                         (PyObject *) copy_bitmap_to_numpy_array(
                             cziReader->ReadSubBlock(idx)->CreateBitmap()));
          // add the coordinates
          coords[2 * cnt] = info.logicalRect.x;
          coords[2 * cnt + 1] = info.logicalRect.y;

          //info.coordinate.EnumValidDimensions([](libCZI::DimensionIndex dim, int value)
          //{
          //    //valid_dims[(int) dim] = true;
          //    cout << "Dimension  " << dim << " value " << value << endl;
          //    return true;
          //});

          cnt++;
          return true;
        });

    return Py_BuildValue("OO", images, (PyObject *) coordinates);
  }
  catch (const BadArgsException &e) {
    PyErr_SetString(PyExc_TypeError, "Unable to map args provided from python to c++.");
    return NULL;
  }
  catch (const BadFileDescriptorException &fbad) {
    PyErr_SetString(PyExc_IOError, "Unable to convert ByteIO object to File pointer.");
    return NULL;
  }
  return NULL;
}
*/

}

#endif //PYLIBCZI_AICS_ADDED_HPP