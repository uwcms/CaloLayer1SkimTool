// -*- C++ -*-
//
// Package:   llvvAnalysis/CaloLayer1MismatchFilter
// Class:    CaloLayer1MismatchFilter
// 
/**\class CaloLayer1MismatchFilter CaloLayer1MismatchFilter.cc llvvAnalysis/CaloLayer1MismatchFilter/plugins/CaloLayer1MismatchFilter.cc

 Description: [one line class summary]

 Implementation:
    [Notes on implementation]
*/
//
// Original Author:  Nicholas Charles Smith
//      Created:  Tue, 14 Jun 2016 22:18:53 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "DQM/L1TMonitor/interface/L1TStage2CaloLayer1.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"

#include "DataFormats/FEDRawData/interface/FEDHeader.h"
#include "DataFormats/TCDS/interface/TCDSRecord.h"

//
// class declaration
//

class CaloLayer1MismatchFilter : public edm::stream::EDFilter<> {
  public:
    explicit CaloLayer1MismatchFilter(const edm::ParameterSet&);
    ~CaloLayer1MismatchFilter();

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  private:
    virtual void beginStream(edm::StreamID) override;
    virtual bool filter(edm::Event&, const edm::EventSetup&) override;
    virtual void endStream() override;

    //virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
    //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
    //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
    //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

    // ----------member data ---------------------------
    edm::EDGetTokenT<EcalTrigPrimDigiCollection> ecalTPSourceSent_;
    edm::EDGetTokenT<HcalTrigPrimDigiCollection> hcalTPSourceSent_;
    edm::EDGetTokenT<EcalTrigPrimDigiCollection> ecalTPSourceRecd_;
    edm::EDGetTokenT<HcalTrigPrimDigiCollection> hcalTPSourceRecd_;
    edm::EDGetTokenT<FEDRawDataCollection> fedData_;

    bool filterEcalMismatch_;
    bool filterHcalMismatch_;
    bool filterEcalLinkErrors_;
    bool filterHcalLinkErrors_;
    bool printout_;

