// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// @file   DCSConfigInfo.h
/// @brief  MFT DCS Configuration file processor

#include <TString.h>
#include <unordered_map>
#include <iostream>

namespace o2
{
namespace mft
{

class DCSConfigInfo
{
  
 public:
  
  DCSConfigInfo(){
    mRUparamData.clear();
    mRUparamRegistAdd.clear();
  }
  
  ~DCSConfigInfo(){

  }
  
  void makeEmpty(){
    mRUparamData.clear();
    mRUparamRegistAdd.clear();
  }
  
  void setData(std::string key, int val){
    mRUparamData[key] = val;
  }

  void setRegistAdd(std::string key, int val){
    mRUparamRegistAdd[key] = val;
  }

  int getData(std::string key)
  {
    return mRUparamData[key];
  }
  
  std::unordered_map<std::string,int> mRUparamData;
  std::unordered_map<std::string,int> mRUparamRegistAdd;

  ClassDefNV(DCSConfigInfo, 1);

}; // end class
} // namespace mft
} // namespace o2
