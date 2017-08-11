from CRABClient.UserUtilities import config, getUsernameFromSiteDB
config = config()

config.General.workArea = 'crab_skim'
config.General.transferOutputs = True
config.General.transferLogs = False

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'mismatchSkimTool.py'

config.Data.inputDBS = 'global'
config.Data.splitting = 'LumiBased'
config.Data.unitsPerJob = 100
config.Data.lumiMask = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions17/13TeV/PromptReco/Cert_294927-299649_13TeV_PromptReco_Collisions17_JSON.txt'
#config.Data.runRange = '299592-299598'
config.Data.outLFNDirBase = '/store/user/%s/layer1MismatchSkim/' % (getUsernameFromSiteDB())
config.Data.publication = False
config.Data.outputDatasetTag = 'Run2017C-v1'

config.Site.storageSite = 'T2_CH_CERN'

if __name__ == '__main__':
    from CRABAPI.RawCommand import crabCommand
    from CRABClient.ClientExceptions import ClientException
    from httplib import HTTPException

    def submit(config):
        try:
            crabCommand('submit', config = config)
        except HTTPException as hte:
            print "Failed submitting task: %s" % (hte.headers)
        except ClientException as cle:
            print "Failed submitting task: %s" % (cle)

    #for n in range(1,11):
    for n in range(1,2):
        config.General.requestName = 'Layer1MismatchSkim_ZeroBias%d_%s' % (n, config.Data.outputDatasetTag)
        config.Data.inputDataset = '/ZeroBias%d/Run2017C-v1/RAW' % n
        submit(config)
