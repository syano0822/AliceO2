// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file GPUROOTDump.h
/// \author David Rohr

#ifndef GPUROOTDUMP_H
#define GPUROOTDUMP_H

#include "GPUROOTDumpCore.h"

#if !defined(GPUCA_NO_ROOT) && !defined(GPUCA_GPUCODE)
#include <TTree.h>
#include <TNtuple.h>
#include <memory>
#include <stdexcept>
#endif

namespace GPUCA_NAMESPACE
{
namespace gpu
{
#if !defined(GPUCA_NO_ROOT) && !defined(GPUCA_GPUCODE)
namespace
{
template <class S>
struct internal_Branch {
  template <typename... Args>
  static void Branch(S* p, Args... args)
  {
  }
};
template <>
struct internal_Branch<TTree> {
  template <typename... Args>
  static void Branch(TTree* p, Args... args)
  {
    p->Branch(args...);
  }
};
} // namespace

template <class T>
class GPUROOTDump : public GPUROOTDumpBase
{
 public:
  static GPUROOTDump<T>& get(const char* name)
  {
    static GPUROOTDump<T> instance(name);
  }
  GPUROOTDump(const char* name)
  {
    mTree = new TTree(name, name);
    mTree->Branch(name, &mObj);
  }

  void write() override { mTree->Write(); }

  void Fill(const T& o)
  {
    mObj = o;
    mTree->Fill();
  }

 private:
  TTree* mTree = nullptr;
  T mObj;
};

template <>
class GPUROOTDump<TNtuple> : public GPUROOTDumpBase
{
 public:
  static GPUROOTDump<TNtuple>& get(const char* name, const char* options)
  {
    static GPUROOTDump<TNtuple> instance(name, options);
    return instance;
  }
  GPUROOTDump(const char* name, const char* options)
  {
    mNTuple = new TNtuple(name, name, options);
  }

  void write() override { mNTuple->Write(); }

  template <typename... Args>
  void Fill(Args... args)
  {
    mNTuple->Fill(args...);
  }

 private:
  TNtuple* mNTuple;
};
#else
template <class T>
class GPUROOTDump
{
 public:
  GPUROOTDump() = delete;
};
#endif
} // namespace gpu
} // namespace GPUCA_NAMESPACE

#endif
