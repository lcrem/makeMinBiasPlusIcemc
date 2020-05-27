#include "AnitaConventions.h"
#include "AnitaGeomTool.h"
#include "UsefulAnitaEvent.h"
#include "CalibratedAnitaEvent.h"
#include "RawAnitaEvent.h"
#include "AnitaDataset.h"
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
  int whichanita=0;
  if((argc!=3)){
    std::cerr << "Usage : " << argv[0] << " [whichanita] [run]" << std::endl;
    return 1;
  } else {
    std::cout << argv[0] << "\t" << argv[1] << "\t" << argv[2];
    std::cout << std::endl;
    whichanita = atoi(argv[1]);
    irun = atoi(argv[2]);
    if (whichanita!=3 && whichanita!=4) return -1;
  }

  if (whichanita==3 && irun>256 && irun<264) return -1;

  // AnitaVersion::set(whichanita);

  // RawAnitaHeader*       dataHeaderPtr = NULL;
  // CalibratedAnitaEvent* dataCalEvPtr  = NULL;
  // RawAnitaEvent*        dataRawEvPtr  = NULL;

  string anitadataFolder="";
  // DATA STUFF
  string anita3dataFolder="/unix/anita3/flight1415/root/";
  string anita4dataFolder="/unix/anita4/flight2016/root/";
  
  if (whichanita==3) anitadataFolder+=anita3dataFolder;
  else if (whichanita==4) anitadataFolder+=anita4dataFolder;


  AnitaDataset d(irun);

  // TChain *dHeadChain  = new TChain("headTree");
  // TChain *dEventChain = new TChain("eventTree");

  // dHeadChain->Add(Form("%s/run%i/timedHeadFile%i.root",       anitadataFolder.c_str(), irun, irun));
  // if (whichanita==3){
  //   dEventChain->Add(Form("%s/run%i/calEventFile%i.root",       anitadataFolder.c_str(), irun, irun));
  // } else if (whichanita==4){
  //   dEventChain->Add(Form("%s/run%i/eventFile%i.root",       anitadataFolder.c_str(), irun, irun));
  // }

  // dHeadChain->SetBranchAddress("header", &dataHeaderPtr);
  // if (whichanita==3)  dEventChain->SetBranchAddress("event", &dataCalEvPtr);
  // else if (whichanita==4)   dEventChain->SetBranchAddress("event", &dataRawEvPtr);
  // dHeadChain ->BuildIndex("realTime" );
  // dEventChain->BuildIndex("eventNumber");

  // OUTPUT STUFF

  TFile *anitafileEvent = new TFile(Form("%s/run%i/minBiasEventFile%i.root", anitadataFolder.c_str(), irun, irun), "RECREATE");

  TTree *eventTree = new TTree("eventTree", "eventTree");
  //  eventTree->Branch("event",             &dataCalEvPtr            );
  eventTree->Branch("event",             d.calibrated()            );

  TFile *anitafileHead = new TFile(Form("%s/run%i/minBiasHeadFile%i.root", anitadataFolder.c_str(), irun, irun), "RECREATE");

  TTree *headTree = new TTree("headTree", "headTree");
  //  headTree->Branch("header",  &dataHeaderPtr           );
  headTree->Branch("header",  d.header()           );

  Int_t nEntries = d.N();

  for (int ientry=0;ientry<nEntries;ientry++){

    d.getEntry(ientry);

    if ( d.header()->getTriggerBitRF()>0 ) continue;

    if (d.header()->eventNumber!=d.calibrated()->eventNumber){
      cout << d.header()->eventNumber << " " << d.calibrated()->eventNumber << endl;
      continue;
    }
    // dHeadChain->GetEntry(ientry);
    
    // // only use min bias triggers
    // if ((dataHeaderPtr->trigType & (1<<0))>0 ) continue;   

    // dEventChain->GetEntry(ientry);

    // if (whichanita==3 && dataHeaderPtr->eventNumber != dataCalEvPtr->eventNumber){
    //   cout << "Something is wrong here " << dataHeaderPtr->eventNumber << " " << dataCalEvPtr->eventNumber << " . Skipping this event."<< endl;
    //   continue;
    // }

    // if (whichanita==4){
    //   if (dataHeaderPtr->eventNumber != dataRawEvPtr->eventNumber){
    // 	cout << "Something is wrong here " << dataHeaderPtr->eventNumber << " " << dataRawEvPtr->eventNumber << " . Skipping this event."<< endl;
    // 	continue;
    //   }
    //   UsefulAnitaEvent *dataUsefulEvPtr = new UsefulAnitaEvent(dataRawEvPtr, WaveCalType::kNoCalib);
    //   dataCalEvPtr = new CalibratedAnitaEvent(dataUsefulEvPtr);
    //   delete dataUsefulEvPtr;
    // }
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
