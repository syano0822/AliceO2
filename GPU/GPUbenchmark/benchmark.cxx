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
///
/// \file benchmark.cxx
/// \author mconcas@cern.ch
/// \brief configuration widely inspired/copied by SimConfig
#include "Shared/Kernels.h"

bool parseArgs(o2::benchmark::benchmarkOpts& conf, int argc, const char* argv[])
{
  namespace bpo = boost::program_options;
  bpo::variables_map vm;
  bpo::options_description options("Benchmark options");
  options.add_options()(
    "help,h", "Print help message.")(
    "chunkSize,c", bpo::value<float>()->default_value(1.f), "Size of scratch partitions (GB).")(
    "regions,r", bpo::value<int>()->default_value(2), "Number of memory regions to partition RAM in.")(
    "freeMemFraction,f", bpo::value<float>()->default_value(0.95f), "Fraction of free memory to be allocated (min: 0.f, max: 1.f).")(
    "launches,l", bpo::value<int>()->default_value(10), "Number of iterations in reading kernels.")(
    "ntests,n", bpo::value<int>()->default_value(1), "Number of times each test is run.");
  try {
    bpo::store(parse_command_line(argc, argv, options), vm);
    if (vm.count("help")) {
      std::cout << options << std::endl;
      return false;
    }

    bpo::notify(vm);
  } catch (const bpo::error& e) {
    std::cerr << e.what() << "\n\n";
    std::cerr << "Error parsing command line arguments. Available options:\n";

    std::cerr << options << std::endl;
    return false;
  }

  conf.freeMemoryFractionToAllocate = vm["freeMemFraction"].as<float>();
  conf.chunkReservedGB = vm["chunkSize"].as<float>();
  conf.nRegions = vm["regions"].as<int>();
  conf.kernelLaunches = vm["launches"].as<int>();
  conf.nTests = vm["ntests"].as<int>();

  return true;
}

using o2::benchmark::ResultWriter;

int main(int argc, const char* argv[])
{

  o2::benchmark::benchmarkOpts opts;

  if (!parseArgs(opts, argc, argv)) {
    return -1;
  }

  std::shared_ptr<ResultWriter> writer = std::make_shared<ResultWriter>();

  o2::benchmark::GPUbenchmark<char> bm_char{opts, writer};
  bm_char.run();
  o2::benchmark::GPUbenchmark<int> bm_int{opts, writer};
  bm_int.run();
  o2::benchmark::GPUbenchmark<size_t> bm_size_t{opts, writer};
  bm_size_t.run();

  // save results
  writer.get()->saveToFile();

  return 0;
}
