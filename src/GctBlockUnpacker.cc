#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "EventFilter/GctRawToDigi/src/GctBlockUnpacker.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/L1GlobalCaloTrigger/interface/L1GctInternEmCand.h"
#include "DataFormats/L1GlobalCaloTrigger/interface/L1GctEmCand.h"
#include "DataFormats/L1CaloTrigger/interface/L1CaloRegionDetId.h"

#include <iostream>
#include <cassert>

using std::cout;
using std::endl;
using std::pair;

// INITIALISE STATIC VARIABLES
GctBlockUnpacker::RctCrateMap GctBlockUnpacker::rctCrate_ = GctBlockUnpacker::RctCrateMap();
GctBlockUnpacker::BlockIdToUnpackFnMap GctBlockUnpacker::blockUnpackFn_ = GctBlockUnpacker::BlockIdToUnpackFnMap();
GctBlockUnpacker::BlockIdToEmCandIsoBoundMap GctBlockUnpacker::InternEmIsoBounds_ = GctBlockUnpacker::BlockIdToEmCandIsoBoundMap();

GctBlockUnpacker::GctBlockUnpacker() :
  srcCardRouting_(),
  rctEm_(0),
  rctCalo_(0),
  gctIsoEm_(0),
  gctNonIsoEm_(0),
  gctInternEm_(0),
  gctFibres_(0),
  gctJets_(NUM_JET_CATAGORIES),
  gctJetCounts_(0),
  gctEtTotal_(0),
  gctEtHad_(0),
  gctEtMiss_(0)
{
  static bool initClass = true;
  
  if(initClass)
  {
    initClass = false;
    
    rctCrate_[0x81] = 13;
    rctCrate_[0x89] = 9;
    rctCrate_[0xC1] = 4;
    rctCrate_[0xC9] = 0; 

    // Setup block unpack function map.
    blockUnpackFn_[0x00] = &GctBlockUnpacker::blockDoNothing;
    blockUnpackFn_[0x58] = &GctBlockUnpacker::blockToGctJetCand;
    blockUnpackFn_[0x59] = &GctBlockUnpacker::blockDoNothing;
    blockUnpackFn_[0x5a] = &GctBlockUnpacker::blockToGctJetCounts;
    blockUnpackFn_[0x5f] = &GctBlockUnpacker::blockDoNothing;
    blockUnpackFn_[0x68] = &GctBlockUnpacker::blockToGctEmCand;
    blockUnpackFn_[0x69] = &GctBlockUnpacker::blockToGctInternEmCand;
    blockUnpackFn_[0x6a] = &GctBlockUnpacker::blockToGctEnergySums;
    blockUnpackFn_[0x6b] = &GctBlockUnpacker::blockDoNothing;
    blockUnpackFn_[0x6f] = &GctBlockUnpacker::blockDoNothing; 
    blockUnpackFn_[0x80] = &GctBlockUnpacker::blockToGctInternEmCand;
    blockUnpackFn_[0x81] = &GctBlockUnpacker::blockToFibresAndToRctEmCand;
    blockUnpackFn_[0x83] = &GctBlockUnpacker::blockToGctInternEmCand;
    blockUnpackFn_[0x88] = &GctBlockUnpacker::blockToGctInternEmCand;
    blockUnpackFn_[0x89] = &GctBlockUnpacker::blockToFibresAndToRctEmCand;
    blockUnpackFn_[0x8b] = &GctBlockUnpacker::blockToGctInternEmCand;
    blockUnpackFn_[0xc0] = &GctBlockUnpacker::blockToGctInternEmCand;
    blockUnpackFn_[0xc1] = &GctBlockUnpacker::blockToFibresAndToRctEmCand;
    blockUnpackFn_[0xc3] = &GctBlockUnpacker::blockToGctInternEmCand;
    blockUnpackFn_[0xc8] = &GctBlockUnpacker::blockToGctInternEmCand;
    blockUnpackFn_[0xc9] = &GctBlockUnpacker::blockToFibresAndToRctEmCand;
    blockUnpackFn_[0xcb] = &GctBlockUnpacker::blockToGctInternEmCand;
    blockUnpackFn_[0xff] = &GctBlockUnpacker::blockToRctCaloRegions;
    
    // Setup Block ID map for pipeline payload positions of isolated Internal EM Cands.
    InternEmIsoBounds_[0x69] = IsoBoundaryPair(8,15);
    InternEmIsoBounds_[0x80] = IsoBoundaryPair(0, 9);
    InternEmIsoBounds_[0x83] = IsoBoundaryPair(0, 1);
    InternEmIsoBounds_[0x88] = IsoBoundaryPair(0, 7);
    InternEmIsoBounds_[0x8b] = IsoBoundaryPair(0, 1);
    InternEmIsoBounds_[0xc0] = IsoBoundaryPair(0, 9);
    InternEmIsoBounds_[0xc3] = IsoBoundaryPair(0, 1);
    InternEmIsoBounds_[0xc8] = IsoBoundaryPair(0, 7);
    InternEmIsoBounds_[0xcb] = IsoBoundaryPair(0, 1);
  }
}

