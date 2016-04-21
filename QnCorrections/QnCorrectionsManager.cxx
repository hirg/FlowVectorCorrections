/**************************************************************************************************
 *                                                                                                *
 * Package:       FlowVectorCorrections                                                           *
 * Authors:       Jaap Onderwaater, GSI, jacobus.onderwaater@cern.ch                              *
 *                Ilya Selyuzhenkov, GSI, ilya.selyuzhenkov@gmail.com                             *
 *                Contributors are mentioned in the code where appropriate.                       *
 * Development:   2012-2015                                                                       *
 *                                                                                                *
 * This file is part of FlowVectorCorrections, a software package that corrects Q-vector          *
 * measurements for effects of nonuniform detector acceptance. The corrections in this package    *
 * are based on publication:                                                                      *
 *                                                                                                *
 *  [1] "Effects of non-uniform acceptance in anisotropic flow measurements"                      *
 *  Ilya Selyuzhenkov and Sergei Voloshin                                                         *
 *  Phys. Rev. C 77, 034904 (2008)                                                                *
 *                                                                                                *
 * The procedure proposed in [1] is extended with the following steps:                            *
 * (*) alignment correction between subevents                                                     *
 * (*) possibility to extract the twist and rescaling corrections                                 *
 *      for the case of three detector subevents                                                  *
 *      (currently limited to the case of two “hit-only” and one “tracking” detectors)            *
 * (*) (optional) channel equalization                                                            *
 * (*) flow vector width equalization                                                             *
 *                                                                                                *
 * FlowVectorCorrections is distributed under the terms of the GNU General Public License (GPL)   *
 * (https://en.wikipedia.org/wiki/GNU_General_Public_License)                                     *
 * either version 3 of the License, or (at your option) any later version.                        *
 *                                                                                                *
 **************************************************************************************************/
 
 
  
 

#include "QnCorrectionsConstants.h"
#include "QnCorrectionsManager.h"
#include "QnCorrectionsConfiguration.h"
#include "QnCorrectionsDataVector.h"
#include "QnCorrectionsHistograms.h"
#include "QnCorrectionsSteps.h"
#include <TMath.h>
#include <THashList.h>
#include <THashList.h>
#include <TClonesArray.h>
#include <TRandom3.h>
//#include <TArrayS.h>
#include <iostream>
#include <iomanip>
#include <fstream>
using namespace std;

ClassImp(QnCorrectionsManager)


//_______________________________________________________________________________
QnCorrectionsManager::QnCorrectionsManager() :
  TObject(),
  fDataVectors(),
  fConfDataVectors(),
  fQnCorrectionsConfigurations(),
  fCorrectedQvectors(),
  fLastStep(),
  fInputHistograms(),
  fOutputHistograms(),
  //fDetectorIdMap(0x0),
  fNumberOfQnConfigurations(0),
  fNumberOfDetectors(0),
  fNumberOfQnConfigurationsForDetector(),
  fNdetectors(0),
  fIndex(),
  fCorrectionStep(0),
  fPassesRequired(0),
  fCalibrateByRun(kFALSE),
  fListInputHistogramsQnCorrections(0x0),
  fDataContainer(),
  fLabel(""),
  fProcessedFirstEvent(kFALSE),
  fQvecOutputList(),
  fSetFillTreeQnVectors(kFALSE),
  fSetFillHistogramsQA(kFALSE),
  fSetFillHistogramsQnCorrections(kFALSE),
  fListQnVectors(0x0),
  fTreeQnVectors(0x0),
  fListOutputHistogramsQnCorrections(0x0),
  fListHistogramsQA(0x0),
  fOutputHistogramsQnCorrectionsFile(0x0),
  fHistogramsQAFile(0x0),
  fTreeQnVectorsFile(0x0),
  fRunList("")
{
  //
  // default constructor
  //

  for(Int_t i=0; i<QnCorrectionsConstants::nDataVectors; ++i) fNumberOfQnConfigurationsForDetector[i]=0;
  for(Int_t i=0; i<QnCorrectionsConstants::nDataVectors; ++i) fDataVectors[i] = new TClonesArray("QnCorrectionsDataVector", 100000);
  for(Int_t i=0; i<QnCorrectionsConstants::nQnConfigurations; ++i) fConfDataVectors[i] = new TClonesArray("QnCorrectionsDataVector", 100000);
  for(Int_t i=0; i<QnCorrectionsConstants::nDataVectors; ++i) fQnCorrectionsConfigurations[i] = new TClonesArray("QnCorrectionsConfiguration", QnCorrectionsConstants::nQnConfigurations);


  for(Int_t iconf=0; iconf<QnCorrectionsConstants::nQnConfigurations; ++iconf){ 
    fInputHistograms[iconf]=0x0;
    fOutputHistograms[iconf]=0x0;
    fLastStep[iconf]=0;
    fIndex[iconf][0]=0x0;
    fIndex[iconf][1]=0x0;
  }

  for(Int_t i=0; i<QnCorrectionsConstants::nDataContainerVariables; i++) fDataContainer[i]=-999.;

  fListOutputHistogramsQnCorrections   = new THashList();
  fListOutputHistogramsQnCorrections->SetName("CalibrationHistograms");
  fListHistogramsQA = new THashList();
  fListHistogramsQA->SetName("CalibrationHistogramsQA");

  fListOutputHistogramsQnCorrections->SetOwner();
  fListHistogramsQA->SetOwner();

  fListQnVectors = new THashList();

}



//_______________________________________________________________________________
QnCorrectionsManager::QnCorrectionsManager(const QnCorrectionsManager &c) :
  TObject(),
  fDetectorIdMap(c.fDetectorIdMap),
  fNumberOfQnConfigurations(c.fNumberOfQnConfigurations),
  fNumberOfDetectors(c.fNumberOfDetectors),
  fNdetectors(c.fNdetectors),
  fCorrectionStep(c.fCorrectionStep),
  fPassesRequired(c.fPassesRequired),
  fCalibrateByRun(c.fCalibrateByRun),
  fListInputHistogramsQnCorrections(c.fListInputHistogramsQnCorrections),
  fLabel(c.fLabel),
  fProcessedFirstEvent(c.fProcessedFirstEvent),
  fSetFillTreeQnVectors(c.fSetFillTreeQnVectors),
  fSetFillHistogramsQA(c.fSetFillHistogramsQA),
  fSetFillHistogramsQnCorrections(c.fSetFillHistogramsQnCorrections),
  fListQnVectors(c.fListQnVectors),
  fTreeQnVectors(c.fTreeQnVectors),
  fListOutputHistogramsQnCorrections(c.fListOutputHistogramsQnCorrections),
  fListHistogramsQA(c.fListHistogramsQA),
  fOutputHistogramsQnCorrectionsFile(c.fOutputHistogramsQnCorrectionsFile),
  fHistogramsQAFile(c.fHistogramsQAFile),
  fTreeQnVectorsFile(c.fTreeQnVectorsFile),
  fRunList(c.fRunList)
{
  //
  // copy constructor
  //

  for(Int_t i=0; i<QnCorrectionsConstants::nDataVectors; ++i) fNumberOfQnConfigurationsForDetector[i]=c.fNumberOfQnConfigurationsForDetector[i];
  for(Int_t i=0; i<QnCorrectionsConstants::nDataVectors; ++i) fDataVectors[i] = c.fDataVectors[i];
  for(Int_t i=0; i<QnCorrectionsConstants::nQnConfigurations; ++i) fConfDataVectors[i] = c.fConfDataVectors[i] ;
  for(Int_t i=0; i<QnCorrectionsConstants::nDataVectors; ++i) fQnCorrectionsConfigurations[i] = c.fQnCorrectionsConfigurations[i];


  for(Int_t iconf=0; iconf<QnCorrectionsConstants::nQnConfigurations; ++iconf){ 
    fInputHistograms[iconf]=c.fInputHistograms[iconf];
    fOutputHistograms[iconf]=c.fOutputHistograms[iconf];
    fLastStep[iconf]=c.fLastStep[iconf];
    fIndex[iconf][0]=c.fIndex[iconf][0];
    fIndex[iconf][1]=c.fIndex[iconf][1];
  }

  for(Int_t i=0; i<QnCorrectionsConstants::nDataContainerVariables; i++){
    fDataContainer[i]= c.fDataContainer[i];
  }

}




//_______________________________________________________________________________
QnCorrectionsManager & QnCorrectionsManager::operator=(const QnCorrectionsManager &c) {
  if (this == &c) return *this;
  else {
    fDetectorIdMap=c.fDetectorIdMap;
    fNumberOfQnConfigurations=c.fNumberOfQnConfigurations;
    fNumberOfDetectors=c.fNumberOfDetectors;
    fNdetectors=c.fNdetectors;
    fCorrectionStep=c.fCorrectionStep;
    fPassesRequired=c.fPassesRequired;
    fCalibrateByRun=c.fCalibrateByRun;
    fListInputHistogramsQnCorrections=c.fListInputHistogramsQnCorrections;
    fLabel=c.fLabel;
    fProcessedFirstEvent=c.fProcessedFirstEvent;
    fSetFillTreeQnVectors=c.fSetFillTreeQnVectors;
    fSetFillHistogramsQA=c.fSetFillHistogramsQA;
    fSetFillHistogramsQnCorrections=c.fSetFillHistogramsQnCorrections;
    fListQnVectors=c.fListQnVectors;
    fTreeQnVectors=c.fTreeQnVectors;
    fListOutputHistogramsQnCorrections=c.fListOutputHistogramsQnCorrections;
    fListHistogramsQA=c.fListHistogramsQA;
    fOutputHistogramsQnCorrectionsFile=c.fOutputHistogramsQnCorrectionsFile;
    fHistogramsQAFile=c.fHistogramsQAFile;
    fTreeQnVectorsFile=c.fTreeQnVectorsFile;
    fRunList=c.fRunList;

    for(Int_t i=0; i<QnCorrectionsConstants::nDataVectors; ++i) fNumberOfQnConfigurationsForDetector[i]=c.fNumberOfQnConfigurationsForDetector[i];
    for(Int_t i=0; i<QnCorrectionsConstants::nDataVectors; ++i) fDataVectors[i] = c.fDataVectors[i];
    for(Int_t i=0; i<QnCorrectionsConstants::nQnConfigurations; ++i) fConfDataVectors[i] = c.fConfDataVectors[i] ;
    for(Int_t i=0; i<QnCorrectionsConstants::nDataVectors; ++i) fQnCorrectionsConfigurations[i] = c.fQnCorrectionsConfigurations[i];


    for(Int_t iconf=0; iconf<QnCorrectionsConstants::nQnConfigurations; ++iconf){ 
      fInputHistograms[iconf]=c.fInputHistograms[iconf];
      fOutputHistograms[iconf]=c.fOutputHistograms[iconf];
      fLastStep[iconf]=c.fLastStep[iconf];
      fIndex[iconf][0]=c.fIndex[iconf][0];
      fIndex[iconf][1]=c.fIndex[iconf][1];
    }

    for(Int_t i=0; i<QnCorrectionsConstants::nDataContainerVariables; i++){
      fDataContainer[i]= c.fDataContainer[i];
    }

      return *this;
  }
    
}










