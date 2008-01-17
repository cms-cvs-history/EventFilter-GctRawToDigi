
#ifndef GCTBLOCKPACKER_H
#define GCTBLOCKPACKER_H

// C++ headers
#include <vector>
#include <map>
#include <memory>
#include <boost/cstdint.hpp>

// CMSSW DataFormats headers
#include "DataFormats/L1CaloTrigger/interface/L1CaloCollections.h"
#include "DataFormats/L1GlobalCaloTrigger/interface/L1GctCollections.h"
#include "DataFormats/L1GlobalCaloTrigger/interface/L1GctEtSums.h"
#include "DataFormats/L1GlobalCaloTrigger/interface/L1GctJetCounts.h"

// Sourcecard routing
#include "L1Trigger/TextToDigi/src/SourceCardRouting.h"


class GctBlockPacker
{
 public:

  GctBlockPacker();
  ~GctBlockPacker();

  uint32_t bcId() { return bcid_; }
  uint32_t evId() { return evid_; }
  
  void setBcId(uint32_t ev) { evid_ = ev; }
  void setEvId(uint32_t bc) { bcid_ = bc; }

  void writeGctHeader(unsigned char * d, uint16_t id, uint16_t nSamples);

  /// Writes the GCT Jet Cands block into an unsigned char array, starting at the position pointed to by d.
  /*! \param d must be pointing at the position where the Jet Cands block header should be written! */
  void writeGctJetCandsBlock(unsigned char * d, const L1GctJetCandCollection* cenJets,
                             const L1GctJetCandCollection* forJets, const L1GctJetCandCollection* tauJets);
  
  /// Writes the GCT Jet Counts block into an unsigned char array, starting at the position pointed to by d.
  /*! \param d must be pointing at the position where the Jet Counts block header should be written! */  
  void writeGctJetCountsBlock(unsigned char * d, const L1GctJetCounts* jetCounts);

  /// Writes the GCT EM block into an unsigned char array, starting at the position pointed to by d.
  /*! \param d must be pointing at the position where the EM block header should be written! */
  void writeGctEmBlock(unsigned char * d, const L1GctEmCandCollection* iso, const L1GctEmCandCollection* nonIso);

  /// Writes the GCT Energy Sums block into an unsigned char array, starting at the position pointed to by d.
  /*! \param d must be pointing at the position where the Energy Sums block header should be written! */
  void writeGctEnergySumsBlock(unsigned char * d, const L1GctEtTotal* etTotal,
                               const L1GctEtHad* etHad, const L1GctEtMiss* etMiss);

  /// Writes the 4 RCT EM Candidate blocks.
  void writeRctEmCandBlocks(unsigned char * d, const L1CaloEmCollection * rctEm);

  /// Writes the giant hack that is the RCT Calo Regions block.
  void writeRctCaloRegionBlock(unsigned char * d, const L1CaloRegionCollection * rctCalo);

 private:

  /// An enum for use with central, forward, and tau jet cand collections vector(s).
  /*! Note that the order here mimicks the order in the RAW data format. */
  enum JetCandCatagory { TAU_JETS, FORWARD_JETS, CENTRAL_JETS, NUM_JET_CATAGORIES };

  /// Typedef for mapping block ID to the first RCT crate in that block
  typedef std::map<unsigned int, unsigned int> RctCrateMap;
  
  uint32_t bcid_;
  uint32_t evid_;

  /// Source card mapping info
  SourceCardRouting srcCardRouting_;

  /// Map to relate capture block ID to the RCT crate the data originated from.
  static RctCrateMap rctCrate_;
  
  /// Struct of all data needed for running the emulator to SFP (sourcecard optical output) conversion.
  struct EmuToSfpData
  {
    // Input data.
    unsigned short eIsoRank[4];
    unsigned short eIsoCardId[4];
    unsigned short eIsoRegionId[4];
    unsigned short eNonIsoRank[4];
    unsigned short eNonIsoCardId[4];
    unsigned short eNonIsoRegionId[4];
    unsigned short mipBits[7][2];
    unsigned short qBits[7][2];
    // Output data.
    unsigned short sfp[2][4]; // [ cycle ] [ output number ]
  };

  
};

#endif
