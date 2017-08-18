#include "AnitaConventions.h"
#include "AnitaGeomTool.h"
#include "UsefulAnitaEvent.h"
#include "CalibratedAnitaEvent.h"
#include "RawAnitaEvent.h"
#include "Adu5Pat.h"
#include "TimedAnitaHeader.h"
#include "PrettyAnitaHk.h"
#include "AnalysisWaveform.h"
#include "ProgressBar.h"
#include "RawAnitaHeader.h"
#include "FFTtools.h"
#include "TruthAnitaEvent.h"

#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TH2.h"
#include "TColor.h"
#include "TMath.h"
#include "TStyle.h"
#include "TSystem.h"

#include <iostream>
#include <fstream>

using namespace Acclaim;
using namespace std;




int main(int argc, char *argv[]){

  Int_t irun;

  if((argc!=2)){
    std::cerr << "Usage : " << argv[0] << " [run]" << std::endl;
    return 1;
  } else {
    std::cout << argv[0] << "\t" << argv[1];
    std::cout << std::endl;
    irun = atoi(argv[1]);
  }

  AnitaVersion::set(3);


  RawAnitaHeader*       dataHeaderPtr = NULL;
  CalibratedAnitaEvent* dataCalEvPtr  = NULL;

  // DATA STUFF
  string anita3dataFolder="/unix/anita3/flight1415/root/";

  TChain *dHeadChain  = new TChain("headTree");
  TChain *dEventChain = new TChain("eventTree");
  if (irun>256 && irun<264) return -1;
  dHeadChain->Add(Form("%s/run%i/timedHeadFile%iOfflineMask.root", anita3dataFolder.c_str(), irun, irun));
  dEventChain->Add(Form("%s/run%i/calEventFile%i.root",             anita3dataFolder.c_str(), irun, irun));

  
  dHeadChain->SetBranchAddress("header", &dataHeaderPtr);
  dEventChain->SetBranchAddress("event", &dataCalEvPtr);
  dHeadChain ->BuildIndex("realTime" );
  dEventChain->BuildIndex("eventNumber");

  // OUTPUT STUFF

  TFile *anitafileEvent = new TFile(Form("%s/run%i/minBiasEventFile%i.root", anita3dataFolder.c_str(), irun, irun), "RECREATE");

  TTree *eventTree = new TTree("eventTree", "eventTree");
  eventTree->Branch("event",             &dataCalEvPtr            );

  TFile *anitafileHead = new TFile(Form("%s/run%i/minBiasHeadFile%i.root", anita3dataFolder.c_str(), irun, irun), "RECREATE");

  TTree *headTree = new TTree("headTree", "headTree");
  headTree->Branch("header",  &dataHeaderPtr           );

  Int_t nEntries = dHeadChain->GetEntries();

  for (int ientry=0;ientry<nEntries;ientry++){

    dHeadChain->GetEntry(ientry);
    
    // only use min bias triggers
    if ((dataHeaderPtr->trigType & (1<<0))>0 ) continue;   

    dEventChain->GetEntry(ientry);

    if (dataHeaderPtr->eventNumber != dataCalEvPtr->eventNumber){
      cout << "Something is wrong here " << dataHeaderPtr->eventNumber << " " << dataCalEvPtr->eventNumber << " . Skipping this event."<< endl;
      continue;
    }

    eventTree->Fill();
    headTree->Fill();

  }


  anitafileEvent->cd();
  eventTree->Write("eventTree");
  anitafileEvent->Close();
  delete anitafileEvent;

  anitafileHead->cd();
  headTree->Write("headTree");
  anitafileHead->Close();
  delete anitafileHead;


}
