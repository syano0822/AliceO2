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

#include <MFTCondition/MFTDCSConfigReader.h>
#include "Framework/Logger.h"
#include "TSystem.h"
#include <fstream>

using namespace o2::mft;

//_______________________________________________________________
void MFTDCSConfigReader::init(bool ver){
  mVerbose = ver;
  initMap();
}

//_______________________________________________________________
void MFTDCSConfigReader::loadConfig(gsl::span<const char> configBuf)
{  
  
  const char* params = configBuf.data();
  mParams = std::string(params);
  
  parseConfig();
}

//_______________________________________________________________

void MFTDCSConfigReader::parseConfig()
{
   
  char delimiter_newline = '\n';
  char delimiter = ',';
    
  std::stringstream stream(mParams);
  std::string row;

  mNumRow = 0;
  mNumRU = 0;
  mNumALPIDE = 0;

  int arrAddRUConf[128];
  int arrAddALPIDEConf[128];
    
  int arrValRUConf[128];
  int arrValALPIDEConf[128];
  
  while (std::getline(stream, row, delimiter_newline)){
      
    std::stringstream srow(row);
    std::string col;

    int ncol = 0;

    while (std::getline(srow, col, delimiter)){
      int val = atoi(col.c_str());
      if(mNumRow==0){
	arrAddRUConf[ncol] = val;
	++mNumRU;
      }
      else if(mNumRow==1){
	arrValRUConf[ncol] = val;
      }
      else if(mNumRow==2){
	arrAddALPIDEConf[ncol] = val;
	++mNumALPIDE;
      }
      else if(mNumRow==3){
	arrValALPIDEConf[ncol] = val;
      }       
      ++ncol;
    }
    ++mNumRow; 
  }
  
  for(int iRUconf=0; iRUconf<mNumRU; ++iRUconf){
    if(mVerbose) LOG(INFO)<<"("<<arrAddRUConf[iRUconf]<<")   "<<mNameFromDecAddRUconf[arrAddRUConf[iRUconf]]<<" : "<<arrValRUConf[iRUconf];
    o2::mft::DCSConfigInfo conf;
    conf.clear();
    conf.setData(arrValRUConf[iRUconf]);
    conf.setAddDeci(arrValRUConf[iRUconf]);
    conf.setName(mNameFromDecAddRUconf[arrAddRUConf[iRUconf]]);      
    conf.setType(0);
    mDCSConfig.emplace_back(conf);
  }

  for(int iALPIDEconf=0; iALPIDEconf<mNumALPIDE; ++iALPIDEconf){
    if(mVerbose) LOG(INFO)<<"("<<arrAddALPIDEConf[iALPIDEconf]<<")   "<<mNameFromDecAddALPIDEconf[arrAddALPIDEConf[iALPIDEconf]]<<" : "<<arrValALPIDEConf[iALPIDEconf];
    o2::mft::DCSConfigInfo conf;
    conf.clear();
    conf.setData(arrValALPIDEConf[iALPIDEconf]);
    conf.setAddDeci(arrValALPIDEConf[iALPIDEconf]);
    conf.setName(mNameFromDecAddALPIDEconf[arrAddALPIDEConf[iALPIDEconf]]);      
    conf.setType(1);
    mDCSConfig.emplace_back(conf);
  }
  
}

void MFTDCSConfigReader::initMap(){
  
  mNameFromDecAddRUconf[1046] = "MANCHESTER";
  mNameFromDecAddRUconf[4096] = "ENABLE";
  mNameFromDecAddRUconf[4097] = "TRIGGER_PERIOD";
  mNameFromDecAddRUconf[4098] = "PULSE_nTRIGGER";
  mNameFromDecAddRUconf[4099] = "TRIGGER_MIN_DISTANCE";
  mNameFromDecAddRUconf[4101] = "OPCODE_GATING";
  mNameFromDecAddRUconf[4102] = "TRIGGER_DELAY";
  mNameFromDecAddRUconf[4103] = "ENABLE_PACKER_0";
  mNameFromDecAddRUconf[4104] = "ENABLE_PACKER_1";
  mNameFromDecAddRUconf[4105] = "ENABLE_PACKER_2";
  mNameFromDecAddRUconf[4106] = "TRIG_SOURCE";
  mNameFromDecAddRUconf[5376] = "TIMEOUT_TO_START";
  mNameFromDecAddRUconf[5377] = "TIMEOUT_TO_STOP";
  mNameFromDecAddRUconf[5378] = "TIMEOUT_IN_IDLE";
    
  mNameFromDecAddALPIDEconf[1] = "Mode Control Register";
  mNameFromDecAddALPIDEconf[4] = "FROMU Configration Register 1";
  mNameFromDecAddALPIDEconf[5] = "FROMU Configration Register 2";
  mNameFromDecAddALPIDEconf[6] = "FROMU Configration Register 3";
  mNameFromDecAddALPIDEconf[7] = "FROMU Pulsing Register 1";
  mNameFromDecAddALPIDEconf[8] = "FROMU Pulsing Register 2";
  mNameFromDecAddALPIDEconf[16] = "CMU&DMU Configration Register";
  mNameFromDecAddALPIDEconf[20] = "DTU Configration Register";
  mNameFromDecAddALPIDEconf[21] = "DTU DACs Register";
  mNameFromDecAddALPIDEconf[24] = "DTU Test Register 1";
  mNameFromDecAddALPIDEconf[25] = "DTU Test Register 2";
  mNameFromDecAddALPIDEconf[26] = "DTU Test Register 3";
  mNameFromDecAddALPIDEconf[1539] = "VCASP";
  mNameFromDecAddALPIDEconf[1544] = "VCLIP";
  mNameFromDecAddALPIDEconf[1549] = "IBIAS";
  
}
