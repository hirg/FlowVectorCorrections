
#ifndef QNCORRECTIONS_CUTS_H
/* Copyright(c) 1998-2015, ALICE Experiment at CERN, All rights reserved. *
* See cxx source for full Copyright notice */

 
#define QNCORRECTIONS_CUTS_H

#include <TClonesArray.h>
#include <TBits.h>
#include <TMath.h>
#include <TArrayS.h>
#include <TString.h>
#include "iostream"



//_____________________________________________________________________
class QnCorrectionsCuts : public TObject {

 public:
  QnCorrectionsCuts();
  ~QnCorrectionsCuts();
    
  // setters
  void AddCut(Int_t type, Float_t min, Float_t max, Bool_t excludeRange=kFALSE) {fCuts[fNcuts][0]= type; fCuts[fNcuts][1]=min; fCuts[fNcuts][2]=max; fCuts[fNcuts][3]= (excludeRange ? 1.0 : 0.0); fNcuts++;}
  void AddFlag(Int_t type, Int_t flag, Bool_t accept) {fCuts[fNcuts][0]= type; fCuts[fNcuts][1]=-999.0; fCuts[fNcuts][2]=flag; fCuts[fNcuts][3]= (accept ? 1.0 : 0.0); fNcuts++;}
  void CopyCuts(QnCorrectionsCuts* cuts);
  void SetName(TString name) {fName=name;}

  // getters
  Int_t Type(Int_t cut) const {return ((Int_t) (fCuts[cut][0]+10e-6));}
  Float_t Min(Int_t cut) const {return fCuts[cut][1];}
  Float_t Max(Int_t cut) const {return fCuts[cut][2];}
  Bool_t ExcludeRange(Int_t cut) const {return (fCuts[cut][3]<0.5 ? kFALSE : kTRUE);}
  Int_t Ncuts() const {return fNcuts;}
  Bool_t  IsSelected(Float_t* values) const;
  TString GetCutsName() const {return fName;}

  
 private:

  Float_t fCuts[QnCorrectionsConstants::nCuts][4];
  Int_t fNcuts;
  TString fName;


  ClassDef(QnCorrectionsCuts, 1);
};



//_____________________________________________________________________
inline Bool_t QnCorrectionsCuts::IsSelected(Float_t* values) const {
  //
  // track selection for TPC event plane
  //

  for(Int_t icut=0; icut<Ncuts(); icut++){

    if(Min(icut)>-999.5&&Min(icut)<-998.5){
      Int_t flag=(Int_t) (values[(Int_t) (Type(icut)+Max(icut))]);
      //cout<<flag<<"  "<<Type(icut)<<"  "<<Max(icut)<<"  "<<values[(Int_t) (Type(icut)+Max(icut))]<<"  "<<Type(icut)<<"  "<<Max(icut)<<endl;
      if(!ExcludeRange(icut)&&flag) return kFALSE;
      if(ExcludeRange(icut)&&flag==0) return kFALSE;
    }
    else if(!ExcludeRange(icut)){
      //cout<<values[Type(icut)]<<"  "<<Min(icut)<<"  "<<values[Type(icut)]<<"  "<<Max(icut)<<endl;
      if(values[Type(icut)]<=Min(icut)) return kFALSE;
      if(values[Type(icut)]>=Max(icut)) return kFALSE;
    }
    else {
      if(values[Type(icut)]>Min(icut)&&values[Type(icut)]<Max(icut)) return kFALSE;
    }

  }
  return kTRUE;
}




#endif