//____________________________________________________________________________
QnCorrectionsManager::~QnCorrectionsManager()
{
  //
  // De-Constructor
  //
  ClearEvent();
  for(Int_t i=0; i<fNumberOfDetectors; ++i) if(fDataVectors[i]) fDataVectors[i]->Clear("C");
  for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; ++iconf) {
    for(Int_t istep=0; istep<QnCorrectionsConstants::nCorrectionSteps; istep++) if(fCorrectedQvectors[iconf][istep]) fCorrectedQvectors[iconf][istep]->Clear("C");
    if(fConfDataVectors[iconf]) fConfDataVectors[iconf]->Clear("C");
  }
  for(Int_t i=0; i<QnCorrectionsConstants::nDataVectors; ++i) if(fQnCorrectionsConfigurations[i]) fQnCorrectionsConfigurations[i]->Clear("C");
}


//_____________________________________________________________________________
void QnCorrectionsManager::ClearEvent() {
  //
  // clear the event
  //
  for(Int_t i=0; i<fNumberOfDetectors; ++i) if(fDataVectors[i]) fDataVectors[i]->Clear("C");
  for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; ++iconf) {
    for(Int_t istep=0; istep<QnCorrectionsConstants::nCorrectionSteps; istep++) if(fCorrectedQvectors[iconf][istep]) if(fCorrectedQvectors[iconf][istep]->At(0)) (static_cast<QnCorrectionsQnVector*>(fCorrectedQvectors[iconf][istep]->At(0)))->Reset();
    if(fConfDataVectors[iconf]) fConfDataVectors[iconf]->Clear("C");
  }
  for(Int_t i=0; i<QnCorrectionsConstants::nDataContainerVariables; i++) fDataContainer[i]=-999.;
  fListQnVectors->Clear();
}

//_____________________________________________________________________________
//Bool_t QnCorrectionsManager::SetCalibrationFile(TFile* inputFile){
//    if(inputFile){
//      if(inputFile->GetListOfKeys()->GetEntries()>0){
//        //fListInputHistogramsQnCorrections = (THashList*)((TKey*)inputFile->GetListOfKeys()->At(0))->ReadObj(); return (fListInputHistogramsQnCorrections ? kTRUE : kFALSE);} //TODO make "CalibrationHistograms" name customizable
//        THashList* list = (THashList*)((TKey*)inputFile->GetListOfKeys()->At(0))->ReadObj();
//        if(list){
//          for(Int_t i=0; i<list->GetEntries(); i++){
//            fListInputHistogramsQnCorrections->Add(new TObject(*(list->At(i)));
//          }
//        }
//      
//      return (fListInputHistogramsQnCorrections ? kTRUE : kFALSE);} //TODO make "CalibrationHistograms" name customizable
//    }
//}

//_____________________________________________________________________________
void QnCorrectionsManager::Process() {

  if(!fProcessedFirstEvent){
    //Initialize();
    InitializeCalibrationHistograms();
    PrintFrameworkInformation();
  }

  ApplyQnCorrections();

  FillHistograms();

  if(fSetFillTreeQnVectors){
    for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
      fQvecOutputList[iconf]=((QnCorrectionsQnVector*) CorrectedQnVector(iconf)->At(0));
    }
    fTreeQnVectors->Fill();
  }

//   ClearEvent();

  fProcessedFirstEvent=kTRUE;

}



////_____________________________________________________________________________
//void QnCorrectionsManager::ApplyQnCorrections() {
//
//
//  QnCorrectionsConfiguration* QnConf = 0x0;
//
//  for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
//    QnCorrectionsConfiguration* QnConf = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf);
//    if(!QnConf) continue;
//
//    CallStepEqualizeDataVectorWeights(QnConf);
//
//    BuildQnVectors(QnConf, kFALSE);   // Get Q vectors with raw detector information
//
//    BuildQnVectors(QnConf, kTRUE);    // Get Q vectors with equalized detector information
//
//    CallStepRecenterQnVector(QnConf);
//
//
//  }
//
//}



//_____________________________________________________________________________
void QnCorrectionsManager::ApplyQnCorrections() {


  QnCorrectionsConfiguration* QnConf = 0x0;


  for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
    QnConf = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf);
    if(!QnConf) continue;


    BuildQnVectors(QnConf, kFALSE);
    if(QnConf->IsApplyCorrection(QnCorrectionsConstants::kDataVectorEqualization)) {CallStepEqualizeDataVectorWeights(QnConf);BuildQnVectors(QnConf, kTRUE);}
    
    if(QnConf->IsApplyCorrection(QnCorrectionsConstants::kRecentering))             CallStepRecenterQnVector(QnConf);
    
    if(QnConf->IsApplyCorrection(QnCorrectionsConstants::kAlignment))               CallStepRotateQvector(QnConf);

    if(QnConf->IsApplyCorrection(QnCorrectionsConstants::kTwist))                   CallStepTwistAndRescaleQnVector(QnConf);



  }
}




//_____________________________________________________________________
void QnCorrectionsManager::FillHistograms() {

  QnCorrectionsConfiguration* QnConf;
  for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
    QnConf = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf);

    if(fSetFillHistogramsQnCorrections){
      if(QnConf->IsFillHistogram(QnCorrectionsConstants::kDataVectorEqualization)||QnConf->IsApplyCorrection(QnCorrectionsConstants::kDataVectorEqualization)){
        FillHistogramsWeights(QnConf);
        //cout<<"go into fill "<<QnConf->QnConfigurationName()<<"  "<<QnConf->IsFillHistogram(QnCorrectionsConstants::kRecentering)<<endl;
        if(QnConf->IsFillHistogram(QnCorrectionsConstants::kRecentering))    {FillHistogramsMeanQ(QnConf, (Int_t) QnCorrectionsConstants::kDataVectorEqualization);}
      }
      if(QnConf->IsFillHistogram(QnCorrectionsConstants::kRecentering)&&!QnConf->IsRequestedCorrection(QnCorrectionsConstants::kDataVectorEqualization))   
                                                                            {FillHistogramsMeanQ(QnConf, (Int_t) QnCorrectionsConstants::kPass0);}
      if(QnConf->IsFillHistogram(QnCorrectionsConstants::kAlignment))       {FillHistogramsQnAlignment(QnConf);} 
      if(QnConf->IsFillHistogram(QnCorrectionsConstants::kTwist))           {FillHistogramsU2n(QnConf); FillHistogramsQnCorrelations(QnConf);}
    }

    // Fill QA correction histograms

    if(fSetFillHistogramsQA){
                                                                                              //FillHistogramsQnCorrelationsQA(QnConf);
                                                                                           FillHistogramsMeanQ_QA(QnConf);
       if(QnConf->IsApplyCorrection(QnCorrectionsConstants::kAlignment))                   FillHistogramsQnAlignmentQA(QnConf); 
       if((QnConf->IsApplyCorrection(QnCorrectionsConstants::kTwist)||QnConf->IsApplyCorrection(QnCorrectionsConstants::kRescaling))&&QnConf->GetTwistAndRescalingMethod()==2)
         FillHistogramsQnCorrelationsQA(QnConf);
                                                                                              
    }

  }

}

//_____________________________________________________________________________
void QnCorrectionsManager::WriteQnVectorsToList() {


  QnCorrectionsConfiguration* QnConf=0x0;
  for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
    QnConf = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf);
    if(!QnConf) continue;
    TClonesArray* qvec = CorrectedQnVector(iconf);
    if(qvec) {
      qvec->SetName(QnConf->QnConfigurationName());
      fListQnVectors->Add(qvec);
    }
    for(Int_t istep=0; istep<QnCorrectionsConstants::nCorrectionSteps; istep++) {
      TClonesArray* qvecAll = CorrectedQnVector(iconf,istep);
      if(qvecAll) {
        qvecAll->SetName(Form("%s_%d",QnConf->QnConfigurationName().Data(),istep));
        fListQnVectors->Add(qvecAll);
      }

    }
  }
}

