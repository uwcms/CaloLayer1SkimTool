import FWCore.ParameterSet.Config as cms

from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('analysis')
options.register(
    "ecalMsm",
    True,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.bool,
    "Filter ECAL mismatches"
)
options.register(
    "hcalMsm",
    True,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.bool,
    "Filter HCAL mismatches"
)
options.register(
    "ecalErr",
    True,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.bool,
    "Filter ECAL link errors"
)
options.register(
    "hcalErr",
    True,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.bool,
    "Filter HCAL link errors"
)
options.register(
    "printout",
    False,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.bool,
    "Printout the mismatched towers"
)
options.parseArguments()

from Configuration.StandardSequences.Eras import eras
process = cms.Process('SKIM'+('2' if options.printout else ''), eras.Run2_2018)

process.options = cms.untracked.PSet(
    allowUnscheduled = cms.untracked.bool(False),
    wantSummary = cms.untracked.bool(True),
)

process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 1 if options.printout else 1000

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents)
)

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        options.inputFiles
    ),
    secondaryFileNames = cms.untracked.vstring()
)

process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase1_2018_realistic', '')

# Required to load EcalMappingRecord
process.load("Configuration.StandardSequences.GeometryRecoDB_cff")

process.load("Configuration.StandardSequences.RawToDigi_Data_cff")    
process.load("EventFilter.L1TXRawToDigi.caloLayer1Stage2Digis_cfi")

process.mismatchFilter = cms.EDFilter("CaloLayer1MismatchFilter",
    ecalTPSourceSent = cms.InputTag("ecalDigis","EcalTriggerPrimitives"),
    hcalTPSourceSent = cms.InputTag("hcalDigis"),
    ecalTPSourceRecd = cms.InputTag("l1tCaloLayer1Digis"),
    hcalTPSourceRecd = cms.InputTag("l1tCaloLayer1Digis"),
    fedRawData = cms.InputTag("rawDataCollector"),
    filterEcalMismatch = cms.bool(options.ecalMsm),
    filterHcalMismatch = cms.bool(options.hcalMsm),
    filterEcalLinkErrors = cms.bool(options.ecalErr),
    filterHcalLinkErrors = cms.bool(options.hcalErr),
    printout = cms.bool(options.printout),
)

process.skimPath = cms.Path(
    process.ecalDigis +
    process.hcalDigis +
    process.l1tCaloLayer1Digis +
    process.mismatchFilter
)

process.skimOutput = cms.OutputModule("PoolOutputModule",
    compressionAlgorithm = cms.untracked.string('LZMA'),
    compressionLevel = cms.untracked.int32(4),
    dropMetaData = cms.untracked.string('NONE'),
    fastCloning = cms.untracked.bool(True),
    fileName = cms.untracked.string(options.outputFile),
    overrideInputFileSplitLevels = cms.untracked.bool(False),
    outputCommands = cms.untracked.vstring(
        'keep *',
    ),
    SelectEvents = cms.untracked.PSet(
        SelectEvents = cms.vstring(
            'skimPath',
        )
    ),
)
process.output_step = cms.EndPath(process.skimOutput)

process.schedule = cms.Schedule(
    process.skimPath,
    process.output_step,
)
