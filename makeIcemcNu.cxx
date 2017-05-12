#include "AnitaConventions.h"
#include "AnitaGeomTool.h"
#include "UsefulAnitaEvent.h"
#include "CalibratedAnitaEvent.h"
#include "RawAnitaEvent.h"
#include "Adu5Pat.h"
#include "TimedAnitaHeader.h"
#include "PrettyAnitaHk.h"
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

  if(!(argc==3)){
    std::cerr << "Usage 1: " << argv[0] << " [run] [exponent]" << std::endl;
    return 1;
  }

  std::cout << argv[0] << "\t" << argv[1];
  std::cout << std::endl;
  Int_t irun = atoi(argv[1]);
  string iexp = argv[2];

  AnitaVersion::set(3);


  RawAnitaHeader*       simHeaderPtr  = NULL;
  UsefulAnitaEvent*     simEvPtr      = NULL;
  Adu5Pat*              simGpsPtr    = NULL;
  TruthAnitaEvent*      truthEvPtr   = NULL;
  
  RawAnitaHeader*       dataHeaderPtr = NULL;
  CalibratedAnitaEvent* dataCalEvPtr  = NULL;
  UsefulAnitaEvent*     dataEvPtr     = NULL;

  double weight;

  // DATA STUFF
  string anita3dataFolder="/unix/anita3/flight1415/root/";
  
  TFile *dHeadFile  = new TFile(Form("%s/run%i/timedHeadFile%iOfflineMask.root", anita3dataFolder.c_str(), irun, irun), "read");
  TFile *dEventFile = new TFile(Form("%s/run%i/calEventFile%i.root",             anita3dataFolder.c_str(), irun, irun), "read");

  TTree *dHeadTree  = (TTree*) dHeadFile->Get("headTree");
  TTree *dEventTree = (TTree*) dEventFile->Get("eventTree");

  dHeadTree->SetBranchAddress("header", &dataHeaderPtr);
  dEventTree->SetBranchAddress("event", &dataCalEvPtr);
  
  // SIMULATION STUFF

  string simDataFolder = "/unix/anita3/linda/SimulatedFiles/Test10/SignalOnly/Energy_E";
  simDataFolder += iexp;

  TChain *simHeadChain  = new TChain("headTree"       );
  TChain *simEventChain = new TChain("eventTree"      );
  TChain *simGpsChain   = new TChain("adu5PatTree"    );
  TChain *simTruthChain = new TChain("truthAnitaTree" );
  
  for (int isimrun=1; isimrun<11; isimrun++){
    simHeadChain ->Add(Form("%s/run%i/SimulatedAnitaHeadFile%i.root",   simDataFolder.c_str(), isimrun, isimrun ));
    simEventChain->Add(Form("%s/run%i/SimulatedAnitaEventFile%i.root",  simDataFolder.c_str(), isimrun, isimrun ));
    simGpsChain  ->Add(Form("%s/run%i/SimulatedAnitaGpsFile%i.root",    simDataFolder.c_str(), isimrun, isimrun ));
    simTruthChain->Add(Form("%s/run%i/SimulatedAnitaTruthFile%i.root",  simDataFolder.c_str(), isimrun, isimrun ));
  }

  
  simHeadChain ->SetBranchAddress("header", &simHeaderPtr );
  simEventChain->SetBranchAddress("event",  &simEvPtr     );
  simGpsChain  ->SetBranchAddress("pat",    &simGpsPtr    );
  simTruthChain->SetBranchAddress("truth",  &truthEvPtr   );

  simEventChain->SetBranchAddress("weight", &weight       );

  simHeadChain ->BuildIndex("realTime"   );
  simEventChain->BuildIndex("eventNumber");
  simGpsChain  ->BuildIndex("eventNumber");
  simTruthChain->BuildIndex("eventNumber");


  // OUTPUT STUFF

  string run_no = Form("%i", irun);


  string outputdir="/unix/anita3/linda/SimulatedFiles/Test10/SignalOnly/MinBiasEnergy_E";
  outputdir += iexp;

  outputdir += "/run"+run_no+"/";

  string outputAnitaFile =outputdir+"SimulatedAnitaEventFile"+run_no+".root";
  TFile *anitafileEvent = new TFile(outputAnitaFile.c_str(), "RECREATE");

  TTree *eventTree = new TTree("eventTree", "eventTree");
  eventTree->Branch("event",             &simEvPtr            );
  eventTree->Branch("run",               &irun,     "run/I"   );
  eventTree->Branch("weight",            &weight,   "weight/D");

  outputAnitaFile =outputdir+"SimulatedAnitaHeadFile"+run_no+".root";
  TFile *anitafileHead = new TFile(outputAnitaFile.c_str(), "RECREATE");

  TTree *headTree = new TTree("headTree", "headTree");
  headTree->Branch("header",  &simHeaderPtr           );
  headTree->Branch("weight",  &weight,      "weight/D");

  outputAnitaFile =outputdir+"SimulatedAnitaGpsFile"+run_no+".root";
  TFile *anitafileGps = new TFile(outputAnitaFile.c_str(), "RECREATE");

  UInt_t eventNumber;
  TTree *adu5PatTree = new TTree("adu5PatTree", "adu5PatTree");
  adu5PatTree->Branch("pat",          &simGpsPtr                   );
  adu5PatTree->Branch("eventNumber",  &simHeaderPtr->eventNumber,  "eventNumber/I");
  adu5PatTree->Branch("weight",       &weight,       "weight/D"     );

  outputAnitaFile = outputdir+"SimulatedAnitaTruthFile"+run_no+".root";
  TFile *anitafileTruth = new TFile(outputAnitaFile.c_str(), "RECREATE");

  TTree *truthAnitaTree = new TTree("truthAnitaTree", "Truth Anita Tree");
  truthAnitaTree->Branch("truth",     &truthEvPtr                   );

  Int_t nEntries = dHeadTree->GetEntries();

  Int_t fNumPoints  = 260;
  Int_t fNumChans   = 96;

  for (int ientry=0;ientry<nEntries;ientry+=20000){

    dHeadTree->GetEntry(ientry);

    // only use min bias triggers
    while ((dataHeaderPtr->trigType & (1<<0))>0 ){
      ientry++;
      dHeadTree->GetEntry(ientry);
    }

    dEventTree->GetEntry(ientry);
    if (dataHeaderPtr->eventNumber != dataCalEvPtr->eventNumber){
      cout << "Something is wrong here " << dataHeaderPtr->eventNumber << " " << dataCalEvPtr->eventNumber << endl;
    }

    dataEvPtr = new UsefulAnitaEvent(dataCalEvPtr, WaveCalType::kDefault);

    int newInd = simHeadChain->GetEntryNumberWithBestIndex(dataHeaderPtr->realTime);
    if (newInd<0){
      cout << "Index not found, skipping event " << endl;
      continue;
    }

    simHeadChain->GetEntry(newInd);   
    

    cout << dataHeaderPtr->realTime << " " << simHeaderPtr->realTime << " " << (dataHeaderPtr->realTime -simHeaderPtr->realTime) <<  endl;
 
    simEventChain->GetEntryWithIndex(simHeaderPtr->eventNumber);
    cout << simHeaderPtr->eventNumber << " " << simEvPtr->eventNumber << endl;


    for (int ichan = 0; ichan < fNumChans; ichan++){
      for (int ipoint = 0; ipoint < fNumPoints; ipoint++){
    	//    	cout <<  dataEvPtr->fTimes[ichan][ipoint] << " " << dataEvPtr->fVolts[ichan][ipoint] << endl;
    	simEvPtr->fVolts[ichan][ipoint]+=dataEvPtr->fVolts[ichan][ipoint];
      }
    }
    
    simGpsChain->GetEntryWithIndex(simHeaderPtr->eventNumber);
    simTruthChain->GetEntryWithIndex(simHeaderPtr->eventNumber);
    
    
    truthAnitaTree->Fill();
    headTree->Fill();
    eventTree->Fill();
    adu5PatTree->Fill();

    // delete truthEvPtr;
    // delete realEvPtr;
    // delete rawHeaderPtr;
    // delete Adu5PatPtr;


    delete dataEvPtr;
  }


  anitafileEvent->cd();
  eventTree->Write("eventTree");
  anitafileEvent->Close();
  delete anitafileEvent;

  anitafileHead->cd();
  headTree->Write("headTree");
  anitafileHead->Close();
  delete anitafileHead;

  anitafileGps->cd();
  adu5PatTree->Write("adu5PatTree");
  anitafileGps->Close();
  delete anitafileGps;

  anitafileTruth->cd();
  truthAnitaTree->Write("truthAnitaTree");
  anitafileTruth->Close();
  delete anitafileTruth;



}