//_____________________________________________________________________________
void QnCorrectionsManager::Initialize() {
  //
  // Initialize before looping over data
  // Sets the correlation detector indices (this prevents having to do many string operations), creates the necessary histograms, free space reserved for unused detectors
  //



  QnCorrectionsConfiguration* QnConf = 0x0;
  QnCorrectionsConfiguration* QnConf2 = 0x0;
  TString detStrCor1 = "";
  TString detStrCor2 = "";
  TString detStrCor3 = "";
  for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
    QnConf = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf);
    if(!QnConf) continue;
    detStrCor1 = QnConf->QnConfigurationCorrelationName(0);
    detStrCor2 = QnConf->QnConfigurationCorrelationName(1);
    detStrCor3 = QnConf->QnConfigurationCorrelationName(2);
    QnConf2 = 0x0;
    for(Int_t iconf2=0; iconf2<fNumberOfQnConfigurations; iconf2++){
      QnConf2 = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf2);
      if(!QnConf2) continue;
      if(detStrCor1.EqualTo(QnConf2->QnConfigurationName())) QnConf->SetQnConfigurationCorrelationIndex(0,QnConf2->GlobalIndex());
      if(detStrCor2.EqualTo(QnConf2->QnConfigurationName())) QnConf->SetQnConfigurationCorrelationIndex(1,QnConf2->GlobalIndex());
      if(detStrCor3.EqualTo(QnConf2->QnConfigurationName())) QnConf->SetQnConfigurationCorrelationIndex(2,QnConf2->GlobalIndex());
    }
  }

  for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
    QnConf = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf);
    if(!QnConf) continue;
    fInputHistograms[iconf]  = new QnCorrectionsHistograms();
    fOutputHistograms[iconf] = new QnCorrectionsHistograms();
    fOutputHistograms[iconf]->CreateCalibrationHistograms(QnConf);
    for(Int_t istep=0; istep<QnCorrectionsConstants::kNcorrectionSteps; istep++)
      {fCorrectedQvectors[iconf][istep] = new TClonesArray("QnCorrectionsQnVector", 10);
       TClonesArray& arr = *(fCorrectedQvectors[iconf][istep]);
       if(istep>1) continue;
       QnCorrectionsQnVector* q = new(arr[0]) QnCorrectionsQnVector();
       if(QnConf->GetTwistAndRescalingMethod()==0) q->SetMaximumHarmonic(QnConf->MaximumHarmonic()*2);
       else q->SetMaximumHarmonic(QnConf->MaximumHarmonic()*2);
      }
  }

  fNumberOfDetectors=fDetectorIdMap.size();
  



  if(fSetFillTreeQnVectors){

    fTreeQnVectors = new TTree("Qvectors","Qvector values");
    fTreeQnVectors->SetDirectory(0);

    for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
      QnConf = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf);
      if(!QnConf) continue;
      //fQvecOutputList[QnConf->GlobalIndex()]->At(0)  = new QnCorrectionsQnVector(); //CorrectedQnVector(iconf,istep);
      //fQvecOutputList[QnConf->GlobalIndex()]->SetName(QnConf->QnConfigurationName());
      //fTreeQnVectors->Branch(Form("%s_%d",QnConf->QnConfigurationName().Data(),istep),&fQvecOutputList[QnConf->GlobalIndex()],256000,1);
      fQvecOutputList[iconf]=new QnCorrectionsQnVector();
      fTreeQnVectors->Branch(QnConf->QnConfigurationName(),&fQvecOutputList[iconf],256000,1);
    }

  }

  //InitializeCalibrationHistograms();
}


//_______________________________________________________________________________
void QnCorrectionsManager::SetFileRunLabels(TString path){
  //
  // Add runlist to the framework
  //

  ifstream inBuf;
  inBuf.open(path);
  Int_t index = 0;
  while(inBuf.good()) {
    Char_t str[512];
    inBuf.getline(str,512,'\n');
    AddRunLabel(str);
  }

}



//_______________________________________________________________________________
void QnCorrectionsManager::AddDataVector( Int_t detectorId, Double_t phi, Double_t weight, Int_t dataVectorId) {
  //
  // Add datavector to TClonesArray of DataVectors and set its bits
  //

  detectorId=fDetectorIdMap[detectorId]-1;
  Bool_t useForQn;

  QnCorrectionsConfiguration* QnConf;
  for(Int_t iconf=0; iconf<fNumberOfQnConfigurationsForDetector[detectorId]; iconf++){
    QnConf=static_cast<QnCorrectionsConfiguration*>(fQnCorrectionsConfigurations[detectorId]->At(iconf));
    if(!QnConf) continue;

    useForQn=kTRUE;

    if(QnConf->Cuts())         useForQn=QnConf->PassCuts(fDataContainer); 
    if(QnConf->ChannelList())  if(useForQn) useForQn=QnConf->UseChannel(dataVectorId);

    if(useForQn) {
      TClonesArray& dataVectors = *(fConfDataVectors[QnConf->GlobalIndex()]);
      QnCorrectionsDataVector *dataVector=new(dataVectors[dataVectors.GetEntriesFast()]) QnCorrectionsDataVector();
      dataVector->SetPhi(phi);
      dataVector->SetWeight(weight);
      dataVector->SetId(dataVectorId);
    }
  } 
}







//_________________________________________________________________
Bool_t QnCorrectionsManager::BuildQnVectors(QnCorrectionsConfiguration* QnConf, Bool_t useEqualizedWeights){
  //
  // Construct the event plane for a specified detector
  //

  //if(useEqualizedWeights){ 
  //  if(QnConf->GetCorrectionStep(fCorrectionStep)<1) return kFALSE;
  //  if(!QnConf->doChannelEqualization()) return kFALSE;
  //}
    
  QnCorrectionsQnVector* QnVector = static_cast<QnCorrectionsQnVector*>(fCorrectedQvectors[QnConf->GlobalIndex()][(Int_t) useEqualizedWeights]->At(0));
  fLastStep[QnConf->GlobalIndex()]=(Int_t) useEqualizedWeights;


  if(!useEqualizedWeights)   QnCorrectionsDataVector::FillQvector(fConfDataVectors[QnConf->GlobalIndex()], QnVector);
  else                       QnCorrectionsDataVector::FillQvector(fConfDataVectors[QnConf->GlobalIndex()], QnVector, QnConf->GetDataVectorEqualizationMethod());

  if(QnVector->N()==0) {               // If detector is empty
    for(Int_t ih=QnConf->MinimumHarmonic(); ih<=QnConf->MaximumHarmonic(); ++ih) QnVector->SetQnVectorStatus(ih, QnCorrectionsConstants::kUndefined);
    return kTRUE;
  }
  else{
    Int_t maxHar = QnConf->MaximumHarmonic();
    if(QnConf->GetTwistAndRescalingMethod()==0) maxHar*=2;
    for(Int_t ih=QnConf->MinimumHarmonic(); ih<=maxHar; ++ih){
      if(!useEqualizedWeights)   QnVector->SetQnVectorStatus(ih, QnCorrectionsConstants::kPass0);
      else                       QnVector->SetQnVectorStatus(ih, QnCorrectionsConstants::kDataVectorEqualization);
    }
  }


  if(QnConf->GetQnNormalizationMethod()==0)        {QnVector->SetQoverSquareRootOfM();QnVector->SetQvectorNormalization(0);}
  else if(QnConf->GetQnNormalizationMethod()==1)   {QnVector->SetQoverM();QnVector->SetQvectorNormalization(1);}
  else if(QnConf->GetQnNormalizationMethod()==2)   {QnVector->Normalize();QnVector->SetQvectorNormalization(2);}

  return kTRUE;


}

//_____________________________________________________________________
Bool_t QnCorrectionsManager::CallStepEqualizeDataVectorWeights(QnCorrectionsConfiguration* QnConf) {
  //
  // Calibrate channel multiplicities
  //

  //if(!QnConf->doChannelEqualization()||QnConf->GetCorrectionStep(fCorrectionStep)<1) return kFALSE;

  Double_t fillValues[20];

  const Int_t* var = QnConf->EqualizationBinning()->Var();
  const Int_t  dim = QnConf->EqualizationBinning()->Dim();

  for(Int_t iav=0; iav<(dim-1); iav++) fillValues[iav] = fDataContainer[var[iav]];
  QnCorrectionsSteps::CalibrateDataVector(GetConfDataVectors(QnConf->GlobalIndex()), QnConf,  fInputHistograms[QnConf->GlobalIndex()], fillValues);

  return kTRUE;
}


//_____________________________________________________________________
Bool_t QnCorrectionsManager::CallStepRecenterQnVector(QnCorrectionsConfiguration* QnConf) {

  //
  // Recenter the detector event plane
  //


  Int_t bin=-1;
  Double_t fillValues[20];

  Int_t useStep=0;


  //if( QnConf->GetCorrectionStep(fCorrectionStep)<2||!QnConf->doRecentering()) return kFALSE;

  useStep=0;
  if(QnConf->IsApplyCorrection(QnCorrectionsConstants::kDataVectorEqualization)) useStep=1;

  QnCorrectionsAxes* EPbinning =  QnConf->GetRecenteringAxes();


  const Int_t* var = EPbinning->Var();
  const Int_t  dim = EPbinning->Dim();

  for(Int_t iav=0; iav<dim; iav++) fillValues[iav] = fDataContainer[var[iav]]; 

  bin=fInputHistograms[QnConf->GlobalIndex()]->CalibrationHistogramE(useStep)->GetBin(fillValues);

  QnCorrectionsQnVector* QvectorIn=0x0;
  if(useStep==0) QvectorIn=static_cast<QnCorrectionsQnVector*>(fCorrectedQvectors[QnConf->GlobalIndex()][0]->At(0));
  else           QvectorIn=static_cast<QnCorrectionsQnVector*>(fCorrectedQvectors[QnConf->GlobalIndex()][1]->At(0));

  TClonesArray& arr = *(fCorrectedQvectors[QnConf->GlobalIndex()][QnCorrectionsConstants::kRecentering]);
  QnCorrectionsQnVector* QvectorOut = new(arr[0]) QnCorrectionsQnVector(*QvectorIn);

  //QnCorrectionsQnVector* QvectorOut=static_cast<QnCorrectionsQnVector*>(fCorrectedQvectors[QnConf->GlobalIndex()][2]->At(0));
  fLastStep[QnConf->GlobalIndex()]=QnCorrectionsConstants::kRecentering;

  QnCorrectionsSteps::RecenterQvec( QvectorIn, QvectorOut, fInputHistograms[QnConf->GlobalIndex()], bin, useStep, QnConf->MinimumHarmonic(), QnConf->MaximumHarmonic());


  return kTRUE;

}






//_____________________________________________________________________
void QnCorrectionsManager::CallStepRotateQvector(QnCorrectionsConfiguration* QnConf) {

  //
  // Align Q-vectors
  //

  Double_t fillValues[20];

  const Int_t* var = QnConf->GetAlignmentAxes()->Var();
  const Int_t  dim = QnConf->GetAlignmentAxes()->Dim();
  for(Int_t iav=0; iav<dim; iav++) fillValues[iav] = fDataContainer[var[iav]];

  Int_t iconf=QnConf->GlobalIndex();
  Int_t bin = fInputHistograms[iconf]->GetRotationHistogramE(0)->GetBin(fillValues);

  QnCorrectionsQnVector* QvectorIn= static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(iconf)->At(0));
  TClonesArray &arr = *(fCorrectedQvectors[QnConf->GlobalIndex()][QnCorrectionsConstants::kAlignment]);
  QnCorrectionsQnVector* QvectorRotated = new(arr[0]) QnCorrectionsQnVector(*QvectorIn);

  QnCorrectionsSteps::RecenterQvec( QvectorIn, QvectorRotated, fInputHistograms[iconf], bin, QnConf->MinimumHarmonic(), QnConf->MaximumHarmonic(), QnConf->AlignmentHarmonic());

  fLastStep[QnConf->GlobalIndex()]=QnCorrectionsConstants::kAlignment;
  return;
}



