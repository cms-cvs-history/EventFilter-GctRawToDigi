
#ifndef GCTBLOCKCONVERTER_H
#define GCTBLOCKCONVERTER_H

#include <vector>
#include <map>
#include <memory>
#include <boost/cstdint.hpp>

#include "DataFormats/L1CaloTrigger/interface/L1CaloCollections.h"
#include "DataFormats/L1GlobalCaloTrigger/interface/L1GctCollections.h"



class GctBlockConverter {
 public:

  GctBlockConverter();
  ~GctBlockConverter();
  
  // recognised block ID
  bool validBlock(unsigned id);

  // return block length in 32-bit words
  unsigned blockLength(unsigned id);
    
  // convert a block
  void convertBlock(const unsigned char * d, unsigned id);

  // set collection pointers
  void setRctEmCollection(L1CaloEmCollection* coll) { rctEm = coll; }
  void setIsoEmCollection(L1GctEmCandCollection* coll) { gctIsoEm = coll; }
  void setNonIsoEmCollection(L1GctEmCandCollection* coll) { gctNonIsoEm = coll; }
  void setInterEmCollection(L1GctEmCandCollection* coll) { gctInterEm = coll; }

 private:

  // ToDo : write functions for each block
  void wordToGctEmCand(uint16_t w0, uint16_t w1, int line);
  void wordToGctInterEmCand(uint16_t w0, uint16_t w1, int line);


 private:

  // block info
  std::map<unsigned, unsigned> blockLength_;  // size of a block

  // map of unpack methods
  typedef  void (GctBlockConverter::*convFn)(uint16_t, uint16_t, int);
  std::map< unsigned, convFn > convertFn_;
  
  // collections of RCT objects
  L1CaloEmCollection* rctEm;

  // collections of output objects
  L1GctEmCandCollection* gctIsoEm;
  L1GctEmCandCollection* gctNonIsoEm;
  L1GctEmCandCollection* gctInterEm;  

};

#endif
