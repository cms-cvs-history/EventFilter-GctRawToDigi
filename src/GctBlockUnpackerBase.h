#ifndef GCTBLOCKUNPACKERBASE_H_
#define GCTBLOCKUNPACKERBASE_H_

#include <vector>
#include <map>
#include <utility>
#include <memory>

#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"

#include "DataFormats/L1CaloTrigger/interface/L1CaloCollections.h"
#include "DataFormats/L1GlobalCaloTrigger/interface/L1GctCollections.h"
#include "DataFormats/L1GlobalCaloTrigger/interface/L1GctEtSums.h"
#include "DataFormats/L1GlobalCaloTrigger/interface/L1GctJetCounts.h"

#include "L1Trigger/TextToDigi/src/SourceCardRouting.h"

#include "EventFilter/GctRawToDigi/src/GctBlockHeaderBase.h"

/*!
* \class GctBlockUnpackerBase
* \brief Abstract base class for common unpack functionality/interface
* 
* This class provides the common interface/functionality for the various
* concrete unpack classes needed to provide backwards compatiblity
* with older data.
*
* \author Robert Frazier
* $Revision: 1.11 $
* $Date: 2008/06/14 13:33:10 $
*/ 


// *******************************************************************
// ***  THE UNPACK PROCESS MUST NEVER THROW ANY KIND OF EXCEPTION! *** 
// *******************************************************************

class GctBlockUnpackerBase
{
protected:

  /// An enum for use with central, forward, and tau jet cand collections vector(s).
  /*! Note that the order here mimicks the order in the RAW data format. */
  enum JetCandCatagory { TAU_JETS, FORWARD_JETS, CENTRAL_JETS, NUM_JET_CATEGORIES };


public:

  /// Constructor.
  /*! \param hltMode - set true to unpack only BX zero and GCT output data (i.e. to run as quick as possible).*/
  explicit GctBlockUnpackerBase(bool hltMode = false);
  
  virtual ~GctBlockUnpackerBase(); ///< Destructor.
  
  /// set collection pointers
  /// when unpacking set these to empty collections that will be filled.
  void setRctEmCollection(L1CaloEmCollection* coll) { rctEm_ = coll; }
  void setRctCaloRegionCollection(L1CaloRegionCollection * coll) { rctCalo_ = coll; }
  void setIsoEmCollection(L1GctEmCandCollection* coll) { gctIsoEm_ = coll; }
  void setNonIsoEmCollection(L1GctEmCandCollection* coll) { gctNonIsoEm_ = coll; }
  void setInternEmCollection(L1GctInternEmCandCollection* coll) { gctInternEm_ = coll; }
  void setFibreCollection(L1GctFibreCollection* coll) { gctFibres_ = coll; }
  void setTauJetCollection(L1GctJetCandCollection* coll) { gctJets_.at(TAU_JETS) = coll; }
  void setForwardJetCollection(L1GctJetCandCollection* coll) { gctJets_.at(FORWARD_JETS) = coll; }
  void setCentralJetCollection(L1GctJetCandCollection* coll) { gctJets_.at(CENTRAL_JETS) = coll; }

  void setJetCountsCollection(L1GctJetCountsCollection* jetCounts) { gctJetCounts_ = jetCounts; }
  void setEtTotalCollection(L1GctEtTotalCollection* etTotal) { gctEtTotal_ = etTotal; }
  void setEtHadCollection(L1GctEtHadCollection* etHad) { gctEtHad_ = etHad; }
  void setEtMissCollection(L1GctEtMissCollection* etMiss) { gctEtMiss_ = etMiss; }

  void setInternJetDataCollection(L1GctInternJetDataCollection* coll) { gctInternJetData_ = coll; }
  void setInternEtSumCollection(L1GctInternEtSumCollection* coll) { gctInternEtSums_ = coll; }

  /// Get digis from the block - will return true if it succeeds, false otherwise.
  virtual bool convertBlock(const unsigned char * d, const GctBlockHeaderBase& hdr) = 0;

protected:
 
  // PROTECTED TYPDEFS, ENUMS, AND CLASS CONSTANTS. 
  /// Typedef for mapping block ID to RCT crate.
  typedef std::map<unsigned int, unsigned int> RctCrateMap;
  typedef std::map<unsigned int, unsigned int> RctJetCrateMap;

  /*! A typedef that holds the inclusive lower and upper bounds of pipeline
   *  gct trigger object pair number for isolated EM candidates.
   *  I.e. if the first and second trig object pair in the pipeline payload
   *  are isolated cands (4 iso in total), then the IsoBoundaryPair would
   *  be (0,1). */ 
  typedef std::pair<unsigned int, unsigned int> IsoBoundaryPair;
  /// A typdef for mapping Block IDs to IsoBoundaryPairs.
  typedef std::map<unsigned int, IsoBoundaryPair> BlockIdToEmCandIsoBoundMap;