//_____________________________________________________________________
void QnCorrectionsManager::FillHistogramsWeights(QnCorrectionsConfiguration* QnConf) {


  Double_t fillValues[20];


  TClonesArray* dataVectorArray = GetConfDataVectors(QnConf->GlobalIndex());

  const Int_t* var = QnConf->EqualizationBinning()->Var();
  const Int_t  dim = QnConf->EqualizationBinning()->Dim();
  for(Int_t iav=0; iav<(dim-1); iav++) fillValues[iav] = fDataContainer[var[iav]];

  Int_t globalIndex=QnConf->GlobalIndex();
  QnCorrectionsDataVector* dataVector;

  for(Int_t idata=0; idata<dataVectorArray->GetEntriesFast(); idata++){
    dataVector = static_cast<QnCorrectionsDataVector*>(dataVectorArray->At(idata));

    //TIter nextEntry(dataVectorArray);
    //while((dataVector=static_cast<QnCorrectionsDataVector*>(nextEntry()))) {
    //if(!dataVector) continue;

    //if(dataVector->Weight()<1E-6) continue;
    //if(!dataVector->CheckEventPlaneDetector(localIndex)) continue;

    fillValues[dim-1] = dataVector->Id();
    //bin=fOutputHistograms[globalIndex]->EqualizationHistogramM(0)->GetBin(fillValues);

    fOutputHistograms[globalIndex]->EqualizationHistogramM(0)->Fill(fillValues,dataVector->Weight());
    fOutputHistograms[globalIndex]->EqualizationHistogramE(0)->Fill(fillValues,1.);
    if(!fSetFillHistogramsQA) continue;
    if(!QnConf->IsApplyCorrection(QnCorrectionsConstants::kDataVectorEqualization)) continue;
    fOutputHistograms[globalIndex]->EqualizationHistogramM(1)->Fill(fillValues,dataVector->Weight(0));
    fOutputHistograms[globalIndex]->EqualizationHistogramE(1)->Fill(fillValues,1.);
    fOutputHistograms[globalIndex]->EqualizationHistogramM(2)->Fill(fillValues,dataVector->Weight(1));
    fOutputHistograms[globalIndex]->EqualizationHistogramE(2)->Fill(fillValues,1.);

  }
}

//______________________________________
void QnCorrectionsManager::FillHistogramsQnAlignment(QnCorrectionsConfiguration* QnConf){

  Int_t iconf=QnConf->GlobalIndex();

  QnCorrectionsQnVector* Qvecs[2];
  QnCorrectionsConfiguration* QnConfRef=GetQnConfiguration(QnConf->QnConfigurationCorrelationIndex(2));

  Double_t fillValues[20];

  const Int_t* var = QnConf->GetAlignmentAxes()->Var();
  const Int_t  dim = QnConf->GetAlignmentAxes()->Dim();
  for(Int_t iav=0; iav<dim; iav++) fillValues[iav] = fDataContainer[var[iav]];

  if(QnConf->IsApplyCorrection(QnCorrectionsConstants::kPass0))                  Qvecs[0] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(iconf,QnCorrectionsConstants::kPass0)->At(0));
  if(QnConf->IsApplyCorrection(QnCorrectionsConstants::kDataVectorEqualization)) Qvecs[0] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(iconf,QnCorrectionsConstants::kDataVectorEqualization)->At(0));
  if(QnConf->IsApplyCorrection(QnCorrectionsConstants::kRecentering))            Qvecs[0] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(iconf,QnCorrectionsConstants::kRecentering)->At(0));
  
  Qvecs[1] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(QnConf->QnConfigurationCorrelationIndex(2))->At(0));

  //cout<<QnConf->QnConfigurationName()<<"  "<<QnConf->QnConfigurationCorrelationIndex(2)<<"  "<<GetQnConfiguration(QnConf->QnConfigurationCorrelationIndex(2))->QnConfigurationName()<<endl;

  THnF* entr = fOutputHistograms[iconf]->GetRotationHistogramE(0);
  Int_t bin=entr->GetBin(fillValues);

  Double_t qx1,qx2,qy1,qy2;

  Int_t har=QnConf->AlignmentHarmonic();
  if(Qvecs[0]->CheckQnVectorStatus(har,QnCorrectionsConstants::kUndefined)||Qvecs[1]->CheckQnVectorStatus(har,QnCorrectionsConstants::kUndefined)) return;
  qx1=Qvecs[0]->Qx(har); qx2=Qvecs[1]->Qx(har); qy1=Qvecs[0]->Qy(har); qy2=Qvecs[1]->Qy(har);
  fOutputHistograms[iconf]->GetRotationHistogram(0,0)->AddBinContent(bin,qx1*qx2);
  fOutputHistograms[iconf]->GetRotationHistogram(0,1)->AddBinContent(bin,qy1*qy2);
  fOutputHistograms[iconf]->GetRotationHistogram(0,2)->AddBinContent(bin,qx1*qy2);
  fOutputHistograms[iconf]->GetRotationHistogram(0,3)->AddBinContent(bin,qy1*qx2);
  fOutputHistograms[iconf]->GetRotationHistogram(0,0)->AddBinError2(bin,qx1*qx2*qx1*qx2);
  fOutputHistograms[iconf]->GetRotationHistogram(0,1)->AddBinError2(bin,qy1*qy2*qy1*qy2);
  fOutputHistograms[iconf]->GetRotationHistogram(0,2)->AddBinError2(bin,qx1*qy2*qx1*qy2);
  fOutputHistograms[iconf]->GetRotationHistogram(0,3)->AddBinError2(bin,qy1*qx2*qy1*qx2);
  entr->AddBinContent(bin,1.0);
  Int_t entries = entr->GetEntries();
  entr->SetEntries(++entries);

}


//______________________________________
void QnCorrectionsManager::FillHistogramsQnAlignmentQA(QnCorrectionsConfiguration* QnConf){

  Int_t iconf=QnConf->GlobalIndex();

  QnCorrectionsQnVector* Qvecs[2];
  QnCorrectionsConfiguration* QnConfRef=GetQnConfiguration(QnConf->QnConfigurationCorrelationIndex(2));

  Double_t fillValues[20];

  const Int_t* var = QnConf->GetAlignmentAxes()->Var();
  const Int_t  dim = QnConf->GetAlignmentAxes()->Dim();
  for(Int_t iav=0; iav<dim; iav++) fillValues[iav] = fDataContainer[var[iav]];


  Qvecs[0] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(iconf,QnCorrectionsConstants::kAlignment)->At(0));
  Qvecs[1] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(QnConf->QnConfigurationCorrelationIndex(2))->At(0));


  //cout<<QnConf->QnConfigurationName()<<"  "<<QnConf->QnConfigurationCorrelationIndex(2)<<"  "<<GetQnConfiguration(QnConf->QnConfigurationCorrelationIndex(2))->QnConfigurationName()<<endl;

  Int_t bin=fOutputHistograms[iconf]->GetRotationHistogramE(1)->GetBin(fillValues);

  Double_t qx1,qx2,qy1,qy2;

  for(Int_t ih=QnConf->MinimumHarmonic(); ih<=QnConf->MaximumHarmonic(); ++ih){ 
    if(ih>QnConfRef->MaximumHarmonic()) continue;
    if(Qvecs[0]->CheckQnVectorStatus(ih,QnCorrectionsConstants::kUndefined)||Qvecs[1]->CheckQnVectorStatus(ih,QnCorrectionsConstants::kUndefined)) continue;
    qx1=Qvecs[0]->Qx(ih); qx2=Qvecs[1]->Qx(ih); qy1=Qvecs[0]->Qy(ih); qy2=Qvecs[1]->Qy(ih);
    fOutputHistograms[iconf]->GetRotationHistogram(1,0)->AddBinContent(bin,qx1*qx2);
    fOutputHistograms[iconf]->GetRotationHistogram(1,1)->AddBinContent(bin,qy1*qy2);
    fOutputHistograms[iconf]->GetRotationHistogram(1,2)->AddBinContent(bin,qx1*qy2);
    fOutputHistograms[iconf]->GetRotationHistogram(1,3)->AddBinContent(bin,qy1*qx2);
    fOutputHistograms[iconf]->GetRotationHistogram(1,0)->AddBinError2(bin,qx1*qx2*qx1*qx2);
    fOutputHistograms[iconf]->GetRotationHistogram(1,1)->AddBinError2(bin,qy1*qy2*qy1*qy2);
    fOutputHistograms[iconf]->GetRotationHistogram(1,2)->AddBinError2(bin,qx1*qy2*qx1*qy2);
    fOutputHistograms[iconf]->GetRotationHistogram(1,3)->AddBinError2(bin,qy1*qx2*qy1*qx2);
  }
  fOutputHistograms[iconf]->GetRotationHistogramE(1)->AddBinContent(bin,1.0);

}



