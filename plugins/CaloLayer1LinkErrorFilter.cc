// -*- C++ -*-
//
// Package:   llvvAnalysis/CaloLayer1LinkErrorFilter
// Class:    CaloLayer1LinkErrorFilter
// 
/**\class CaloLayer1LinkErrorFilter CaloLayer1LinkErrorFilter.cc llvvAnalysis/CaloLayer1LinkErrorFilter/plugins/CaloLayer1LinkErrorFilter.cc

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

#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"

//
// class declaration
//

class CaloLayer1LinkErrorFilter : public edm::stream::EDFilter<> {
  public:
    explicit CaloLayer1LinkErrorFilter(const edm::ParameterSet&);
    ~CaloLayer1LinkErrorFilter();

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
    edm::EDGetTokenT<EcalTrigPrimDigiCollection> ecalTPSource;
    edm::EDGetTokenT<HcalTrigPrimDigiCollection> hcalTPSource;
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
CaloLayer1LinkErrorFilter::CaloLayer1LinkErrorFilter(const edm::ParameterSet& iConfig):
  ecalTPSource(consumes<EcalTrigPrimDigiCollection>(iConfig.getParameter<edm::InputTag>("ecalToken"))),
  hcalTPSource(consumes<HcalTrigPrimDigiCollection>(iConfig.getParameter<edm::InputTag>("hcalToken")))
{
  //now do what ever initialization is needed

}


CaloLayer1LinkErrorFilter::~CaloLayer1LinkErrorFilter()
{
 
  // do anything here that needs to be done at destruction time
  // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called on each new Event  ------------
bool
CaloLayer1LinkErrorFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<EcalTrigPrimDigiCollection> ecalTPs;
  iEvent.getByToken(ecalTPSource, ecalTPs);
  edm::Handle<HcalTrigPrimDigiCollection> hcalTPs;
  iEvent.getByToken(hcalTPSource, hcalTPs);

  bool linkErrorDetected = false;

  for ( const auto& ecalTp : *ecalTPs ) {
    if(((ecalTp.sample(0).raw()>>15)&1)==1){
      linkErrorDetected = true;
    }
  }

  for ( const auto& hcalTp : *hcalTPs ) {
    if(((hcalTp.sample(0).raw()>>15)&1)==1){
      linkErrorDetected = true;
    }
  }

  return linkErrorDetected;
}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void
CaloLayer1LinkErrorFilter::beginStream(edm::StreamID)
{
}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void
CaloLayer1LinkErrorFilter::endStream() {
}

// ------------ method called when starting to processes a run  ------------
/*
void
CaloLayer1LinkErrorFilter::beginRun(edm::Run const&, edm::EventSetup const&)
{ 
}
*/
 
// ------------ method called when ending the processing of a run  ------------
/*
void
CaloLayer1LinkErrorFilter::endRun(edm::Run const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when starting to processes a luminosity block  ------------
/*
void
CaloLayer1LinkErrorFilter::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when ending the processing of a luminosity block  ------------
/*
void
CaloLayer1LinkErrorFilter::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
CaloLayer1LinkErrorFilter::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}
//define this as a plug-in
DEFINE_FWK_MODULE(CaloLayer1LinkErrorFilter);