GctBlockUnpacker::~GctBlockUnpacker() { }

// conversion
void GctBlockUnpacker::convertBlock(const unsigned char * data, GctBlockHeader& hdr)
{
  unsigned int id = hdr.id();
  unsigned int nSamples = hdr.nSamples();

  // if the block has no time samples, don't bother
  if ( nSamples < 1 ) { return; }

  // check block is valid
  if ( !hdr.valid() )
  {
    edm::LogError("GCT") << "Attempting to unpack an unidentified block\n" << hdr << endl;
    return;     
  }

  // check block doesn't have too many time samples
  if ( nSamples >= 0xf ) {
    edm::LogError("GCT") << "Cannot unpack a block with 15 or more time samples\n" << hdr << endl;
    return; 
  }

  // The header validity check above will protect against 
  // the map::find() method returning the end of the map,
  // assuming the GctBlockHeader definitions are up-to-date.
  (this->*blockUnpackFn_.find(id)->second)(data, hdr);  // Calls the correct unpack function, based on block ID.
}


// Output EM Candidates unpacking
void GctBlockUnpacker::blockToGctEmCand(const unsigned char * d, const GctBlockHeader& hdr)
{
  LogDebug("GCT") << "Unpacking GCT output EM Cands" << std::endl;

  const unsigned int id = hdr.id();
  const unsigned int nSamples = hdr.nSamples();

  const unsigned int catagoryOffset = nSamples * 4;  // Offset to jump from the non-iso electrons to the isolated ones.
  const unsigned int timeSampleOffset = nSamples * 2;  // Offset to jump to next candidate pair in the same time-sample.

  // Re-interpret pointer.  p will be pointing at the 16 bit word that
  // contains the rank0 non-isolated electron of the zeroth time-sample. 
  const uint16_t * p = reinterpret_cast<const uint16_t *>(d);

  for (unsigned int iso=0; iso<2; ++iso)  // loop over non-iso/iso candidate pairs
  {
    bool isoFlag = (iso==1);

    // Get the correct collection to put them in.
    L1GctEmCandCollection* em;
    if (isoFlag) { em = gctIsoEm_; }
    else { em = gctNonIsoEm_; }

    for (unsigned int bx=0; bx<nSamples; ++bx) // loop over time samples
    {
      // cand0Offset will give the offset on p to get the rank 0 candidate
      // of the correct catagory and timesample.
      const unsigned int cand0Offset = iso*catagoryOffset + bx*2;
      
      em->push_back(L1GctEmCand(p[cand0Offset], isoFlag, id, 0, bx));  // rank0 electron
      em->push_back(L1GctEmCand(p[cand0Offset + timeSampleOffset], isoFlag, id, 1, bx));  // rank1 electron
      em->push_back(L1GctEmCand(p[cand0Offset + 1], isoFlag, id, 2, bx));  // rank2 electron
      em->push_back(L1GctEmCand(p[cand0Offset + timeSampleOffset + 1], isoFlag, id, 3, bx));  // rank3 electron
    }
  }
}


