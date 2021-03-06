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

#include "build_geometry.C"
#if !defined(__CLING__) || defined(__ROOTCLING__)
#include <Generators/PrimaryGenerator.h>
#include <Generators/GeneratorFactory.h>
#include <Generators/PDG.h>
#include "SimulationDataFormat/MCEventHeader.h"
#include <SimConfig/SimConfig.h>
#include <SimConfig/SimParams.h>
#include <CommonUtils/ConfigurableParam.h>
#include <CommonUtils/RngHelper.h>
#include <TStopwatch.h>
#include <memory>
#include "DataFormatsParameters/GRPObject.h"
#include "FairParRootFileIo.h"
#include "FairSystemInfo.h"
#include <SimSetup/SimSetup.h>
#include <Steer/O2RunSim.h>
#include <DetectorsBase/MaterialManager.h>
#include <CCDB/BasicCCDBManager.h>
#include <DetectorsCommonDataFormats/NameConf.h>
#include "DetectorsBase/Aligner.h"
#include <unistd.h>
#include <sstream>
#endif
#include "migrateSimFiles.C"

FairRunSim* o2sim_init(bool asservice)
{
  auto& confref = o2::conf::SimConfig::Instance();
  // initialize CCDB service
  auto& ccdbmgr = o2::ccdb::BasicCCDBManager::instance();
  ccdbmgr.setURL(confref.getConfigData().mCCDBUrl);
  ccdbmgr.setTimestamp(confref.getConfigData().mTimestamp);
  // try to verify connection
  if (!ccdbmgr.isHostReachable()) {
    LOG(ERROR) << "Could not setup CCDB connecting";
  } else {
    LOG(INFO) << "Initialized CCDB Manager at URL: " << ccdbmgr.getURL();
    LOG(INFO) << "Initialized CCDB Manager with timestamp : " << ccdbmgr.getTimestamp();
  }

  // we can read from CCDB (for the moment faking with a TFile)
  // o2::conf::ConfigurableParam::fromCCDB("params_ccdb.root", runid);

  // update the parameters from an INI/JSON file, if given (overrides code-based version)
  o2::conf::ConfigurableParam::updateFromFile(confref.getConfigFile());

  // update the parameters from stuff given at command line (overrides file-based version)
  o2::conf::ConfigurableParam::updateFromString(confref.getKeyValueString());

  // write the final configuration file
  o2::conf::ConfigurableParam::writeINI(o2::base::NameConf::getMCConfigFileName(confref.getOutPrefix()));

  // we can update the binary CCDB entry something like this ( + timestamp key )
  // o2::conf::ConfigurableParam::toCCDB("params_ccdb.root");

  // set seed
  auto seed = o2::utils::RngHelper::setGRandomSeed(confref.getStartSeed());
  LOG(INFO) << "RNG INITIAL SEED " << seed;

  auto genconfig = confref.getGenerator();
  FairRunSim* run = new o2::steer::O2RunSim(asservice);
  run->SetImportTGeoToVMC(false); // do not import TGeo to VMC since the latter is built together with TGeo
  run->SetSimSetup([confref]() { o2::SimSetup::setup(confref.getMCEngine().c_str()); });
  run->SetRunId(confref.getConfigData().mTimestamp);

  auto pid = getpid();
  std::stringstream s;
  s << confref.getOutPrefix();
  if (asservice) {
    s << "_" << pid;
  }
  s << ".root";

  std::string outputfilename = s.str();
  run->SetOutputFile(outputfilename.c_str());  // Output file
  run->SetName(confref.getMCEngine().c_str()); // Transport engine
  run->SetIsMT(confref.getIsMT());             // MT mode

  /** set event header **/
  auto header = new o2::dataformats::MCEventHeader();
  run->SetMCEventHeader(header);

  // construct geometry / including magnetic field
  build_geometry(run);

  // setup generator
  auto embedinto_filename = confref.getEmbedIntoFileName();
  auto primGen = new o2::eventgen::PrimaryGenerator();
  if (!embedinto_filename.empty()) {
    primGen->embedInto(embedinto_filename);
  }
  if (!asservice) {
    o2::eventgen::GeneratorFactory::setPrimaryGenerator(confref, primGen);
  }
  run->SetGenerator(primGen);

  // Timer
  TStopwatch timer;
  timer.Start();

  o2::detectors::DetID::mask_t detMask{};
  {
    auto& modulelist = o2::conf::SimConfig::Instance().getActiveDetectors();
    for (const auto& md : modulelist) {
      int id = o2::detectors::DetID::nameToID(md.c_str());
      if (id >= o2::detectors::DetID::First) {
        detMask |= o2::detectors::DetID::getMask(id);
      }
    }
    // somewhat ugly, but this is the most straighforward way to make sure the detectors to align
    // don't include detectors which are not activated
    auto& aligner = o2::base::Aligner::Instance();
    if (aligner.getDetectorsMask().any()) {
      aligner.setValue(fmt::format("{}.mDetectors", aligner.getName()), o2::detectors::DetID::getNames(detMask, ','));
    }
  }

  // set global density scaling factor
  auto& matmgr = o2::base::MaterialManager::Instance();
  matmgr.setDensityScalingFactor(o2::conf::SimMaterialParams::Instance().globalDensityFactor);

  // run init
  run->Init();

  std::time_t runStart = std::time(nullptr);

  // runtime database
  bool kParameterMerged = true;
  auto rtdb = run->GetRuntimeDb();
  auto parOut = new FairParRootFileIo(kParameterMerged);

  std::stringstream s2;
  s2 << confref.getOutPrefix();
  if (asservice) {
    s2 << "_" << pid;
  }
  s2 << "_par.root";
  std::string parfilename = s2.str();
  parOut->open(parfilename.c_str());
  rtdb->setOutput(parOut);
  rtdb->saveOutput();
  rtdb->print();
  o2::PDG::addParticlesToPdgDataBase(0);

  {
    // store GRPobject
    o2::parameters::GRPObject grp;
    grp.setRun(run->GetRunId());
    grp.setTimeStart(runStart);
    grp.setTimeEnd(std::time(nullptr));
    grp.setDetsReadOut(detMask);
    // CTP is not a physical detector, just flag in the GRP if requested
    if (isActivated("CTP")) {
      grp.addDetReadOut(o2::detectors::DetID::CTP);
    }

    grp.print();
    printf("VMC: %p\n", TVirtualMC::GetMC());
    auto field = dynamic_cast<o2::field::MagneticField*>(run->GetField());
    if (field) {
      o2::units::Current_t currDip = field->getCurrentDipole();
      o2::units::Current_t currL3 = field->getCurrentSolenoid();
      grp.setL3Current(currL3);
      grp.setDipoleCurrent(currDip);
      grp.setFieldUniformity(field->IsUniform());
    }
    // save
    std::string grpfilename = o2::base::NameConf::getGRPFileName(confref.getOutPrefix());
    TFile grpF(grpfilename.c_str(), "recreate");
    grpF.WriteObjectAny(&grp, grp.Class(), "GRP");
  }

  // todo: save beam information in the grp

  // print summary about cuts and processes used
  std::ofstream cutfile(o2::base::NameConf::getCutProcFileName(confref.getOutPrefix()));
  matmgr.printCuts(cutfile);
  matmgr.printProcesses(cutfile);

  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();

  // extract max memory usage for init
  FairSystemInfo sysinfo;
  LOG(INFO) << "Init: Real time " << rtime << " s, CPU time " << ctime << "s";
  LOG(INFO) << "Init: Memory used " << sysinfo.GetMaxMemory() << " MB";

  return run;
}

// only called from the normal o2sim
void o2sim_run(FairRunSim* run, bool asservice)
{
  TStopwatch timer;
  timer.Start();

  auto& confref = o2::conf::SimConfig::Instance();
  if (!asservice) {
    run->Run(confref.getNEvents());
  } else {
    run->Run(1);
  }

  // Finish
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();

  // extract max memory usage
  FairSystemInfo sysinfo;

  LOG(INFO) << "Macro finished succesfully.";
  LOG(INFO) << "Real time " << rtime << " s, CPU time " << ctime << "s";
  LOG(INFO) << "Memory used " << sysinfo.GetMaxMemory() << " MB";

  // migrate to file format where hits sit in separate files
  // (Note: The parallel version is doing this intrinsically;
  //  The serial version uses FairRootManager IO which handles a common file IO for all outputs)
  if (!asservice) {
    LOG(INFO) << "Migrating simulation output to separate hit file format";
    migrateSimFiles(confref.getOutPrefix().c_str());
  }
}

// asservice: in a parallel device-based context?
void o2sim(bool asservice = false)
{
  auto run = o2sim_init(asservice);
  o2sim_run(run, asservice);
  delete run;
}
