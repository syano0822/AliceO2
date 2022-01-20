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

#ifndef MFTDCSCONFIGPROCESSOR_H_
#define MFTDCSCONFIGPROCESSOR_H_

#include "Rtypes.h"
#include "MFTCondition/DCSConfigInfo.h"
#include <gsl/span>
#include <memory>

/// @brief Class to read the MFT config information

namespace o2
{
namespace mft
{

  using namespace o2::mft;

  class MFTDCSConfigReader
  {

  public:
    MFTDCSConfigReader() = default;  // default constructor
    ~MFTDCSConfigReader() = default; // default destructor
    
    void init(bool);
    void loadConfig(gsl::span<const char> configBuf); // load FEElight config
    
    
    std::vector<o2::mft::DCSConfigInfo>& getConfigInfo() { return mDCSConfig; }
    
  private:
    
    void initMap();
    void parseConfig();

    std::string mParams;
    
    int mNumRow;
    int mNumRU;
    int mNumALPIDE;

    bool mVerbose = false;
    
    std::unordered_map<int,std::string> mNameFromDecAddRUconf;
    std::unordered_map<int,std::string> mNameFromDecAddALPIDEconf;
    
    std::vector<o2::mft::DCSConfigInfo> mDCSConfig;

    ClassDefNV(MFTDCSConfigReader, 1);
};

} // namespace tof
} // namespace o2

#endif