// Internal EM Candidates unpacking
void GctBlockUnpacker::blockToGctInternEmCand(const unsigned char * d, const GctBlockHeader& hdr)
{
  LogDebug("GCT") << "Unpacking internal EM Cands" << std::endl;

  unsigned int id = hdr.id();
  unsigned int nSamples = hdr.nSamples();
  unsigned int numCandPairs = hdr.length();

  // Debug assertion to prevent problems if definitions not up to date.
  assert(InternEmIsoBounds_.find(id) != InternEmIsoBounds_.end());  

  unsigned int lowerIsoPairBound = InternEmIsoBounds_[id].first;
  unsigned int upperIsoPairBound = InternEmIsoBounds_[id].second;

  // Re-interpret pointer to 16 bits so it sees one candidate at a time.
  uint16_t * p = reinterpret_cast<uint16_t *>(const_cast<unsigned char *>(d));

  // Loop over timesamples (i.e. bunch crossings)
  for(unsigned int bx=0; bx < nSamples; ++bx)
  {
    // Loop over candidate pairs (i.e. each iteration unpacks a pair of candidates)
    for(unsigned int candPair = 0 ; candPair < numCandPairs ; ++candPair)
    {
      // Is the candidate electron pair an isolated pair or not?
      bool iso = ((candPair>=lowerIsoPairBound) && (candPair<=upperIsoPairBound));
      
      // Loop over the two electron candidates in each pair
      for(unsigned int i = 0 ; i < 2 ; ++i)
      { 
        unsigned offset = 2*(bx + candPair*nSamples) + i;
        uint16_t candRaw = p[offset]; 
        gctInternEm_->push_back( L1GctInternEmCand(candRaw, iso, id, candPair*2 + i, bx) );
      }
    }
  }
}


// Input EM Candidates unpacking
// this is the last time I deal the RCT bit assignment travesty!!!
void GctBlockUnpacker::blockToRctEmCand(const unsigned char * d, const GctBlockHeader& hdr)
{
  LogDebug("GCT") << "Unpacking RCT EM Cands" << std::endl;

  unsigned int id = hdr.id();
  unsigned int nSamples = hdr.nSamples();
  unsigned int length = hdr.length();

  // re-interpret pointer
  uint16_t * p = reinterpret_cast<uint16_t *>(const_cast<unsigned char *>(d));

  // arrays of source card data
  uint16_t sfp[2][4]; // [ cycle ] [ SFP ]
  uint16_t eIsoRank[4];
  uint16_t eIsoCard[4];
  uint16_t eIsoRgn[4];
  uint16_t eNonIsoRank[4];
  uint16_t eNonIsoCard[4];
  uint16_t eNonIsoRgn[4];
  uint16_t MIPbits[7][2];
  uint16_t QBits[7][2];

  unsigned int bx = 0;

  // loop over crates
  for (unsigned int crate=rctCrate_[id]; crate<rctCrate_[id]+length/3; ++crate) {

    // read SC SFP words
    for (unsigned short iSfp=0 ; iSfp<4 ; ++iSfp) {
      for (unsigned short cyc=0 ; cyc<2 ; ++cyc) {
        if (iSfp==0) { sfp[cyc][iSfp] = 0; } // muon bits
        else {                               // EM candidate
          sfp[cyc][iSfp] = *p;
          ++p;
        }
      }
      p = p + 2*(nSamples-1);
    }

    // fill SC arrays
    srcCardRouting_.SFPtoEMU(eIsoRank, eIsoCard, eIsoRgn, eNonIsoRank, eNonIsoCard, eNonIsoRgn, MIPbits, QBits, sfp);
    
    // create EM cands
    for (unsigned short int i=0; i<4; ++i) {
      rctEm_->push_back( L1CaloEmCand( eIsoRank[i], eIsoRgn[i], eIsoCard[i], crate, true, i, bx) );
    }
    for (unsigned short int i=0; i<4; ++i) {
      rctEm_->push_back( L1CaloEmCand( eNonIsoRank[i], eNonIsoRgn[i], eNonIsoCard[i], crate, false, i, bx) );
    }
  }
}