//______________________________________
void QnCorrectionsManager::FillHistogramsQnCorrelations(QnCorrectionsConfiguration* QnConfig){

  Float_t value;
  Int_t index1 = QnConfig->QnConfigurationCorrelationIndex(0);
  Int_t index2 = QnConfig->QnConfigurationCorrelationIndex(1);
  if(index1==-1||index2==-1) return;
  Int_t iconf=QnConfig->GlobalIndex();

  QnCorrectionsQnVector* Qvecs[3];
  QnCorrectionsConfiguration* QnConf[3]={0x0};
  QnConf[0]=QnConfig;

  Int_t dim=QnConf[0]->GetAlignmentAxes()->Dim();

  QnConf[1]=GetQnConfiguration(index1);
  QnConf[2]=GetQnConfiguration(index2);

  Qvecs[1] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(index1)->At(0));
  
  if(QnConf[0]->IsRequestedCorrection(QnCorrectionsConstants::kAlignment))
           Qvecs[0] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(iconf,QnCorrectionsConstants::kAlignment)->At(0));
  else if(QnConf[0]->IsRequestedCorrection(QnCorrectionsConstants::kRecentering))
           Qvecs[0] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(iconf,QnCorrectionsConstants::kRecentering)->At(0));
  else if(QnConf[0]->IsRequestedCorrection(QnCorrectionsConstants::kDataVectorEqualization))
           Qvecs[0] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(iconf,QnCorrectionsConstants::kDataVectorEqualization)->At(0));
  else     Qvecs[0] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(iconf,QnCorrectionsConstants::kPass0)->At(0));

  if(QnConf[2]->IsRequestedCorrection(QnCorrectionsConstants::kAlignment))
           Qvecs[2] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(index2,QnCorrectionsConstants::kAlignment)->At(0));
  else if(QnConf[2]->IsRequestedCorrection(QnCorrectionsConstants::kRecentering))
           Qvecs[2] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(index2,QnCorrectionsConstants::kRecentering)->At(0));
  else if(QnConf[2]->IsRequestedCorrection(QnCorrectionsConstants::kDataVectorEqualization))
           Qvecs[2] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(index2,QnCorrectionsConstants::kDataVectorEqualization)->At(0));
  else     Qvecs[2] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(index2,QnCorrectionsConstants::kPass0)->At(0));



  for(Int_t icomb=0; icomb<3; ++icomb){ 
    for(Int_t iaxis=0; iaxis<dim; iaxis++){
    value=fDataContainer[QnConf[0]->GetAlignmentAxes()->Var(iaxis)];
      for(Int_t ih=QnConf[icomb]->MinimumHarmonic(); ih<=QnConf[icomb]->MaximumHarmonic(); ++ih){ 
        if(ih>QnConf[(icomb+1)%3]->MaximumHarmonic()) continue;
        if(Qvecs[icomb]->CheckQnVectorStatus(ih,QnCorrectionsConstants::kUndefined)||Qvecs[(icomb+1)%3]->CheckQnVectorStatus(ih,QnCorrectionsConstants::kUndefined)) continue;
        fOutputHistograms[iconf]->CorrelationProf(0,icomb,ih,0,iaxis)->Fill(value,Qvecs[icomb]->Qx(ih)*Qvecs[(icomb+1)%3]->Qx(ih));
        fOutputHistograms[iconf]->CorrelationProf(0,icomb,ih,1,iaxis)->Fill(value,Qvecs[icomb]->Qy(ih)*Qvecs[(icomb+1)%3]->Qx(ih));
        fOutputHistograms[iconf]->CorrelationProf(0,icomb,ih,2,iaxis)->Fill(value,Qvecs[icomb]->Qx(ih)*Qvecs[(icomb+1)%3]->Qy(ih));
        fOutputHistograms[iconf]->CorrelationProf(0,icomb,ih,3,iaxis)->Fill(value,Qvecs[icomb]->Qy(ih)*Qvecs[(icomb+1)%3]->Qy(ih));
      }
    }
  }

}


//______________________________________
void QnCorrectionsManager::FillHistogramsQnCorrelationsQA(QnCorrectionsConfiguration* QnConfig){

  Float_t value;
  Int_t index1 = QnConfig->QnConfigurationCorrelationIndex(0);
  Int_t index2 = QnConfig->QnConfigurationCorrelationIndex(1);
  if(index1==-1||index2==-1) return;
  Int_t iconf=QnConfig->GlobalIndex();

  QnCorrectionsQnVector* Qvecs[3];
  QnCorrectionsConfiguration* QnConf[3]={0x0};
  QnConf[0]=QnConfig;

  Int_t dim=QnConf[0]->GetAlignmentAxes()->Dim();

  QnConf[1]=GetQnConfiguration(index1);
  QnConf[2]=GetQnConfiguration(index2);

  for(Int_t icor=0; icor<2; icor++){

    Int_t correction;
    if(icor==0) correction=QnCorrectionsConstants::kTwist;
    if(icor==1) correction=QnCorrectionsConstants::kRescaling;
  
  if(QnConf[0]->IsRequestedCorrection(correction))
           Qvecs[0] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(iconf,correction)->At(0));
  if(QnConf[2]->IsRequestedCorrection(correction))
           Qvecs[2] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(index2,correction)->At(0));

  Qvecs[1] = static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(index1)->At(0));
  



  for(Int_t icomb=0; icomb<3; ++icomb){ 
    for(Int_t iaxis=0; iaxis<dim; iaxis++){
    value=fDataContainer[QnConf[0]->GetAlignmentAxes()->Var(iaxis)];
      for(Int_t ih=QnConf[icomb]->MinimumHarmonic(); ih<=QnConf[icomb]->MaximumHarmonic(); ++ih){ 
        if(ih>QnConf[(icomb+1)%3]->MaximumHarmonic()) continue;
        if(Qvecs[icomb]->CheckQnVectorStatus(ih,QnCorrectionsConstants::kUndefined)||Qvecs[(icomb+1)%3]->CheckQnVectorStatus(ih,QnCorrectionsConstants::kUndefined)) continue;
        fOutputHistograms[iconf]->CorrelationProf(icor+1,icomb,ih,0,iaxis)->Fill(value,Qvecs[icomb]->Qx(ih)*Qvecs[(icomb+1)%3]->Qx(ih));
        fOutputHistograms[iconf]->CorrelationProf(icor+1,icomb,ih,1,iaxis)->Fill(value,Qvecs[icomb]->Qy(ih)*Qvecs[(icomb+1)%3]->Qx(ih));
        fOutputHistograms[iconf]->CorrelationProf(icor+1,icomb,ih,2,iaxis)->Fill(value,Qvecs[icomb]->Qx(ih)*Qvecs[(icomb+1)%3]->Qy(ih));
        fOutputHistograms[iconf]->CorrelationProf(icor+1,icomb,ih,3,iaxis)->Fill(value,Qvecs[icomb]->Qy(ih)*Qvecs[(icomb+1)%3]->Qy(ih));
      }
    }
  }
  }

}


//______________________________________
void QnCorrectionsManager::FillHistogramsMeanQ(QnCorrectionsConfiguration* QnConf, Int_t step){
        //cout<<"requested fill "<<QnConf->QnConfigurationName()<<"  "<<endl;

  Double_t fillValues[20];
  Int_t iconf=QnConf->GlobalIndex();

  const Int_t* var = QnConf->GetRecenteringAxes()->Var();
  const Int_t  dim = QnConf->GetRecenteringAxes()->Dim();
  
  
  for(Int_t iav=0; iav<dim; iav++) fillValues[iav] = fDataContainer[var[iav]];
  Int_t bin=fOutputHistograms[iconf]->CalibrationHistogramE(0)->GetBin(fillValues);

  QnCorrectionsQnVector* Qvec=static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(iconf,step)->At(0));


  Bool_t once=kTRUE;

  Int_t maxHar = QnConf->MaximumHarmonic();
  if(QnConf->GetTwistAndRescalingMethod()==0) maxHar*=2;
  for(Int_t ih=QnConf->MinimumHarmonic(); ih<=maxHar; ++ih){

  Double_t weight=1.;
  //if(QnConf->GetQnNormalizationMethod()==1) weight = Qvec->SumOfWeights();

    if(Qvec->CheckQnVectorStatus(ih, step)){
      fOutputHistograms[iconf]->CalibrationHistogramQ(step,ih,0)->AddBinContent(bin,Qvec->Qx(ih)*weight);
      fOutputHistograms[iconf]->CalibrationHistogramQ(step,ih,1)->AddBinContent(bin,Qvec->Qy(ih)*weight);
      fOutputHistograms[iconf]->CalibrationHistogramQ(step,ih,0)->AddBinError2(bin,Qvec->Qx(ih)*weight*Qvec->Qx(ih)*weight);
      fOutputHistograms[iconf]->CalibrationHistogramQ(step,ih,1)->AddBinError2(bin,Qvec->Qy(ih)*weight*Qvec->Qy(ih)*weight);
      if(once) {
        THnF* entr = fOutputHistograms[iconf]->CalibrationHistogramE(step);
        entr->AddBinContent(bin,weight);
        Int_t entries = entr->GetEntries();
        entr->SetEntries(++entries);
        once=kFALSE;
      }

    }
  }

  return;

}


//______________________________________
void QnCorrectionsManager::FillHistogramsMeanQ_QA(QnCorrectionsConfiguration* QnConf){


  Double_t fillValues[20];
  Int_t iconf=QnConf->GlobalIndex();


  const Int_t* var = QnConf->GetRecenteringAxes()->Var();
  const Int_t  dim = QnConf->GetRecenteringAxes()->Dim();
  for(Int_t iav=0; iav<dim; iav++) fillValues[iav] = fDataContainer[var[iav]];

  Int_t bin=fOutputHistograms[iconf]->CalibrationHistogramE(0)->GetBin(fillValues);

  for(Int_t istep=0; istep<=fCorrectionStep; istep++){
    if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kDataVectorEqualization)) {if(istep==1) continue;}
    else if(istep==0) continue;

    QnCorrectionsQnVector* Qvec=static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(iconf,istep)->At(0));
    if(!Qvec) continue;
    if(Qvec->N()==0) continue;


    Bool_t once=kTRUE;

    for(Int_t ih=QnConf->MinimumHarmonic(); ih<=QnConf->MaximumHarmonic(); ++ih){

      if(Qvec->CheckQnVectorStatus(ih, istep)){
        fOutputHistograms[iconf]->CalibrationHistogramQ(istep,ih,0)->AddBinContent(bin,Qvec->Qx(ih));
        fOutputHistograms[iconf]->CalibrationHistogramQ(istep,ih,1)->AddBinContent(bin,Qvec->Qy(ih));
        if(once) {
          THnF* entr = fOutputHistograms[iconf]->CalibrationHistogramE(istep);
          entr->AddBinContent(bin);
          Int_t entries = entr->GetEntries();
          entr->SetEntries(++entries);
          once=kFALSE;
        }
      }
    }
  }

  return;

}



