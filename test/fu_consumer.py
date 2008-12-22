import FWCore.ParameterSet.Config as cms

process = cms.Process("FUEVENTCONSUMER")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True),
                                     makeTriggerResults = cms.untracked.bool(True),
                                     Rethrow = cms.untracked.vstring('ProductNotFound',
                                                                     'TooManyProducts',
                                                                     'TooFewProducts')
                                     )

process.MessageLogger = cms.Service("MessageLogger",
                                    destinations = cms.untracked.vstring('cout'),
                                    cout = cms.untracked.PSet(threshold = cms.untracked.string('FATAL')),
                                    log4cplus = cms.untracked.PSet(threshold = cms.untracked.string('INFO'))
                                    )

process.source = cms.Source("EventStreamHttpReader",
                            sourceURL = cms.string('http://cmsroc8.fnal.gov:50003/urn:xdaq-application:lid=30'),
                            consumerName = cms.untracked.string('Consumer FUEP'),
                            consumerPriority = cms.untracked.string('normal'),
                            headerRetryInterval = cms.untracked.int32(3),
                            maxEventRequestRate = cms.untracked.double(0.1),
                            SelectEvents = cms.untracked.PSet( SelectEvents = cms.vstring('*DQM') ),
                            SelectHLTOutput = cms.untracked.string('out4DQM'),
                            maxConnectTries = cms.untracked.int32(1)
                            )

process.contentAna = cms.EDAnalyzer("EventContentAnalyzer")

#process.p = cms.Path(process.contentAna)
