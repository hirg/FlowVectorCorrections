/************************************************************************** 
 * Copyright(c) 1998-2015, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/
 
 /* 
 Contact:
 Jaap Onderwaater, GSI, jacobus.onderwaater@cern.ch 
 Ilya Selyuzhenkov, GSI, ilya.selyuzhenkov@gmail.com
*/

 
#include "QnCorrectionsConstants.h"
#include "QnCorrectionsAxes.h"

#include <TObject.h>
#include <TMath.h>
#include <TArrayI.h>
#include <TArrayD.h>
//#include <TList.h>
//#include <TClonesArray.h>
//#include <TRandom3.h>
//#include <TArrayS.h>
#include <TAxis.h>
#include <iostream>

ClassImp(QnCorrectionsAxes)


//____________________________________________________________________________
QnCorrectionsAxes::QnCorrectionsAxes(Int_t dim) :
  // Constructor
  TObject(),
  fDim(dim)
{
  for(Int_t i=0; i<QnCorrectionsConstants::nHistogramDimensions; i++) {fAxes[i]=TAxis();fVar[i]=-1;fAxesLabels[i]="";}

}

//____________________________________________________________________________
QnCorrectionsAxes::QnCorrectionsAxes(const QnCorrectionsAxes &c) :
  // Copy constructor
  TObject()
{
  fDim = c.Dim();
  for(Int_t i=0; i<fDim; i++) {
    fVar[i]=c.Var(i);
    fAxes[i]=TAxis(c.Axis(i));
    fAxesLabels[i]=c.AxisLabel(i);
  }
  
}




//_______________________________________________________________________________
QnCorrectionsAxes::~QnCorrectionsAxes()
{
  //
  // De-Constructor
  //
}





//____________________________________________________________________________________
void QnCorrectionsAxes::SetAxis(Int_t axis, Int_t var, TAxis ax, TString label) {

  
  fAxesLabels[axis]=label;
  fVar[axis]=var;
  //fNbins.SetAt(ax.GetNbins(),axis);
  fAxes[axis] = TAxis(ax);

  return;
}

//____________________________________________________________________________________
void QnCorrectionsAxes::SetAxis(Int_t axis, Int_t var, Int_t nwidths, Int_t * nbins, Double_t * edges, TString label) {


  Int_t Nnewbins=0;
  for(Int_t i=0; i<nwidths; i++) Nnewbins+=nbins[i];
  
  Double_t newbins[Nnewbins+1];
  
  newbins[0] = edges[0];
  
  Int_t ibin = 1;
  
  for(Int_t iw=0; iw<nwidths; iw++) {
  	
  	Double_t xwidth = (edges[iw+1]-edges[iw])/nbins[iw];
  
  	for(Int_t ib=(iw==0 ? 1 : 0); ib<(iw==0 ? nbins[iw]+1 : nbins[iw]); ib++){ 
  		newbins[ibin] = newbins[ibin-1] + xwidth;
  		ibin++;
  	
  }}
  
  fAxesLabels[axis]=label;
  fVar[axis]=var;
  TAxis ax = TAxis(ibin-1, newbins);
  fAxes[axis] = TAxis(ax);

  return;
}




//____________________________________________________________________________________
void QnCorrectionsAxes::SetAxis(Int_t axis, Int_t var, Double_t binArray[][2], TString label) {

  fAxesLabels[axis]=label;
  fVar[axis]=var;
  TAxis ax = MakeAxis(binArray);
  fAxes[axis] = TAxis(ax);

  return;
}



//____________________________________________________________________________________
TAxis QnCorrectionsAxes::MakeAxis(Double_t binArray[][2]){


  Int_t Nnewbins=0;
  for(Int_t i=1; i<=(binArray[0][1]-1); i++) Nnewbins+=binArray[i][1];
  
  Double_t newbins[Nnewbins+1];
  
  newbins[0] = binArray[0][0];
  
  Int_t ibin = 1;
  
  for(Int_t iw=0; iw<(binArray[0][1]-1); iw++) {
  	
  	Double_t xwidth = (binArray[iw+1][0]-binArray[iw][0])/binArray[iw+1][1];
  
  	for(Int_t ib=(iw==0 ? 1 : 0); ib<(iw==0 ? binArray[iw+1][1]+1 : binArray[iw+1][1]); ib++){ 
  		newbins[ibin] = newbins[ibin-1] + xwidth;
  		ibin++;
  	
  }}
  
  return TAxis(ibin-1, newbins);

}





//____________________________________________________________________________________
void QnCorrectionsAxes::SetNchannels(Int_t nchannels){
    
  Double_t channelArray[][2] = {{-0.5, 2}, {-0.5+nchannels, nchannels}};

  SetAxis(fDim-1, 0, channelArray, "Channel number");
  
}