//_________________________________________________________________
void QnCorrectionsManager::FillHistogramsU2n(QnCorrectionsConfiguration* QnConf){

  if(QnConf->GetTwistAndRescalingMethod()!=0) return;
  Double_t fillValues[20];
  const Int_t* var = QnConf->GetTwistAndRescalingAxes()->Var();
  const Int_t  dim = QnConf->GetTwistAndRescalingAxes()->Dim();
  for(Int_t iav=0; iav<dim; iav++) fillValues[iav] = fDataContainer[var[iav]];

  Int_t bin=fOutputHistograms[QnConf->GlobalIndex()]->U2nHistogramE()->GetBin(fillValues);

  TClonesArray* dataVectorArray = GetConfDataVectors(QnConf->GlobalIndex());
  //TIter nextEntry(dataVectorArray);
  QnCorrectionsDataVector* dataVector=0x0;
  //while((dataVector=static_cast<QnCorrectionsDataVector*>(nextEntry()))) {
  //  if(!dataVector) continue;

  THnF* entr = fOutputHistograms[QnConf->GlobalIndex()]->U2nHistogramE();
  for(Int_t idata=0; idata<dataVectorArray->GetEntriesFast(); idata++){
    dataVector = static_cast<QnCorrectionsDataVector*>(dataVectorArray->At(idata));
    //if(!dataVector->CheckEventPlaneDetector(QnConf->LocalIndex())) continue;
    for(Int_t ih=QnConf->MinimumHarmonic(); ih<=QnConf->MaximumHarmonic(); ++ih) {
      fOutputHistograms[QnConf->GlobalIndex()]->U2nHistogram(ih,0)->AddBinContent(bin,TMath::Cos(dataVector->Phi()*ih*2));
      fOutputHistograms[QnConf->GlobalIndex()]->U2nHistogram(ih,1)->AddBinContent(bin,TMath::Sin(dataVector->Phi()*ih*2));
    }
    entr->AddBinContent(bin);
    Int_t entries = entr->GetEntries();
    entr->SetEntries(++entries);

  }
}



//_____________________________________________________________________
void QnCorrectionsManager::CallStepTwistAndRescaleQnVector(QnCorrectionsConfiguration* QnConf) {

  //
  // Recenter the detector event plane
  //
  
  
  Int_t bin=0;
  Int_t maxHarmonic;
  Double_t fillValues[20];
  const Int_t* var = QnConf->GetTwistAndRescalingAxes()->Var();
  const Int_t  dim = QnConf->GetTwistAndRescalingAxes()->Dim();
  for(Int_t iav=0; iav<dim; iav++) fillValues[iav] = fDataContainer[var[iav]];

  QnCorrectionsQnVector* QvectorIn= static_cast<QnCorrectionsQnVector*>(CorrectedQnVector(QnConf->GlobalIndex())->At(0));
  TClonesArray& arr = *(fCorrectedQvectors[QnConf->GlobalIndex()][QnCorrectionsConstants::kTwist]);
  QnCorrectionsQnVector* QvectorTwist   = new(arr[0]) QnCorrectionsQnVector(*QvectorIn);
  TClonesArray& arr2 = *(fCorrectedQvectors[QnConf->GlobalIndex()][QnCorrectionsConstants::kRescaling]);
  QnCorrectionsQnVector* QvectorRescale = new(arr2[0]) QnCorrectionsQnVector(*QvectorIn);

  if(QnConf->IsApplyCorrection(QnCorrectionsConstants::kTwist))     fLastStep[QnConf->GlobalIndex()]=QnCorrectionsConstants::kTwist;
  if(QnConf->IsApplyCorrection(QnCorrectionsConstants::kRescaling)) fLastStep[QnConf->GlobalIndex()]=QnCorrectionsConstants::kRescaling;

  //Double_t cos2n, sin2n, Lp, Ln, Ap, An, Qx, Qy, QxTwist, QyTwist, QxRescaling, QyRescaling, nentries;



  if(QnConf->GetTwistAndRescalingMethod()==0){

  bin=fInputHistograms[QnConf->GlobalIndex()]->CalibrationHistogramE(0)->GetBin(fillValues);

  QnCorrectionsSteps::TwistAndRescale2nQn(QvectorIn, QvectorTwist, QvectorRescale, fInputHistograms[QnConf->GlobalIndex()], bin, QnConf->MinimumHarmonic(), QnConf->MaximumHarmonic(),QnConf->IsApplyCorrection(QnCorrectionsConstants::kTwist), QnConf->IsApplyCorrection(QnCorrectionsConstants::kRescaling));

  }

  if(QnConf->GetTwistAndRescalingMethod()==2){

    Int_t eventClassParameter=0; // for this method we use profiles and only do twist and rescaling corrections against one user defined axis (currently hardcoded as the first axis)

  bin=fInputHistograms[QnConf->GlobalIndex()]->CorrelationProf(0, 0, QnConf->MinimumHarmonic(), 0, eventClassParameter)->FindBin(fDataContainer[var[eventClassParameter]]);

  QnCorrectionsSteps::TwistAndRescale3DetectorCorrelation(QvectorIn, QvectorTwist, QvectorRescale, fInputHistograms[QnConf->GlobalIndex()], bin, QnConf->MinimumHarmonic(), QnConf->MaximumHarmonic(),QnConf->IsApplyCorrection(QnCorrectionsConstants::kTwist), QnConf->IsApplyCorrection(QnCorrectionsConstants::kRescaling),eventClassParameter);

  }


  return;
}



//__________________________________________________________________
void QnCorrectionsManager::DisableFillHistograms(QnCorrectionsConstants::CorrectionSteps step){

    for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
    QnCorrectionsConfiguration* QnConf = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf);

      for(Int_t istep=(Int_t) step; istep<QnCorrectionsConstants::kNcorrectionSteps; istep++){
        QnConf->SetFillHistogram(istep,kFALSE);
      }

    }

}

//__________________________________________________________________
void QnCorrectionsManager::DisableCorrections(QnCorrectionsConstants::CorrectionSteps step){

    for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
    QnCorrectionsConfiguration* QnConf = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf);

      QnConf->SetApplyCorrection(step,kFALSE);

    }

}


//__________________________________________________________________
void QnCorrectionsManager::InitializeCalibrationHistograms(){


  QnCorrectionsConfiguration* QnConf = 0x0;
  for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
    QnConf = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf);
    if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kDataVectorEqualization)) if(fCorrectionStep<QnCorrectionsConstants::kDataVectorEqualization) {fCorrectionStep++;}
    if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kRecentering))            if(fCorrectionStep<QnCorrectionsConstants::kRecentering           ) {fCorrectionStep++;}
    if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kAlignment))              if(fCorrectionStep<QnCorrectionsConstants::kAlignment             ) {fCorrectionStep++;}
    if(QnConf->GetTwistAndRescalingMethod()==2){                                                                                                   
      if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kTwist))                if(fCorrectionStep<QnCorrectionsConstants::kTwist                 ) fCorrectionStep++;
      if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kRescaling))            if(fCorrectionStep<QnCorrectionsConstants::kTwist                 ) fCorrectionStep++;
    }
  }

  fPassesRequired=fCorrectionStep;


  for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
    QnConf = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf);

    TString label="allData";
    if(QnConf->CorrectWithEventLabel()) label=fLabel;
    THashList* list = GetInputListWithLabel(fLabel);
    if(!list)  list = GetInputListWithLabel("allData");

    Int_t step;

    step=(Int_t)QnCorrectionsConstants::kDataVectorEqualization;
    if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kDataVectorEqualization)){
      if(!fInputHistograms[iconf]->ConnectMultiplicityHistograms(list,QnConf)){
        DisableCorrections(QnCorrectionsConstants::kDataVectorEqualization);
        fCorrectionStep=step-1;
        if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kRecentering)) DisableFillHistograms(QnCorrectionsConstants::kRecentering);
      }
    };

    step=(Int_t)QnCorrectionsConstants::kRecentering;
    if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kRecentering)){
      if(!fInputHistograms[iconf]->ConnectMeanQnCalibrationHistograms(list,QnConf)){
         DisableCorrections(QnCorrectionsConstants::kRecentering);
         DisableFillHistograms(QnCorrectionsConstants::kAlignment);
         DisableFillHistograms(QnCorrectionsConstants::kTwist);
         DisableFillHistograms(QnCorrectionsConstants::kRescaling);
         if(fCorrectionStep>=step) fCorrectionStep=step-1;
      }
    };

    step=(Int_t)QnCorrectionsConstants::kTwist;
    if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kTwist)){
      if(QnConf->GetTwistAndRescalingMethod()==0||QnConf->GetTwistAndRescalingMethod()==1){
        //if(!fInputHistograms[iconf]->ConnectU2nQnCalibrationHistograms(list,QnConf)){
        if(!fInputHistograms[iconf]->ConnectMeanQnCalibrationHistograms(list,QnConf)){
          DisableCorrections(QnCorrectionsConstants::kTwist);
          DisableCorrections(QnCorrectionsConstants::kRescaling);
          if(fCorrectionStep>=step) fCorrectionStep=step-1;
        }
      }
      if(QnConf->GetTwistAndRescalingMethod()==2){
        if(!fInputHistograms[iconf]->ConnectCorrelationQnCalibrationHistograms(list,QnConf)){
          QnConf->SetApplyCorrection(QnCorrectionsConstants::kTwist,kFALSE);
          QnConf->SetApplyCorrection(QnCorrectionsConstants::kRescaling,kFALSE);
          if(fCorrectionStep>=step) fCorrectionStep=step-1;
      }}
    };

    
    step=(Int_t)QnCorrectionsConstants::kAlignment;
    if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kAlignment)){
        if(!fInputHistograms[iconf]->ConnectRotationQnCalibrationHistograms(list,QnConf)){
          DisableCorrections(QnCorrectionsConstants::kAlignment);
          if(fCorrectionStep>=step) fCorrectionStep=step-1;
          if(QnConf->GetTwistAndRescalingMethod()==2){
            QnConf->SetFillHistogram(QnCorrectionsConstants::kTwist,kFALSE);
            QnConf->SetFillHistogram(QnCorrectionsConstants::kRescaling,kFALSE);
          }
        }
    };
  }


}

  