  /// Typedef for a vector of pointers to L1GctJetCandCollection.
  typedef std::vector<L1GctJetCandCollection*> GctJetCandCollections;


  // PROTECTED MEMBERS

  // collections of RCT objects
  L1CaloEmCollection* rctEm_;  ///< RCT EM cands
  L1CaloRegionCollection* rctCalo_;  ///< RCT Calo regions

  // Output object pointers (collections should be empty, and will be filled)
  L1GctEmCandCollection* gctIsoEm_;  ///< GCT output isolated EM cands.
  L1GctEmCandCollection* gctNonIsoEm_;  ///< GCT output non-isolated EM cands.
  L1GctInternEmCandCollection* gctInternEm_;  ///< GCT internal EM Cands.  
  L1GctFibreCollection* gctFibres_;  ///< Fibre data.
  GctJetCandCollections gctJets_;  ///< Vector of pointers to the various jet candidate collections.
  L1GctJetCountsCollection* gctJetCounts_;  ///< Jet counts
  L1GctEtTotalCollection* gctEtTotal_;  ///< Total Et
  L1GctEtHadCollection* gctEtHad_;  /// Total Ht
  L1GctEtMissCollection* gctEtMiss_;  /// Missing Et
  L1GctInternJetDataCollection* gctInternJetData_; ///< GCT internal jet data
  L1GctInternEtSumCollection* gctInternEtSums_; ///< GCT internal Et sums


  // PROTECTED METHODS
  virtual RctCrateMap& rctCrateMap() = 0;  ///< get the RCT crate map.
  virtual const RctCrateMap& rctCrateMap() const = 0;  ///< get the RCT crate map.
 
  virtual RctJetCrateMap& rctJetCrateMap() = 0;  ///< get the RCT jet crate map.
  virtual const RctJetCrateMap& rctJetCrateMap() const = 0;  ///< get the RCT jet crate map.

  virtual BlockIdToEmCandIsoBoundMap& internEmIsoBounds() = 0;  ///< get the intern EM cand isolated boundary map.
  virtual const BlockIdToEmCandIsoBoundMap& internEmIsoBounds() const = 0;  ///< get the intern EM cand isolated boundary map.

  bool hltMode() const { return hltMode_; }  ///< Protected interface to get HLT optimisation mode flag.

  /// Performs checks on the block header to see if the block is possible to unpack or not.
  bool checkBlock(const GctBlockHeaderBase& hdr);

  // convert functions for each type of block
  /// unpack GCT internal EM Candidates
  void blockToGctInternEmCand(const unsigned char * d, const GctBlockHeaderBase& hdr);

  /// unpack RCT EM Candidates
  void blockToRctEmCand(const unsigned char * d, const GctBlockHeaderBase& hdr);

  /// unpack Fibres
  void blockToFibres(const unsigned char * d, const GctBlockHeaderBase& hdr);
  
  /// unpack Fibres and RCT EM Candidates
  void blockToFibresAndToRctEmCand(const unsigned char * d, const GctBlockHeaderBase& hdr);

  /// Unpack RCT Calo Regions
  void blockToRctCaloRegions(const unsigned char * d, const GctBlockHeaderBase& hdr);

  /// Unpack All RCT Calo Regions ('orrible hack for DigiToRaw use)
  void blockToAllRctCaloRegions(const unsigned char * d, const GctBlockHeaderBase& hdr);
  
  /// Do nothing
  void blockDoNothing(const unsigned char * d, const GctBlockHeaderBase& hdr) {}

  /// unpack GCT internal Et sums
  void blockToGctInternEtSums(const unsigned char * d, const GctBlockHeaderBase& hdr);

  /// unpack GCT internal output of leaf jet finder
  void blockToGctInternEtSumsAndJetCluster(const unsigned char * d, const GctBlockHeaderBase& hdr);

  /// unpack GCT internal wheel and conc jets
  void blockToGctTrigObjects(const unsigned char * d, const GctBlockHeaderBase& hdr);

  /// unpack GCT internal input to wheel jet sort
  void blockToGctJetClusterMinimal(const unsigned char * d, const GctBlockHeaderBase& hdr);

  /// unpack GCT internal shared jet finder info
  void blockToGctJetPreCluster(const unsigned char * d, const GctBlockHeaderBase& hdr);

  /// unpack GCT internal HF ring sums
  void blockToGctInternRingSums(const unsigned char * d, const GctBlockHeaderBase& hdr);

private:

  // PRIVATE MEMBER DATA

  /// If true, unpack only BX zero and GCT output data (i.e. to run as quickly as possible) 
  bool hltMode_;

  /// Source card mapping info
  SourceCardRouting srcCardRouting_;

};

#endif /*GCTBLOCKUNPACKERBASE_H_*/
