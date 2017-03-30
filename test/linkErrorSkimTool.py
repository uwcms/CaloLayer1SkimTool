import FWCore.ParameterSet.Config as cms

from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('analysis')
options.register(
    "printout",
    False,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.bool,
    "Printout the error towers"
)
options.parseArguments()

from Configuration.StandardSequences.Eras import eras
process = cms.Process('SKIM'+('2' if options.printout else ''), eras.Run2_25ns)

process.options = cms.untracked.PSet(
    allowUnscheduled = cms.untracked.bool(True),
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

process.load("EventFilter.L1TXRawToDigi.caloLayer1Stage2Digis_cfi")

process.linkErrorFilter = cms.EDFilter("CaloLayer1LinkErrorFilter",
    ecalToken = cms.InputTag("l1tCaloLayer1Digis"),
    hcalToken = cms.InputTag("l1tCaloLayer1Digis"),
)

process.skimPath = cms.Path(
    process.l1tCaloLayer1Digis +
    process.linkErrorFilter
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