//__________________________________________________________________
void QnCorrectionsManager::PrintFrameworkInformation(){



	const int widthEntry = 18;
	const int widthBar = 3;

  Bool_t forCorrection=kTRUE;
  PrintFrameworkInformationLine(!forCorrection, "----------------", QnCorrectionsConstants::kNothing, widthEntry, widthBar);
  PrintFrameworkInformationLine(forCorrection, Form("FLOW VECTOR FRAMEWORK - PASS %d/%d", fCorrectionStep+1, fPassesRequired+1), QnCorrectionsConstants::kNothing, widthEntry, widthBar);
  PrintFrameworkInformationLine(!forCorrection, " ", QnCorrectionsConstants::kNothing, widthEntry, widthBar);
  PrintFrameworkInformationLine(forCorrection, "--FLOW VECTORS--", QnCorrectionsConstants::kNothing, widthEntry, widthBar);
    
  cout<<setw(widthEntry)<<" "<<setw(widthBar)<<"|";
  for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
    QnCorrectionsConfiguration* QnConf = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf);
    cout<<setw(widthEntry)<<QnConf->QnConfigurationName()<<setw(widthBar)<<"  |";
  }
  cout<<endl;

  PrintFrameworkInformationLine(!forCorrection, "CORRECTIONS", QnCorrectionsConstants::kNothing, widthEntry, widthBar);
  PrintFrameworkInformationLine(forCorrection, "Equalization", QnCorrectionsConstants::kDataVectorEqualization, widthEntry, widthBar);
  PrintFrameworkInformationLine(forCorrection, "Recentering", QnCorrectionsConstants::kRecentering, widthEntry, widthBar);
  PrintFrameworkInformationLine(forCorrection, "Alignment", QnCorrectionsConstants::kAlignment, widthEntry, widthBar);
  PrintFrameworkInformationLine(forCorrection, "Twist", QnCorrectionsConstants::kTwist, widthEntry, widthBar);
  PrintFrameworkInformationLine(forCorrection, "Rescaling", QnCorrectionsConstants::kRescaling, widthEntry, widthBar);
  PrintFrameworkInformationLine(!forCorrection, "FILL HISTS", QnCorrectionsConstants::kNothing, widthEntry, widthBar);
  PrintFrameworkInformationLine(!forCorrection, "Equalization", QnCorrectionsConstants::kDataVectorEqualization, widthEntry, widthBar);
  PrintFrameworkInformationLine(!forCorrection, "Recentering", QnCorrectionsConstants::kRecentering, widthEntry, widthBar);
  PrintFrameworkInformationLine(!forCorrection, "Alignment", QnCorrectionsConstants::kAlignment, widthEntry, widthBar);
  PrintFrameworkInformationLine(!forCorrection, "Twist", QnCorrectionsConstants::kTwist, widthEntry, widthBar);
  PrintFrameworkInformationLine(!forCorrection, "Rescaling", QnCorrectionsConstants::kRescaling, widthEntry, widthBar);
  PrintFrameworkInformationLine(!forCorrection, "----------------", QnCorrectionsConstants::kNothing, widthEntry, widthBar);
      
  cout<<setw(widthEntry)<<"Legend "<<setw(widthBar)<<"|";
  cout<<setw(widthEntry)<<"x: this pass"<<setw(widthBar)<<" ";
  cout<<setw(widthEntry)<<"0: future pass"<<setw(widthBar)<<" ";
  cout<<setw(widthEntry)<<"-: N/A"<<setw(widthBar)<<"|"<<endl;

  PrintFrameworkInformationLine(!forCorrection, "----------------", QnCorrectionsConstants::kNothing, widthEntry, widthBar);


}

//__________________________________________________________________
void QnCorrectionsManager::PrintFrameworkInformationLine(Bool_t HistOrCor,TString correctionname, QnCorrectionsConstants::CorrectionSteps stepflag, Int_t widthEntry, Int_t widthBar){





  if(HistOrCor&&stepflag!=QnCorrectionsConstants::kNothing){
    cout<<setw(widthEntry)<<correctionname<<setw(widthBar)<<"|";
    for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
      QnCorrectionsConfiguration* QnConf = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf);
      if(QnConf->IsApplyCorrection(stepflag)) cout<<setw(widthEntry)<<"x"<<setw(widthBar)<<"|";
      else if(QnConf->IsRequestedCorrection(stepflag)) cout<<setw(widthEntry)<<"0"<<setw(widthBar)<<"|";
      else cout<<setw(widthEntry)<<"-"<<setw(widthBar)<<"|";
    }
  cout<<endl;
  }


  
  if(!HistOrCor&&stepflag!=QnCorrectionsConstants::kNothing){
    cout<<setw(widthEntry)<<correctionname<<setw(widthBar)<<"|";
    for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
      QnCorrectionsConfiguration* QnConf = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf);
      if(QnConf->IsFillHistogram(stepflag)) cout<<setw(widthEntry)<<"x"<<setw(widthBar)<<"|";
      else if(QnConf->IsRequestedFillHistogram(stepflag)) cout<<setw(widthEntry)<<"0"<<setw(widthBar)<<"|";
      else cout<<setw(widthEntry)<<"-"<<setw(widthBar)<<"|";
    }
  cout<<endl;
  }

  
  
  if(HistOrCor&&stepflag==QnCorrectionsConstants::kNothing){
    cout<<setw(widthEntry)<<" "<<setw(widthBar)<<"|";
    for(Int_t iconf=0; iconf<fNumberOfQnConfigurations/2; iconf++){
      cout<<setw(widthEntry)<<" ";
    }
    cout<<correctionname;
    for(Int_t iconf=fNumberOfQnConfigurations/2; iconf<fNumberOfQnConfigurations; iconf++){
      cout<<setw(widthEntry)<<" ";
    }
    cout<<endl;
  }


    
  if(!HistOrCor&&stepflag==QnCorrectionsConstants::kNothing){
    cout<<setw(widthEntry)<<correctionname<<setw(widthBar)<<"-";
    for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
        for(Int_t i=0; i<widthEntry; i++) cout<<"-";
        for(Int_t i=0; i<widthBar; i++) cout<<"-";
    }
  cout<<endl;
  }




}



//__________________________________________________________________
void QnCorrectionsManager::WriteCalibrationHistogramsToList()
{
  //
  // Finish Task 
  //
  THashList* listall = new THashList();
  TObjArray* runarr = fRunList.Tokenize(";");
  Int_t nruns = runarr->GetEntries();
  if(nruns==0){
    if(fLabel.EqualTo("")) {
      fRunList="allData;";
      fLabel="allData";
    } 
    else {
      fRunList=fLabel+";";
    }
    runarr=fRunList.Tokenize(";");
  };
  for(Int_t ir=0; ir<runarr->GetEntries(); ir++){
    TString runlabel = runarr->At(ir)->GetName();

    QnCorrectionsHistograms* histos;

  THashList* dataByRun = new THashList();
  //THashList* dataByRun = fListOutputHistogramsQnCorrections;//new THashList();
  dataByRun->SetName(runlabel);
  dataByRun->SetOwner(kTRUE);

  QnCorrectionsConfiguration* QnConf = 0x0;
  for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
    QnConf = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf);
    if(!QnConf) continue;

    if(runlabel==fLabel) histos = fOutputHistograms[iconf];
    else {histos = new QnCorrectionsHistograms();histos->CreateCalibrationHistograms(QnConf);}

    THashList* detector = new THashList();
    THashList* detectorM = new THashList();
    THashList* detectorC = new THashList();
    detector->SetOwner();
    detectorM->SetOwner();
    detectorC->SetOwner();

    if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kRecentering)||(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kTwist)&&QnConf->GetTwistAndRescalingMethod())==0){
      Int_t maxHar = QnConf->MaximumHarmonic();
      if(QnConf->GetTwistAndRescalingMethod()==0) maxHar*=2;
        for(Int_t ih=QnConf->MinimumHarmonic(); ih<=maxHar; ++ih) {
          for(Int_t ic=0; ic<2; ++ic){
            detector->Add(histos->CalibrationHistogramQ( (QnConf->IsRequestedCorrection(QnCorrectionsConstants::kDataVectorEqualization) ? 1 : 0) ,ih,ic));
          }
        }
        detector->Add(histos->CalibrationHistogramE((QnConf->IsRequestedCorrection(QnCorrectionsConstants::kDataVectorEqualization) ? 1 : 0) ));
    }

    if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kTwist)&&QnConf->GetTwistAndRescalingMethod()==1){
    for(Int_t ih=QnConf->MinimumHarmonic(); ih<=QnConf->MaximumHarmonic(); ++ih){
      if(histos->U2nHistogram(ih,0)){
        detector->Add(histos->U2nHistogram(ih,0));
        detector->Add(histos->U2nHistogram(ih,1));
      }
    }
    detector->Add(histos->U2nHistogramE());
    }

    detector->SetName(Form("Qvec%s",QnConf->QnConfigurationName().Data()));
    if(detector->GetEntries()!=0){
      dataByRun->Add(detector);
    }


    if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kDataVectorEqualization)){
      //for(Int_t idim=0; idim<10; idim++) if(histos->EventHistogram(idim)) detectorM->Add(histos->EventHistogram(idim));
      for(Int_t is=0; is<1; ++is){
        if(histos->EqualizationHistogramM(is)) detectorM->Add(histos->EqualizationHistogramM(is));
        if(histos->EqualizationHistogramE(is)) detectorM->Add(histos->EqualizationHistogramE(is));
        if(histos->GroupEqualizationHistogramM(is)) detectorM->Add(histos->GroupEqualizationHistogramM(is));
        if(histos->GroupEqualizationHistogramE(is)) detectorM->Add(histos->GroupEqualizationHistogramE(is));
      }
      detectorM->SetName(Form("Mult%s",QnConf->QnConfigurationName().Data()));
      if(detectorM->GetEntries()!=0){
        dataByRun->Add(detectorM);
      }
    }

  //  

    if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kAlignment)){
      for(Int_t is=0; is<4; ++is) if(histos->GetRotationHistogram(0,is)) detectorC->Add(histos->GetRotationHistogram(0,is));
      if(histos->GetRotationHistogramE(0)) detectorC->Add(histos->GetRotationHistogramE(0));
    }




    if((QnConf->IsRequestedCorrection(QnCorrectionsConstants::kTwist)||QnConf->IsRequestedCorrection(QnCorrectionsConstants::kRescaling))&&QnConf->GetTwistAndRescalingMethod()==2){
    for(Int_t icomb=0; icomb<3; ++icomb){ 
      for(Int_t icomp=0; icomp<4; ++icomp){ 
        for(Int_t iaxis=0; iaxis<=QnConf->GetAlignmentAxes()->Dim(); ++iaxis){ 
          for(Int_t ih=QnConf->MinimumHarmonic(); ih<=QnConf->MaximumHarmonic(); ++ih){ 
            if(histos->CorrelationProf(0,icomb,ih,icomp,iaxis)) detectorC->Add(histos->CorrelationProf(0,icomb,ih,icomp,iaxis));
      }}}}
      detectorC->SetName(Form("Correlations%s",QnConf->QnConfigurationName().Data()));
    }
    if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kAlignment)||((QnConf->IsRequestedCorrection(QnCorrectionsConstants::kTwist)||QnConf->IsRequestedCorrection(QnCorrectionsConstants::kRescaling))&&QnConf->GetTwistAndRescalingMethod()==2)){
      detectorC->SetName(Form("Correlations%s",QnConf->QnConfigurationName().Data()));
      dataByRun->Add(detectorC);
    }
  }

  
  if(runlabel==fLabel){
    listall->SetName("allData");
    for(Int_t i=0; i<dataByRun->GetEntries(); i++){
      THashList* l = (THashList*) dataByRun->At(i);
      l->SetOwner();
      listall->Add(l->Clone());
    }
  }

  if(!(runlabel.EqualTo("allData"))) fListOutputHistogramsQnCorrections->Add(dataByRun->Clone());
  
  }

  fListOutputHistogramsQnCorrections->Add(listall);
  //fListOutputHistogramsQnCorrections->Add(dataAllRuns);

}




