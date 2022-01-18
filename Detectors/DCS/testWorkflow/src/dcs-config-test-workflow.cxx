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

#include "Framework/DataProcessorSpec.h"
#include "Framework/Task.h"
#include "Framework/Logger.h"
#include "CommonUtils/ConfigurableParam.h"
#include "DetectorsCommonDataFormats/DetID.h"
#include "CCDB/CcdbObjectInfo.h"
#include "CommonUtils/MemFileHelper.h"
#include "CCDB/CcdbApi.h"
#include "MFTCondition/DCSConfigInfo.h"
#include "DetectorsCalibration/Utils.h"

#include <unistd.h>
#include <TRandom.h>
#include <TStopwatch.h>
#include "DetectorsDCS/DataPointIdentifier.h"
#include "DetectorsDCS/DataPointValue.h"
#include "DetectorsDCS/DataPointCompositeObject.h"
#include "DetectorsDCS/DeliveryType.h"
#include "DetectorsDCS/AliasExpander.h"
#include "MFTCondition/MFTDCSProcessor.h"
#include "MFTCondition/DCSNameResolver.h"
#include "DetectorsCalibration/Utils.h"
#include "CCDB/CcdbApi.h"
#include "CCDB/BasicCCDBManager.h"
#include "Framework/DeviceSpec.h"
#include "Framework/ConfigParamRegistry.h"
#include "Framework/ControlService.h"
#include "Framework/WorkflowSpec.h"
#include "Framework/Task.h"
#include "Framework/Logger.h"

using namespace o2::framework;
using DetID = o2::detectors::DetID;

std::string getHexNum(int dec){
  return Form("%x",dec);
}


// we need to add workflow options before including Framework/runDataProcessing
void customize(std::vector<o2::framework::ConfigParamSpec>& workflowOptions)
{
  // option allowing to set parameters
  std::vector<o2::framework::ConfigParamSpec> options{
    {"configKeyValues", VariantType::String, "", {"Semicolon separated key=value strings"}},
    {"detector", VariantType::String, "ITS", {"detector name"}}};

  std::swap(workflowOptions, options);
}
// ------------------------------------------------------------------

namespace o2
{
namespace dcs
{
class DCSConfigConsumer : public o2::framework::Task
{
public:
  void run(o2::framework::ProcessingContext& pc) final
  {
    auto fileBuff = pc.inputs().get<gsl::span<char>>("confFile");
    auto fileName = pc.inputs().get<std::string>("confFileName");

    LOG(info) << "got input file " << fileName << " of size " << fileBuff.size();

    std::unordered_map<int,std::string> getNameFromDecAddRUconf;
    std::unordered_map<int,std::string> getNameFromDecAddALPIDEconf;

    getNameFromDecAddRUconf[1046] = "MANCHESTER";
    getNameFromDecAddRUconf[4096] = "ENABLE";
    getNameFromDecAddRUconf[4097] = "TRIGGER_PERIOD";
    getNameFromDecAddRUconf[4098] = "PULSE_nTRIGGER";
    getNameFromDecAddRUconf[4099] = "TRIGGER_MIN_DISTANCE";
    getNameFromDecAddRUconf[4101] = "OPCODE_GATING";
    getNameFromDecAddRUconf[4102] = "TRIGGER_DELAY";
    getNameFromDecAddRUconf[4103] = "ENABLE_PACKER_0";
    getNameFromDecAddRUconf[4104] = "ENABLE_PACKER_1";
    getNameFromDecAddRUconf[4105] = "ENABLE_PACKER_2";
    getNameFromDecAddRUconf[4106] = "TRIG_SOURCE";
    getNameFromDecAddRUconf[5376] = "TIMEOUT_TO_START";
    getNameFromDecAddRUconf[5377] = "TIMEOUT_TO_STOP";
    getNameFromDecAddRUconf[5378] = "TIMEOUT_IN_IDLE";
    
    getNameFromDecAddALPIDEconf[1] = "Mode Control Register";
    getNameFromDecAddALPIDEconf[4] = "FROMU Configration Register 1";
    getNameFromDecAddALPIDEconf[5] = "FROMU Configration Register 2";
    getNameFromDecAddALPIDEconf[6] = "FROMU Configration Register 3";
    getNameFromDecAddALPIDEconf[7] = "FROMU Pulsing Register 1";
    getNameFromDecAddALPIDEconf[8] = "FROMU Pulsing Register 2";
    getNameFromDecAddALPIDEconf[16] = "CMU&DMU Configration Register";
    getNameFromDecAddALPIDEconf[20] = "DTU Configration Register";
    getNameFromDecAddALPIDEconf[21] = "DTU DACs Register";
    getNameFromDecAddALPIDEconf[24] = "DTU Test Register 1";
    getNameFromDecAddALPIDEconf[25] = "DTU Test Register 2";
    getNameFromDecAddALPIDEconf[26] = "DTU Test Register 3";
    getNameFromDecAddALPIDEconf[1539] = "VCASP";
    getNameFromDecAddALPIDEconf[1544] = "VCLIP";
    getNameFromDecAddALPIDEconf[1549] = "IBIAS";
    
    const char* chrparams = fileBuff.data();     
    std::string params = std::string(chrparams);
    
    char delimiter_newline = '\n';
    char delimiter = ',';
    
    std::stringstream sparams(params);
    std::string list;
    
    int nline=0;
    
    int nRUConf=0;
    int nALPIDEConf=0;

    int arrAddRUConf[128];
    int arrAddALPIDEConf[128];
    
    int arrValRUConf[128];
    int arrValALPIDEConf[128];

    while (std::getline(sparams, list, delimiter_newline)){
      
      std::stringstream slist(list);
      std::string strvalue;

      int nval = 0;

      while (std::getline(slist, strvalue, delimiter)){
	int val = atoi(strvalue.c_str());
	if(nline==0){
	  arrAddRUConf[nval] = val;
	  ++nRUConf;
	}
	else if(nline==1){
	  arrValRUConf[nval] = val;
	}	
	else if(nline==2){
	  arrAddALPIDEConf[nval] = val;
	  ++nALPIDEConf;
	}
	else if(nline==3){
	  arrValALPIDEConf[nval] = val;
	}	       
	++nval;
      }     
      ++nline; 
    }
    
    std::vector<o2::mft::DCSConfigInfo> ccdbConfig;
    
    //LOG(INFO)<<"RU parameter name and values:: ";

    for(int iRUconf=0; iRUconf<nRUConf; ++iRUconf){
      LOG(INFO)<<"("<<arrAddRUConf[iRUconf]<<")   "<<getNameFromDecAddRUconf[arrAddRUConf[iRUconf]]<<" : "<<arrValRUConf[iRUconf];
      o2::mft::DCSConfigInfo info;
      info.makeEmpty();
      info.setData(getNameFromDecAddRUconf[arrAddRUConf[iRUconf]],arrValRUConf[iRUconf]);
      ccdbConfig.emplace_back(info);
    }
    
    LOG(INFO)<<"The number of configuration parameters in RU: "<<ccdbConfig.size();
    
    /*
    for(int iALPIDEconf=0; iALPIDEconf<nALPIDEConf; ++iALPIDEconf){
      //std::cout<<iALPIDEconf<<std::endl;
      std::cout<<"("<<arrAddALPIDEConf[iALPIDEconf]<<")   "<<getNameFromDecAddALPIDEconf[arrAddALPIDEConf[iALPIDEconf]]<<" : "<<arrValALPIDEConf[iALPIDEconf]<<std::endl;      
      o2::mft::DCSConfigInfo info;
      //info.makeEmpty();
      //ccdbConfig.emplace_back();
    }
    */

    o2::ccdb::CcdbObjectInfo ccdbConfigInfo;    
    std::map<std::string, std::string> md;
    md["responsible"] = "Satoshi Yano";    
    auto tfid = o2::header::get<o2::framework::DataProcessingHeader*>(pc.inputs().get("confFile").header)->startTime;
    prepareCCDBobjectInfo(ccdbConfig,ccdbConfigInfo,"MFT/Config/Params",tfid,md);
    
    sendConfigoutput(pc.outputs(),ccdbConfig,ccdbConfigInfo);
    
  }
  
private:

  template <typename T>
  void sendConfigoutput(DataAllocator& output, T& payload, o2::ccdb::CcdbObjectInfo& info){
    
    auto image = o2::ccdb::CcdbApi::createObjectImage(&payload, &info);
    LOG(info) << "Sending object " << info.getPath() << "/" << info.getFileName() << " of size " << image->size()
              << " bytes, valid for " << info.getStartValidityTimestamp() << " : " << info.getEndValidityTimestamp();


  }

  template <typename T>
  void prepareCCDBobjectInfo(T& obj, o2::ccdb::CcdbObjectInfo& info, const std::string& path, uint64_t tf, const std::map<std::string, std::string>& md)
  {
    
    // prepare all info to be sent to CCDB for object obj
    auto clName = o2::utils::MemFileHelper::getClassName(obj);
    auto flName = o2::ccdb::CcdbApi::generateFileName(clName);
    info.setPath(path);
    info.setObjectType(clName);
    info.setFileName(flName);
    info.setStartValidityTimestamp(tf);
    info.setEndValidityTimestamp(99999999999999);
    info.setMetaData(md);

    constexpr long SECONDSPERYEAR = 365 * 24 * 60 * 60;
    long tstart = o2::ccdb::getCurrentTimestamp();    
    long tend = o2::ccdb::getFutureTimestamp(SECONDSPERYEAR);

    info.setStartValidityTimestamp(tstart);
    info.setEndValidityTimestamp(tend);

  }

};
} // namespace dcs
} // namespace o2

DataProcessorSpec getDCSConsumerSpec(DetID det)
{

  std::vector<OutputSpec> outputs;  
  outputs.emplace_back(ConcreteDataTypeMatcher{o2::calibration::Utils::gDataOriginCDBPayload, "DCS_CONFIG_FILE"}, Lifetime::Condition);
  outputs.emplace_back(ConcreteDataTypeMatcher{o2::calibration::Utils::gDataOriginCDBWrapper, "DCS_CONFIG_FILE"}, Lifetime::Condition);
  
  std::string procName = "dcs-config-consumer-";
  procName += det.getName();
  return DataProcessorSpec{
    procName,
    Inputs{{"confFile", ConcreteDataTypeMatcher{det.getDataOrigin(), "DCS_CONFIG_FILE"}, Lifetime::Timeframe},
           {"confFileName", ConcreteDataTypeMatcher{det.getDataOrigin(), "DCS_CONFIG_NAME"}, Lifetime::Timeframe}},
      Outputs{},
    AlgorithmSpec{adaptFromTask<o2::dcs::DCSConfigConsumer>()},
    Options{}};
}

#include "Framework/runDataProcessing.h"

WorkflowSpec defineDataProcessing(ConfigContext const& configcontext)
{
  WorkflowSpec specs;
  o2::conf::ConfigurableParam::updateFromString(configcontext.options().get<std::string>("configKeyValues"));
  auto detName = configcontext.options().get<std::string>("detector");
  auto detID = DetID::nameToID(detName.c_str(), DetID::First);
  if (detID < 0) {
    throw std::runtime_error(fmt::format("{} is not a valid detector name", detName));
  }
  specs.emplace_back(getDCSConsumerSpec({detID}));

  return specs;
}
