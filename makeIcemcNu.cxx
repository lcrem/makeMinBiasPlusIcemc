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

  Int_t isimrun;
  string iexp;
  string baseDir;
  int whichanita;
  
  if((argc!=5)){
    std::cerr << "Usage 1: " << argv[0] << " [simrun] [exponent] [simfolder] [whichanita]" << std::endl;
    return 1;
  } 

  std::cout << argv[0] << "\t" << argv[1];
  std::cout << std::endl;
  isimrun = atoi(argv[1]);
  iexp += argv[2];
  baseDir += argv[3];
  whichanita = atoi(argv[4]);


  string simDataFolder = baseDir + "/Energy_";
  string outputdir= baseDir + "/MinBiasEnergy_E";
  AnitaVersion::set(whichanita);


  RawAnitaHeader*       simHeaderPtr  = NULL;
  UsefulAnitaEvent*     simEvPtr      = NULL;
  Adu5Pat*              simGpsPtr    = NULL;
  TruthAnitaEvent*      truthEvPtr   = NULL;
  
  RawAnitaHeader*       dataHeaderPtr = NULL;
  CalibratedAnitaEvent* dataCalEvPtr  = NULL;
  UsefulAnitaEvent*     dataEvPtr     = NULL;

  double weight;

  // DATA STUFF
  string anita3dataFolder;
  int minrun, maxrun;
  int skip1, skip2;
  if (whichanita==3){
    minrun=130;
    maxrun=439;
    skip1 = 256;
    skip2 = 264;
    anita3dataFolder+="/unix/anita3/flight1415/root/";
  }else if (whichanita==4){
    minrun=41;
    maxrun=367;
    skip1=59;
    skip2=62;
    anita3dataFolder+="/unix/anita4/flight2016/root/";
  }
  TChain *dHeadChain  = new TChain("headTree");
  TChain *dEventChain = new TChain("eventTree");
  for (int irun=minrun; irun<maxrun; irun++){
    if (irun>skip1 && irun<skip2) continue;
    dHeadChain->Add(Form("%s/run%i/minBiasHeadFile%i.root", anita3dataFolder.c_str(), irun, irun));
    dEventChain->Add(Form("%s/run%i/minBiasEventFile%i.root",             anita3dataFolder.c_str(), irun, irun));
  }


  dHeadChain->SetBranchAddress("header", &dataHeaderPtr);
  dEventChain->SetBranchAddress("event", &dataCalEvPtr);
  dHeadChain ->BuildIndex("realTime" );
  dEventChain->BuildIndex("eventNumber");

  // SIMULATION STUFF

  simDataFolder += iexp;

  TChain *simHeadChain  = new TChain("headTree"       );
  TChain *simEventChain = new TChain("eventTree"      );
  TChain *simGpsChain   = new TChain("adu5PatTree"    );
  TChain *simTruthChain = new TChain("truthAnitaTree" );
  

  simHeadChain ->Add(Form("%s/run%i/SimulatedAnitaHeadFile%i.root",   simDataFolder.c_str(), isimrun, isimrun ));
  simEventChain->Add(Form("%s/run%i/SimulatedAnitaEventFile%i.root",  simDataFolder.c_str(), isimrun, isimrun ));
  simGpsChain  ->Add(Form("%s/run%i/SimulatedAnitaGpsFile%i.root",    simDataFolder.c_str(), isimrun, isimrun ));
  simTruthChain->Add(Form("%s/run%i/SimulatedAnitaTruthFile%i.root",  simDataFolder.c_str(), isimrun, isimrun ));
  
  simHeadChain ->SetBranchAddress("header", &simHeaderPtr );
  simEventChain->SetBranchAddress("event",  &simEvPtr     );
  simGpsChain  ->SetBranchAddress("pat",    &simGpsPtr    );
  simTruthChain->SetBranchAddress("truth",  &truthEvPtr   );

  simHeadChain->SetBranchAddress("weight", &weight       );

  //  simHeadChain ->BuildIndex("realTime"   );
  simEventChain->BuildIndex("eventNumber");
  simGpsChain  ->BuildIndex("eventNumber");
  simTruthChain->BuildIndex("eventNumber");

  TFile *_fTruth = new TFile(Form("%s/run%i/SimulatedAnitaTruthFile%i.root",  simDataFolder.c_str(), isimrun, isimrun ), "read");
  TTree *configAnitaTreeInput      = (TTree*)_fTruth->Get("configAnitaTree");
  TTree *triggerSettingsTreeInput  = (TTree*)_fTruth->Get("triggerSettingsTree");
  //_fTruth->Close();

  // OUTPUT STUFF

  string run_no = Form("%i", isimrun);

  outputdir += iexp;

  outputdir += "/run"+run_no+"/";

  string outputAnitaFile =outputdir+"SimulatedAnitaEventFile"+run_no+".root";
  TFile *anitafileEvent = new TFile(outputAnitaFile.c_str(), "RECREATE");

  TTree *eventTree = new TTree("eventTree", "eventTree");
  eventTree->Branch("event",             &simEvPtr            );
  eventTree->Branch("run",               &isimrun,     "run/I"   );
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

  Int_t nEntries = simHeadChain->GetEntries();

  Int_t fNumPoints  = NUM_SAMP;
  Int_t fNumChans   = NUM_DIGITZED_CHANNELS;

  int tsurf, tchan;
  double simValue;
  
  AnitaGeomTool *fGeomTool = AnitaGeomTool::Instance();

  for (int ientry=0;ientry<nEntries;ientry++){

    simHeadChain->GetEntry(ientry);
    int newInd = dHeadChain->GetEntryNumberWithBestIndex(simHeaderPtr->realTime);
    if (newInd<0){
      cout << "Index not found, skipping event " << endl;
      continue;
    }
    dHeadChain->GetEntry(newInd);
    

    // // only use min bias triggers
    // while ((dataHeaderPtr->trigType & (1<<0))>0 ){
    //   newInd++;
    //   dHeadChain->GetEntry(newInd);
    // }

    dEventChain->GetEntryWithIndex(dataHeaderPtr->eventNumber);
    if (dataHeaderPtr->eventNumber != dataCalEvPtr->eventNumber){
      cout << "Something is wrong here " << dataHeaderPtr->eventNumber << " " << dataHeaderPtr->eventNumber << " " << dataCalEvPtr->eventNumber << " . Skipping this event."<< endl;
      continue;
    }

    dataEvPtr = new UsefulAnitaEvent(dataCalEvPtr, WaveCalType::kDefault);


    simHeadChain->GetEntry(newInd);       

    if (weight<0.000001) continue;
    
    cout << dataHeaderPtr->realTime << " " << simHeaderPtr->realTime << " " << (dataHeaderPtr->realTime -simHeaderPtr->realTime) <<  endl;
 
    simEventChain->GetEntryWithIndex(simHeaderPtr->eventNumber);
    cout << simHeaderPtr->eventNumber << " " << simEvPtr->eventNumber << endl;
    simTruthChain->GetEntryWithIndex(simHeaderPtr->eventNumber);
    
    for (int ichan = 0; ichan < fNumChans; ichan++){
      fGeomTool->getSurfChanFromChanIndex(ichan, tsurf, tchan);

      fNumPoints = dataEvPtr->fNumPoints[ichan];
      
      if (tchan!=8) { // if it's not the clock

	AnalysisWaveform wf(simEvPtr->fNumPoints[ichan], truthEvPtr->fSignalAtDigitizer[ichan], 1./2.6, truthEvPtr->fTimes[ichan][0]);
	wf.padFreq(10);
	// TGraph *gtemp = new TGraph (260, simEvPtr->fTimes[ichan], simEvPtr->fVolts[ichan]);
	// TGraph *graphUp = FFTtools::getInterpolatedGraph(gtemp, 1./(2.6*40));
      
	for (int ipoint = 0; ipoint < fNumPoints; ipoint++){
	  //	  double simValue = graphUp->Eval(dataEvPtr->fTimes[ichan][ipoint]);

	  simValue = wf.evalEven(dataEvPtr->fTimes[ichan][ipoint]);
	  simEvPtr->fVolts[ichan][ipoint] = simValue + dataEvPtr->fVolts[ichan][ipoint];
	  simEvPtr->fTimes[ichan][ipoint] =  dataEvPtr->fTimes[ichan][ipoint];
	  //    	cout <<  dataEvPtr->fTimes[ichan][ipoint] << " " << dataEvPtr->fVolts[ichan][ipoint] << endl;
	}
	// delete graphUp;
	// delete gtemp;
      } else { // if it's the clock
	for (int ipoint = 0; ipoint < fNumPoints; ipoint++){
	  simEvPtr->fVolts[ichan][ipoint] =  dataEvPtr->fVolts[ichan][ipoint];
	  simEvPtr->fTimes[ichan][ipoint] =  dataEvPtr->fTimes[ichan][ipoint];
	}
      }
      simEvPtr->fNumPoints[ichan] = fNumPoints;
    }
    
    simGpsChain->GetEntryWithIndex(simHeaderPtr->eventNumber);
    
    
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
  
  TTree *configAnitaTreeOutput = configAnitaTreeInput->CloneTree(-1,"fast");
  configAnitaTreeOutput->Write("configAnitaTree");
  truthAnitaTree->Write("truthAnitaTree");
  TTree *triggerSettingsTreeOutput = triggerSettingsTreeInput->CloneTree(-1, "fast");
  triggerSettingsTreeOutput->Write("triggerSettingsTree");
  
  anitafileTruth->Close();
  delete anitafileTruth;



}