//__________________________________________________________________
void QnCorrectionsManager::WriteQaHistogramsToList()
{
  //
  // Finish Task 
  //

  THashList* listall = new THashList();


  //detector->SetOwner();
  //detectorMqa->SetOwner();
  //detectorC->SetOwner();
  //dataByRunQA->SetOwner();


  QnCorrectionsHistograms* histos;

  THashList* dataByRunQA = new THashList();
  dataByRunQA->SetOwner();
    //dataByRunQA->Clear();


  dataByRunQA->SetName(fLabel);


  QnCorrectionsConfiguration* QnConf = 0x0;
  for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
    QnConf = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf);
    if(!QnConf) continue;
    THashList* detector    = new THashList();
    THashList* detectorMqa = new THashList();
    THashList* detectorC   = new THashList();
    detector->SetOwner();
    detectorMqa->SetOwner();
    detectorC->SetOwner();
    //detector->Clear();
    //detectorMqa->Clear();
    //detectorC->Clear();

    histos = fOutputHistograms[iconf];

     Int_t istep=0;
    for(Int_t istep=0; istep<QnCorrectionsConstants::nCorrectionSteps; istep++){
      if(istep==0) if(!QnConf->IsRequestedCorrection(QnCorrectionsConstants::kDataVectorEqualization)) continue;
      if(istep==1) continue;
      if(istep>1)  if(!QnConf->IsRequestedCorrection(istep)) continue;
      

    Int_t maxHar = QnConf->MaximumHarmonic();
      for(Int_t ih=QnConf->MinimumHarmonic(); ih<=maxHar; ++ih) {
        for(Int_t ic=0; ic<2; ++ic){
          detector->Add(histos->CalibrationHistogramQ(istep,ih,ic));
        }
      }
      detector->Add(histos->CalibrationHistogramE(istep));
    }

    for(Int_t ih=QnConf->MinimumHarmonic(); ih<=QnConf->MaximumHarmonic(); ++ih){
      if(histos->U2nHistogram(ih,0)){
        detector->Add(histos->U2nHistogram(ih,0));
        detector->Add(histos->U2nHistogram(ih,1));
      }
    }
    if(histos->U2nHistogramE()) detector->Add(histos->U2nHistogramE());

    detector->SetName(Form("Qvec%s",QnConf->QnConfigurationName().Data()));
    dataByRunQA->Add(detector);


    if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kDataVectorEqualization)){
      for(Int_t is=1; is<3; ++is){
        if(histos->EqualizationHistogramM(is)) detectorMqa->Add(histos->EqualizationHistogramM(is));
        if(histos->EqualizationHistogramE(is)) detectorMqa->Add(histos->EqualizationHistogramE(is));
      }
      detectorMqa->SetName(Form("Mult%s",QnConf->QnConfigurationName().Data()));
      dataByRunQA->Add(detectorMqa);
    }

    if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kAlignment)){
      for(Int_t is=0; is<4; ++is) if(histos->GetRotationHistogram(1,is)) detectorC->Add(histos->GetRotationHistogram(1,is));
      if(histos->GetRotationHistogramE(1)) detectorC->Add(histos->GetRotationHistogramE(1));
    }

    
    if((QnConf->IsRequestedCorrection(QnCorrectionsConstants::kTwist)||QnConf->IsRequestedCorrection(QnCorrectionsConstants::kRescaling))&&QnConf->GetTwistAndRescalingMethod()==2){
    for(Int_t icomb=0; icomb<3; ++icomb){ 
      for(Int_t icomp=0; icomp<4; ++icomp){ 
        for(Int_t iaxis=0; iaxis<=QnConf->GetAlignmentAxes()->Dim(); ++iaxis){ 
          for(Int_t ih=QnConf->MinimumHarmonic(); ih<=QnConf->MaximumHarmonic(); ++ih){ 
            if(histos->CorrelationProf(1,icomb,ih,icomp,iaxis)) detectorC->Add(histos->CorrelationProf(1,icomb,ih,icomp,iaxis));
            if(histos->CorrelationProf(2,icomb,ih,icomp,iaxis)) detectorC->Add(histos->CorrelationProf(2,icomb,ih,icomp,iaxis));
      }}}}
    }

    if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kAlignment)||((QnConf->IsRequestedCorrection(QnCorrectionsConstants::kTwist)||QnConf->IsRequestedCorrection(QnCorrectionsConstants::kRescaling))&&QnConf->GetTwistAndRescalingMethod()==2)){
      detectorC->SetName(Form("Correlations%s",QnConf->QnConfigurationName().Data()));
      dataByRunQA->Add(detectorC);
    }


    //for(Int_t istep=0; istep<=QnCorrectionsConstants::kNcorrectionSteps; istep++){
    //  for(Int_t icomb=0; icomb<3; ++icomb){ 
    //    for(Int_t icomp=0; icomp<4; ++icomp){ 
    //      for(Int_t iaxis=0; iaxis<=QnConf->CalibrationBinning()->Dim(); ++iaxis){ 
    //      for(Int_t ih=QnConf->MinimumHarmonic(); ih<=QnConf->MaximumHarmonic(); ++ih){ 
    //        //if(histos->CorrelationProf(QnConf->GetCorrectionStep(istep),icomb,ih,icomp,iaxis)&&QnConf->GetCorrectionStep(istep)>2) detectorCqa->Add(histos->CorrelationProf(QnConf->GetCorrectionStep(istep),icomb,ih,icomp,iaxis));
    //        //if(histos->CorrelationEpProf(QnConf->GetCorrectionStep(istep),icomb,ih,icomp,iaxis)) detectorCqa->Add(histos->CorrelationEpProf(QnConf->GetCorrectionStep(istep),icomb,ih,icomp,iaxis));
    //      }}}}}
    //detectorCqa->SetName(Form("Correlations%s",QnConf->QnConfigurationName().Data()));
    //dataByRunQA->Add(detectorCqa);
    //dataAllRunsQA->Add(detectorCqa);

  }
  //if(!(fLabel.EqualTo("allData"))){
  //  listall = (THashList*) dataByRunQA->Clone("allData");
  //  //listall->SetName("allData");
  //  //for(Int_t i=0; i<dataByRunQA->GetEntries(); i++){
  //  //  THashList* l = (THashList*) dataByRunQA->At(i);
  //  //  l->SetOwner();
  //  //  listall->Add(l->Clone());
  //  //}
  //}

  fListHistogramsQA->Add(dataByRunQA->Clone());


  //fListHistogramsQA->Add(listall);


}

//_______________________________________________________________________________
void QnCorrectionsManager::AddQnConfiguration(QnCorrectionsConfiguration* QnConf, Int_t type)
{
    
  AddDetectorType(type);
  Int_t detId=fDetectorIdMap[type]-1;
  
  QnConf->SetDetectorId( (UShort_t) detId);
  TClonesArray& eparr = *(fQnCorrectionsConfigurations[ detId]);
  eparr[fNumberOfQnConfigurationsForDetector[ detId]]=QnConf;
  fIndex[fNumberOfQnConfigurations][0] = detId;
  fIndex[fNumberOfQnConfigurations][1] = fNumberOfQnConfigurationsForDetector[ detId];
  QnConf->SetLocalIndex(fNumberOfQnConfigurationsForDetector[detId]);
  QnConf->SetGlobalIndex(fNumberOfQnConfigurations);
  if(QnConf->IsRequestedCorrection(QnCorrectionsConstants::kAlignment)) if(QnConf->MinimumHarmonic()>QnConf->AlignmentHarmonic()) QnConf->SetMinimumHarmonic(QnConf->AlignmentHarmonic());
  //QnConf->SetPassNumbers();
  fNumberOfQnConfigurationsForDetector[detId]++;
  fNumberOfQnConfigurations++;
}


//_______________________________________________________________________________
QnCorrectionsConfiguration* QnCorrectionsManager::GetQnConfiguration(TString name)
{

  QnCorrectionsConfiguration* QnConf = 0x0;
  for(Int_t iconf=0; iconf<fNumberOfQnConfigurations; iconf++){
    QnConf = (QnCorrectionsConfiguration*) GetQnConfiguration(iconf);
    if(!QnConf) continue;
    if(name.EqualTo(QnConf->QnConfigurationName())) return QnConf;
  }
  return 0x0;
}

//_______________________________________________________________________________
void QnCorrectionsManager::Finalize()
{
  if(fSetFillHistogramsQnCorrections)  WriteCalibrationHistogramsToList();
  if(fSetFillHistogramsQA)             WriteQaHistogramsToList();
  WriteOutputToFile();
}

//_______________________________________________________________________________
void QnCorrectionsManager::WriteOutputToFile()
{
  if(fOutputHistogramsQnCorrectionsFile&&fSetFillHistogramsQnCorrections){
    fOutputHistogramsQnCorrectionsFile->cd();
    fListOutputHistogramsQnCorrections->Write(fListOutputHistogramsQnCorrections->GetName(),TObject::kSingleKey);
    //fListOutputHistogramsQnCorrections->Write(fLabel,TObject::kSingleKey);
    //if(!(fLabel.EqualTo("allData"))) fListOutputHistogramsQnCorrections->Write("allData",TObject::kSingleKey);
  }
  if(fHistogramsQAFile&&fSetFillHistogramsQA){
    fHistogramsQAFile->cd();
    fListHistogramsQA->Write(fListHistogramsQA->GetName(),TObject::kSingleKey);
  }
  if(fTreeQnVectorsFile&&fSetFillTreeQnVectors){
    fTreeQnVectorsFile->cd();
    fTreeQnVectors->Write();
  }
}