    std::vector<std::pair<EcalTriggerPrimitiveDigi, EcalTriggerPrimitiveDigi> > ecalTPSentRecd_;
    std::vector<std::pair<HcalTriggerPrimitiveDigi, HcalTriggerPrimitiveDigi> > hcalTPSentRecd_;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
CaloLayer1MismatchFilter::CaloLayer1MismatchFilter(const edm::ParameterSet& ps):
  ecalTPSourceSent_(consumes<EcalTrigPrimDigiCollection>(ps.getParameter<edm::InputTag>("ecalTPSourceSent"))),
  hcalTPSourceSent_(consumes<HcalTrigPrimDigiCollection>(ps.getParameter<edm::InputTag>("hcalTPSourceSent"))),
  ecalTPSourceRecd_(consumes<EcalTrigPrimDigiCollection>(ps.getParameter<edm::InputTag>("ecalTPSourceRecd"))),
  hcalTPSourceRecd_(consumes<HcalTrigPrimDigiCollection>(ps.getParameter<edm::InputTag>("hcalTPSourceRecd"))),
  fedData_(consumes<FEDRawDataCollection>(ps.getParameter<edm::InputTag>("fedRawData"))),
  filterEcalMismatch_(ps.getParameter<bool>("filterEcalMismatch")),
  filterHcalMismatch_(ps.getParameter<bool>("filterHcalMismatch")),
  filterEcalLinkErrors_(ps.getParameter<bool>("filterEcalLinkErrors")),
  filterHcalLinkErrors_(ps.getParameter<bool>("filterHcalLinkErrors")),
  printout_(ps.getParameter<bool>("printout"))
{
  ecalTPSentRecd_.reserve(28*2*72);
  hcalTPSentRecd_.reserve(41*2*72);
}


CaloLayer1MismatchFilter::~CaloLayer1MismatchFilter()
{
 
  // do anything here that needs to be done at destruction time
  // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called on each new Event  ------------
bool
CaloLayer1MismatchFilter::filter(edm::Event& event, const edm::EventSetup& iSetup)
{
  if ( false and printout_ ) {
    edm::Handle<FEDRawDataCollection> feds;
    event.getByToken(fedData_, feds);
    const FEDRawData& tcdsData = feds->FEDData(1024);
    TCDSRecord tcds(tcdsData.data());
    std::cout << "Addl. event info: bx=" << event.bunchCrossing() << ", orbit=" << event.orbitNumber() << ", l1a=" << tcds.getTriggerCount() << std::endl;
  }

  edm::Handle<EcalTrigPrimDigiCollection> ecalTPsSent;
  event.getByToken(ecalTPSourceSent_, ecalTPsSent);
  bool tccFullReadout = ( ecalTPsSent->size() == 28*72*2 );
  edm::Handle<EcalTrigPrimDigiCollection> ecalTPsRecd;
  event.getByToken(ecalTPSourceRecd_, ecalTPsRecd);

  ecalTPSentRecd_.clear();
  ComparisonHelper::zip(ecalTPsSent->begin(), ecalTPsSent->end(), 
                        ecalTPsRecd->begin(), ecalTPsRecd->end(), 
                        std::inserter(ecalTPSentRecd_, ecalTPSentRecd_.begin()), EcalTrigPrimDigiCollection::key_compare());

  edm::Handle<HcalTrigPrimDigiCollection> hcalTPsSent;
  event.getByToken(hcalTPSourceSent_, hcalTPsSent);
  HcalTrigPrimDigiCollection hcalTPsSentCleaned;
  for(auto&& tp : *hcalTPsSent) {
    if ( tp.id().version() == 0 and std::abs(tp.id().ieta()) > 28 ) continue;
    hcalTPsSentCleaned.push_back(tp);
  }
  edm::Handle<HcalTrigPrimDigiCollection> hcalTPsRecd;
  event.getByToken(hcalTPSourceRecd_, hcalTPsRecd);

  hcalTPSentRecd_.clear();
  ComparisonHelper::zip(hcalTPsSentCleaned.begin(), hcalTPsSentCleaned.end(), 
                        hcalTPsRecd->begin(), hcalTPsRecd->end(), 
                        std::inserter(hcalTPSentRecd_, hcalTPSentRecd_.begin()), HcalTrigPrimDigiCollection::key_compare());


  int nEcalLinkErrors{0};
  int nEcalMismatch{0};
  int nHcalLinkErrors{0};
  int nHcalMismatch{0};

  for ( const auto& tpPair : ecalTPSentRecd_ ) {
    const auto& sentTp = tpPair.first;
    const auto& recdTp = tpPair.second;
    const bool towerMasked = recdTp.sample(0).raw() & (1<<13);
    const bool linkMasked  = recdTp.sample(0).raw() & (1<<14);
    const bool linkError   = recdTp.sample(0).raw() & (1<<15);

    if ( linkError ) {
      nEcalLinkErrors++;
      if ( printout_ && filterEcalLinkErrors_ ) std::cout << "Found ECAL link error at ieta=" << std::dec << sentTp.id().ieta() << ", iphi=" << sentTp.id().iphi() << std::endl;
    }

    if ( sentTp.compressedEt() == recdTp.compressedEt() && sentTp.fineGrain() == recdTp.fineGrain() ) {
      // Full match
    }
    else if ( towerMasked || linkMasked ) {
      // Don't compare
    }
    else if ( sentTp.compressedEt() < 0 && !tccFullReadout ) {
      // Don't compare
    }
    else {
      nEcalMismatch++;
      if ( printout_ && filterEcalMismatch_ ) {
        std::cout << "Found ECAL TP mismatch at ieta=" << sentTp.id().ieta() << ", iphi=" << sentTp.id().iphi() << std::endl;
        std::cout << "\tSent TP raw: 0x" << std::internal << std::setfill('0') << std::setw(4) << std::hex << sentTp.sample(0).raw() << std::dec << std::endl;
        std::cout << "\tRecd TP raw: 0x" << std::internal << std::setfill('0') << std::setw(4) << std::hex << recdTp.sample(0).raw() << std::dec << std::endl;
      }
    }
  }

  for ( const auto& tpPair : hcalTPSentRecd_ ) {
    const auto& sentTp = tpPair.first;
    const auto& recdTp = tpPair.second;
    const bool towerMasked = recdTp.sample(0).raw() & (1<<13);
    const bool linkMasked  = recdTp.sample(0).raw() & (1<<14);
    const bool linkError   = recdTp.sample(0).raw() & (1<<15);

    if ( linkError ) {
      nHcalLinkErrors++;
      if ( printout_ && filterHcalLinkErrors_ ) std::cout << "Found HCAL link error at ieta=" << sentTp.id().ieta() << ", iphi=" << sentTp.id().iphi() << std::endl;
    }

    if ( sentTp.SOI_compressedEt() == recdTp.SOI_compressedEt() && sentTp.SOI_fineGrain() == recdTp.SOI_fineGrain() && 
          (sentTp.SOI_fineGrain(1) == recdTp.SOI_fineGrain(1) or std::abs(sentTp.id().ieta()) <= 28)
       ) {
      // Full match
    }
    else if ( towerMasked || linkMasked ) {
      // Don't compare
    }
    else {
      nHcalMismatch++;
      if ( printout_ && filterHcalMismatch_ ) {
        std::cout << "Found HCAL TP mismatch at ieta=" << std::dec << sentTp.id().ieta() << ", iphi=" << sentTp.id().iphi() << std::endl;
        std::cout << "\tSent TP raw: 0x" << std::internal << std::setfill('0') << std::setw(4) << std::hex << sentTp.sample(sentTp.presamples()).raw() << std::dec << std::endl;
        std::cout << "\tRecd TP raw: 0x" << std::internal << std::setfill('0') << std::setw(4) << std::hex << recdTp.sample(0).raw() << std::dec << std::endl;
        std::cout << "\tAddl sent TP info:" << std::endl << sentTp << "-------" << std::endl;
        std::cout << "\tAddl recd TP info:" << std::endl << recdTp << "-------" << std::endl;
      }
    }
  }

  if ( nEcalMismatch > 0 && filterEcalMismatch_ ) return true;
  if ( nHcalMismatch > 0 && filterHcalMismatch_ ) return true;
  if ( nEcalLinkErrors > 0 && filterEcalLinkErrors_ ) return true;
  if ( nHcalLinkErrors > 0 && filterHcalLinkErrors_ ) return true;
  return false;
}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void
CaloLayer1MismatchFilter::beginStream(edm::StreamID)
{
}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void
CaloLayer1MismatchFilter::endStream() {
}

// ------------ method called when starting to processes a run  ------------
/*
void
CaloLayer1MismatchFilter::beginRun(edm::Run const&, edm::EventSetup const&)
{ 
}
*/
 
// ------------ method called when ending the processing of a run  ------------
/*
void
CaloLayer1MismatchFilter::endRun(edm::Run const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when starting to processes a luminosity block  ------------
/*
void
CaloLayer1MismatchFilter::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when ending the processing of a luminosity block  ------------
/*
void
CaloLayer1MismatchFilter::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
CaloLayer1MismatchFilter::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}
//define this as a plug-in
DEFINE_FWK_MODULE(CaloLayer1MismatchFilter);