// Fibre unpacking
void GctBlockUnpacker::blockToFibres(const unsigned char * d, const GctBlockHeader& hdr)
{
  LogDebug("GCT") << "Unpacking GCT Fibres" << std::endl;
  
  unsigned int id = hdr.id();
  unsigned int nSamples = hdr.nSamples();
  unsigned int length = hdr.length();

  // re-interpret pointer
  uint32_t * p = reinterpret_cast<uint32_t *>(const_cast<unsigned char *>(d));

  for (unsigned int i=0; i<length; ++i) {
    for (unsigned int bx=0; bx<nSamples; ++bx) {
      gctFibres_->push_back( L1GctFibreWord(*p, id, i, bx) );
      ++p;
    }
  } 
}

void GctBlockUnpacker::blockToFibresAndToRctEmCand(const unsigned char * d, const GctBlockHeader& hdr)
{
  this->blockToRctEmCand(d, hdr);
  this->blockToFibres(d, hdr);
}

void GctBlockUnpacker::blockToGctJetCand(const unsigned char * d, const GctBlockHeader& hdr)
{
  LogDebug("GCT") << "Unpacking GCT output Jet Cands" << std::endl;
  
  const unsigned int id = hdr.id();  // Capture block ID.
  const unsigned int nSamples = hdr.nSamples();  // Number of time-samples.
  
  const unsigned int catagoryOffset = nSamples * 4;  // Offset to jump from one jet catagory to the next.
  const unsigned int timeSampleOffset = nSamples * 2;  // Offset to jump to next candidate pair in the same time-sample.

  // Re-interpret block payload pointer to 16 bits so it sees one candidate at a time.
  // p points to the start of the block payload, at the rank0 tau jet candidate.
  const uint16_t * p = reinterpret_cast<const uint16_t *>(d);
  
  // Loop over the different catagories of jets
  for(unsigned int iCat = 0 ; iCat < NUM_JET_CATAGORIES ; ++iCat)
  {
    assert(gctJets_.at(iCat)->empty()); // The supplied vector should be empty.

    bool tauflag = (iCat == TAU_JETS);
    bool forwardFlag = (iCat == FORWARD_JETS);
    
    // Loop over the different timesamples (bunch crossings).
    for(unsigned int bx = 0 ; bx < nSamples ; ++bx)
    {
      // cand0Offset will give the offset on p to get the rank 0 Jet Cand of the correct catagory and timesample.
      const unsigned int cand0Offset = iCat*catagoryOffset + bx*2;
      
      // Rank 0 Jet.
      gctJets_.at(iCat)->push_back(L1GctJetCand(p[cand0Offset], tauflag, forwardFlag, id, 0, bx));
      // Rank 1 Jet.
      gctJets_.at(iCat)->push_back(L1GctJetCand(p[cand0Offset + timeSampleOffset], tauflag, forwardFlag, id, 1, bx));
      // Rank 2 Jet.
      gctJets_.at(iCat)->push_back(L1GctJetCand(p[cand0Offset + 1],  tauflag, forwardFlag, id, 2, bx));
      // Rank 3 Jet.
      gctJets_.at(iCat)->push_back(L1GctJetCand(p[cand0Offset + timeSampleOffset + 1], tauflag, forwardFlag, id, 3, bx));      
    }
  }
}

