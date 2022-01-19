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

const int mNumParam = 512;

namespace o2
{
namespace mft
{

class DCSConfigInfo
{
  
 public:
  
  DCSConfigInfo(){
    mName = "";
    mData = -999;
    mAddrHexa = -999;
    mAddrDeci = -999;
    mConfigType = -999;
    mIsStored = false;
  }
  
  ~DCSConfigInfo(){
    
  }
  
  void clear(){
    mName = "";
    mData = -999;
    mAddrHexa = -999;
    mAddrDeci = -999;
    mConfigType = -999;
    mIsStored = false;    
  }

  void setName(std::string name){
    mName = name;
  }
  void setData(int data){
    mData = data;
  }
  void setAddHexa(int addr){
    mAddrHexa = addr;
  }
  void setAddDeci(int addr){
    mAddrDeci = addr;
  }
  void setType(int type){
    mConfigType = type;
    if(type == 0) mConfigTypeStr = "RU";
    else if(type == 1) mConfigTypeStr = "ALPIDE";
    else mConfigTypeStr = "";
  }
  void setTypeStr(std::string type){
    mConfigTypeStr = type;
    if(type == "RU") mConfigType = 0;
    else if(type == "ALPIDE") mConfigType = 1;
    else mConfigType = -999;
  }

  void getName(std::string& name){
    name = mName;
  }
  void getData(int& data){
    data = mData;
  }
  void getAddHexa(int& addr){
    addr = mAddrHexa;
  }
  void getAddDeci(int& addr){
    addr = mAddrDeci;
  }
  void getType(int& type){
    type = mConfigType;
  }
  
  bool isStored(){    
    return mIsStored;
  }
  
  void init(){
    if(mData>=0 && mAddrDeci>=0){
      mIsStored = true;
    }
    
  }

 private:
  
  std::string mName;
  int mData;
  int mAddrDeci;
  int mAddrHexa;
  int mConfigType; //RU = 0, ALPIDE = 1;
  std::string mConfigTypeStr;
  bool mIsStored;
  
  ClassDefNV(DCSConfigInfo, 1);

}; // end class
} // namespace mft
} // namespace o2
