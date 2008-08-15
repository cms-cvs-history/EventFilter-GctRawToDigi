import FWCore.ParameterSet.Config as cms

process = cms.Process('GctVmeToDigi')

#import FWCore.MessageLogger.data.MessageLogger.cfi
#process.MessageLogger = cms.Service ( 
#  destinations = cms.untracked.vstring ( "debug.log" ),
#  debuglog = cms.untracked.PSet ( threshold = cms.untracked.string ( "DEBUG" ) ),
#  debugModules = cms.untracked.vstring ( "TextToRaw", "GctRawToDigi" )
#)

process.source = cms.Source ( "EmptySource" )
  
process.maxEvents = cms.untracked.PSet ( input = cms.untracked.int32 ( 3564 ) )
  
process.gctRaw = cms.OutputModule( "TextToRaw",
#  filename = cms.untracked.string ( "eventsForEmyr2.txt" ),
  filename = cms.untracked.string("/home/jbrooke/patternCaptureOrbit_ts__2008_08_15__18h57m24s.txt"),
  GctFedId = cms.untracked.int32 ( 745 )
)
  
process.load('EventFilter/GctRawToDigi/l1GctHwDigis_cfi')
process.l1GctHwDigis.inputLabel = cms.InputTag( "gctRaw" )
process.l1GctHwDigis.verbose = cms.untracked.bool ( True )
process.l1GctHwDigis.unpackFibres = cms.untracked.bool ( True )
process.l1GctHwDigis.unpackInternEm = cms.untracked.bool ( True )

process.dumpRaw = cms.OutputModule ( "DumpFEDRawDataProduct",
  feds = cms.untracked.vint32 ( 745 ),
  dumpPayload = cms.untracked.bool ( True )
)

process.load('L1Trigger/L1GctAnalyzer/dumpGctDigis_cfi')
process.dumpGctDigis.rawInput = cms.untracked.InputTag( "l1GctHwDigis" )
process.dumpGctDigis.emuRctInput = cms.untracked.InputTag( "rctDigis" )
process.dumpGctDigis.emuGctInput = cms.untracked.InputTag( "gctDigis" )
process.dumpGctDigis.doHardware = cms.untracked.bool ( True )
process.dumpGctDigis.doEmulated = cms.untracked.bool ( False )
process.dumpGctDigis.doRctEm = cms.untracked.bool ( True )
process.dumpGctDigis.doInternEm = cms.untracked.bool ( False )
process.dumpGctDigis.doEm = cms.untracked.bool ( True )
process.dumpGctDigis.doJets = cms.untracked.bool ( True )
process.dumpGctDigis.doEnergySums = cms.untracked.bool ( True )

# DQM
process.load('DQMServices.Core.DQM_cfg')

process.load('DQM.L1TMonitor.L1TGCT_cfi')
process.l1tgct.disableROOToutput = cms.untracked.bool(False)
process.l1tgct.outputFile = cms.untracked.string('gctDqm.root')
process.l1tgct.gctCentralJetsSource = cms.InputTag("l1GctHwDigis","cenJets")
process.l1tgct.gctNonIsoEmSource = cms.InputTag("l1GctHwDigis","nonIsoEm")
process.l1tgct.gctForwardJetsSource = cms.InputTag("l1GctHwDigis","forJets")
process.l1tgct.gctIsoEmSource = cms.InputTag("l1GctHwDigis","isoEm")
process.l1tgct.gctEnergySumsSource = cms.InputTag("l1GctHwDigis","")
process.l1tgct.gctTauJetsSource = cms.InputTag("l1GctHwDigis","tauJets")

process.load('DQM.L1TMonitor.L1TRCT_cfi')
process.l1trct.disableROOToutput = cms.untracked.bool(False)
process.l1trct.outputFile = cms.untracked.string('gctDqm.root')
process.l1trct.rctSource = cms.InputTag("l1GctHwDigis","")

#process.p = cms.Path ( process.gctRaw * process.dumpRaw * process.l1GctHwDigis * process.dumpGctDigis )

process.p = cms.Path ( process.gctRaw * process.l1GctHwDigis * process.l1trct * process.l1tgct )

process.output = cms.OutputModule ( "PoolOutputModule",
  outputCommands = cms.untracked.vstring ( 
    "drop *",
    "keep *_l1GctHwDigis_*_*",
    "keep *_gctRaw_*_*"
  ),

  fileName = cms.untracked.string ( "gctDigis.root" )

)
process.out = cms.EndPath(process.output)