void GctBlockUnpacker::blockToGctJetCounts(const unsigned char * d, const GctBlockHeader& hdr)
{
  LogDebug("GCT") << "Unpacking GCT output Jet Counts" << std::endl;
  
  /* 
   * Note that we are only unpacking one timesample of these, because the dataformat for
   * jet counts does not have timesample support. 
   */
  
  // Re-interpret block payload pointer to 32 bits so it sees six jet counts at a time.
  // p points to the start of the block payload, at the first six jet counts in timesample 0.
  const uint32_t * p = reinterpret_cast<const uint32_t *>(d);

  // The call to hdr.nSamples() in the below line gives the offset from the start of the block
  // payload for a 32-bit pointer to get to the second set of six jet counts in timesample 0.
  *gctJetCounts_ = L1GctJetCounts(p[0], p[hdr.nSamples()]);
}

void GctBlockUnpacker::blockToGctEnergySums(const unsigned char * d, const GctBlockHeader& hdr)
{
  LogDebug("GCT") << "Unpacking GCT output Energy Sums" << std::endl;
  
  /* 
   * Note that we are only unpacking one timesample of these, because the relevant dataformats
   * do not have timesample support. 
   */
  
  // Re-interpret block payload pointer to both 16 and 32 bits.
  const uint16_t * p16 = reinterpret_cast<const uint16_t *>(d);  // For getting Et + Ht (16-bit raw data each)
  const uint32_t * p32 = reinterpret_cast<const uint32_t *>(d);  // For getting Missing Et (32-bit raw data)
  
  *gctEtTotal_ = L1GctEtTotal(p16[0]);  // Et total (first 16 bits in block payload for timesample 0)
  *gctEtHad_ = L1GctEtHad(p16[1]);  // Et hadronic (second 16 bits in block payload for timesample 0)
  
  // The call to hdr.nSamples() in the below line gives the offset from the start of the block
  // payload for a 32-bit pointer to get to the missing Et data in timesample 0.
  *gctEtMiss_ = L1GctEtMiss(p32[hdr.nSamples()]);    
}

void GctBlockUnpacker::blockToRctCaloRegions(const unsigned char * d, const GctBlockHeader& hdr)
{
  // This method is one giant "temporary" hack for CMSSW_1_8_X.
  
  LogDebug("GCT") << "Unpacking RCT Calorimeter Regions" << std::endl;
  
  const int nSamples = hdr.nSamples();  // Number of time-samples.

  // Re-interpret block payload pointer to 16 bits
  const uint16_t * p16 = reinterpret_cast<const uint16_t *>(d);
  
  for(unsigned iCrate = 0 ; iCrate < 18 ; ++iCrate)
  {
    // Barrel and endcap regions
    for(unsigned iCard = 0 ; iCard < 7 ; ++iCard)
    {
      // Samples
      for(int16_t iSample = 0 ; iSample < nSamples ; ++iSample)
      {
        // Two regions per card (and per 32-bit word).
        for(unsigned iRegion = 0 ; iRegion < 2 ; ++iRegion)
        {
          L1CaloRegionDetId id(iCrate, iCard, iRegion);
          rctCalo_->push_back(L1CaloRegion(*p16, id.ieta(), id.iphi(), iSample));
          ++p16; //advance pointer
        }
      }
    }
    // Forward regions (8 regions numbered 0 through 7, packed in 4 sets of pairs)
    for(unsigned iRegionPairNum = 0 ; iRegionPairNum < 4 ; ++iRegionPairNum)
    {
      // Samples
      for(int16_t iSample = 0 ; iSample < nSamples ; ++iSample)
      {
        // two regions in a pair
        for(unsigned iPair = 0 ; iPair < 2 ; ++iPair)
        {
          // For forward regions, RCTCard=999
          L1CaloRegionDetId id(iCrate, 999, iRegionPairNum*2 + iPair);
          rctCalo_->push_back(L1CaloRegion(*p16, id.ieta(), id.iphi(), iSample));
          ++p16; //advance pointer
        }
      }
    }
  }
}
