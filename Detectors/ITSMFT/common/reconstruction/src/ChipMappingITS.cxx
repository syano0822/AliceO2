// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

// \file ChipMappingITS.cxx
// \brief Autimatically generated ITS chip <-> module mapping

#include "ITSMFTReconstruction/ChipMappingITS.h"
#include <cassert>

using namespace o2::ITSMFT;

constexpr std::array<int, ChipMappingITS::NSubB> ChipMappingITS::NModulesAlongStaveSB;

constexpr std::array<int, ChipMappingITS::NSubB> ChipMappingITS::NChipsPerModuleSB;
constexpr std::array<int, ChipMappingITS::NSubB> ChipMappingITS::NModulesPerStaveSB;
constexpr std::array<int, ChipMappingITS::NSubB> ChipMappingITS::NCablesPerStaveSB;
constexpr std::array<int, ChipMappingITS::NSubB> ChipMappingITS::CablesOnStaveSB;

constexpr std::array<int, ChipMappingITS::NSubB> ChipMappingITS::NStavesSB;
constexpr std::array<int, ChipMappingITS::NSubB> ChipMappingITS::NChipsPerStaveSB;
constexpr std::array<int, ChipMappingITS::NSubB> ChipMappingITS::NChipsPerCableSB;
constexpr std::array<int, ChipMappingITS::NSubB> ChipMappingITS::NChipsSB;
constexpr std::array<int, ChipMappingITS::NLayers> ChipMappingITS::NStavesPerLr;
constexpr std::array<uint8_t, ChipMappingITS::NLayers> ChipMappingITS::RUTypeLr;

constexpr std::array<uint8_t, ChipMappingITS::NSubB> ChipMappingITS::GBTHeaderFlagSB;
constexpr std::uint8_t ChipMappingITS::ChipOBModSW2HW[14];
constexpr std::uint8_t ChipMappingITS::ChipOBModHW2SW[15];

//______________________________________________
ChipMappingITS::ChipMappingITS()
{
  // init chips info

  // IB: single cable per chip
  int ctrChip = 0;
  mChipInfoEntrySB[IB] = ctrChip;
  mCableHW2SW[IB].resize(NChipsPerStaveSB[IB], 0xff);
  mCableHWFirstChip[IB].resize(NChipsPerStaveSB[IB], 0xff);
  for (int i = 0; i < NChipsPerStaveSB[IB]; i++) {
    auto& cInfo = mChipsInfo[ctrChip++];
    cInfo.id = i;
    cInfo.moduleHW = 0;
    cInfo.moduleSW = 0;
    cInfo.chipOnModuleSW = i;
    cInfo.chipOnModuleHW = i;
    cInfo.cableHW = i;
    cInfo.cableSW = i;
    cInfo.chipOnCable = 0;                          // every chip is master
    mCableHW2SW[IB][cInfo.cableHW] = cInfo.cableSW; //1-to-1 mapping
    mCableHWFirstChip[IB][i] = 0;                   // stave and module are the same
  }

  for (int bid = MB; bid <= OB; bid++) { // MB and OB staves have similar layout
    mChipInfoEntrySB[bid] = ctrChip;
    mCableHW2SW[bid].resize(0xff, 0xff);
    mCableHWFirstChip[bid].resize(0xff, 0xff);

    for (int i = 0; i < NChipsPerStaveSB[bid]; i++) {
      auto& cInfo = mChipsInfo[ctrChip++];
      int hstave = i / (NChipsPerStaveSB[bid] / 2);
      cInfo.id = i;
      cInfo.moduleSW = i / NChipsPerModuleSB[bid];                         // identification within the stav
      cInfo.moduleHW = 1 + cInfo.moduleSW % (NModulesPerStaveSB[bid] / 2); // identification within the HS

      cInfo.chipOnModuleSW = i % NChipsPerModuleSB[bid];
      cInfo.chipOnModuleHW = ChipOBModSW2HW[cInfo.chipOnModuleSW];

      uint8_t connector = (hstave << 1) + (cInfo.chipOnModuleSW < (NChipsPerModuleSB[bid] / 2) ? 0 : 1);
      cInfo.cableHW = (connector << 3) + (cInfo.moduleHW - 1);
      cInfo.cableSW = (cInfo.moduleHW - 1) + connector * (NModulesPerStaveSB[bid] / 2);
      cInfo.chipOnCable = cInfo.chipOnModuleSW % (NChipsPerModuleSB[bid] / 2); // each cable serves half module
      mCableHW2SW[bid][cInfo.cableHW] = cInfo.cableSW;
      if (cInfo.moduleHW == 1) { // numbering on MB and OB starts from 1!!!
        mCableHWFirstChip[bid][cInfo.cableHW] = cInfo.chipOnModuleHW;
      }
    }
  }

  int ctrStv = 0;
  uint32_t maxRUHW = 0;
  uint16_t chipCount = 0;
  for (int ilr = 0; ilr < NLayers; ilr++) {
    for (int ist = 0; ist < NStavesPerLr[ilr]; ist++) {
      auto& sInfo = mStavesInfo[ctrStv];
      sInfo.idSW = ctrStv++;
      sInfo.idHW = sInfo.idSW; // at the moment assume 1 to 1 mapping
      if (sInfo.idHW > maxRUHW) {
        maxRUHW = sInfo.idHW;
      }
      sInfo.layer = ilr;
      sInfo.ruType = RUTypeLr[ilr];
      sInfo.nCables = NCablesPerStaveSB[sInfo.ruType];
      sInfo.firstChipIDSW = chipCount;
      chipCount += NChipsPerStaveSB[sInfo.ruType];
    }
  }
  assert(ctrStv == getNRUs());
  assert(maxRUHW < 0xffff);
  mRUHW2SW.resize(maxRUHW + 1, 0xff);
  for (int i = 0; i < getNRUs(); i++) {
    auto& sInfo = mStavesInfo[i];
    mRUHW2SW[sInfo.idHW] = sInfo.idSW;
  }
}

void ChipMappingITS::print() const
{
  int ctrChip = 0;
  printf("\n\nSubBarrel IB\n");
  for (int i = 0; i < NChipsPerStaveSB[IB]; i++) {
    mChipsInfo[ctrChip++].print();
  }
  printf("\n\nSubBarrel MB\n");
  for (int i = 0; i < NChipsPerStaveSB[MB]; i++) {
    mChipsInfo[ctrChip++].print();
  }
  printf("\n\nSubBarrel OB\n");
  for (int i = 0; i < NChipsPerStaveSB[OB]; i++) {
    mChipsInfo[ctrChip++].print();
  }
}
