/**************************************************************************
 * Copyright(c) 1998-2010, ALICE Experiment at CERN, All rights reserved. *
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

/* $Id$ */

/////////////////////////////////////////////////////////////
//
// Class for cuts on AOD reconstructed Lc->V0+X
//
// Modified by A.De Caro - decaro@sa.infn.it
//
/////////////////////////////////////////////////////////////

#include <Riostream.h>

#include <TDatabasePDG.h>
#include <TMath.h>

#include "AliAnalysisManager.h"
#include "AliInputEventHandler.h"
#include "AliPIDResponse.h"
#include "AliRDHFCutsLctoV0.h"
#include "AliAODRecoCascadeHF.h"
#include "AliAODTrack.h"
#include "AliESDtrack.h"
#include "AliESDVertex.h"
#include "AliAODVertex.h"
#include "AliAODv0.h"
#include "AliESDv0.h"

using std::cout;
using std::endl;

ClassImp(AliRDHFCutsLctoV0)

//--------------------------------------------------------------------------
  AliRDHFCutsLctoV0::AliRDHFCutsLctoV0(const char* name, Short_t /*v0channel*/) :
  AliRDHFCuts(name),
  fPidSelectionFlag(0),
  fPidHFV0pos(0),
  fPidHFV0neg(0),
  fV0daughtersCuts(0),
  fV0Type(0)
{
  //
  // Default Constructor
  //

  const Int_t nvars=17;
  SetNVars(nvars);
  TString varNames[nvars]={"Lc inv. mass if K0S [GeV/c2]",                   //  0
			   "Lc inv. mass if Lambda [GeV/c2]",                //  1
			   "K0S inv. mass [GeV/c2]",                         //  2
			   "Lambda/LambdaBar inv. mass[GeV/c2]",             //  3
			   "pT min bachelor track [GeV/c]",                  //  4
			   "pT min V0-positive track [GeV/c]",               //  5
			   "pT min V0-negative track [GeV/c]",               //  6
			   "dca cascade (prong-to-prong) cut [cm]",          //  7
			   "dca V0 (prong-to-prong) cut [number of sigmas]", //  8
			   "V0 cosPA min wrt PV",                            //  9
			   "d0 max bachelor wrt PV [cm]",                    // 10
			   "d0 max V0 wrt PV [cm]",                          // 11
			   "mass K0S veto [GeV/c2]",                         // 12
			   "mass Lambda/LambdaBar veto [GeV/c2]",            // 13
			   "mass Gamma veto [GeV/c2]",                       // 14
			   "pT min V0 track [GeV/c]",                        // 15
			   "V0 type"                                         // 16
  };

  Bool_t isUpperCut[nvars]={kTRUE,  //  0
			    kTRUE,  //  1
			    kTRUE,  //  2
			    kTRUE,  //  3
			    kFALSE, //  4
			    kFALSE, //  5
			    kFALSE, //  6
			    kTRUE,  //  7
			    kTRUE,  //  8
			    kFALSE, //  9
			    kTRUE,  // 10
			    kTRUE,  // 11
			    kFALSE, // 12
			    kFALSE, // 13
			    kFALSE, // 14
			    kFALSE, // 15
			    kFALSE  // 16
  };
  SetVarNames(nvars,varNames,isUpperCut);
  Bool_t forOpt[nvars]={kFALSE, //  0
			kFALSE, //  1
			kTRUE,  //  2
			kTRUE,  //  3
			kTRUE,  //  4
			kTRUE,  //  5
			kTRUE,  //  6
			kTRUE,  //  7
			kTRUE,  //  8
			kTRUE,  //  9
			kTRUE,  // 10
			kTRUE,  // 11
			kTRUE,  // 12
			kTRUE,  // 13
			kTRUE,  // 14
			kTRUE,  // 15
			kFALSE  // 16
  };
  SetVarsForOpt(nvars,forOpt);

  Float_t limits[2]={0,999999999.};
  SetPtBins(2,limits);

  /*
  switch (v0channel) {
  case 0:
    fV0channel = 0x0001;
    break;
  case 1:
    fV0channel = 0x0002;
    break;
  case 2:
    fV0channel = 0x0004;
    break;
  }
  */

}
//--------------------------------------------------------------------------
AliRDHFCutsLctoV0::AliRDHFCutsLctoV0(const AliRDHFCutsLctoV0 &source) :
  AliRDHFCuts(source),
  fPidSelectionFlag(source.fPidSelectionFlag),
  fPidHFV0pos(0),
  fPidHFV0neg(0),
  fV0daughtersCuts(0),
  fV0Type(source.fV0Type)
    /*fV0channel(source.fV0channel)*/
{
  //
  // Copy constructor
  //

  if (source.fPidHFV0pos) fPidHFV0pos = new AliAODPidHF(*(source.fPidHFV0pos));
  else fPidHFV0pos = new AliAODPidHF();
  if (source.fPidHFV0neg) fPidHFV0neg = new AliAODPidHF(*(source.fPidHFV0neg));
  else fPidHFV0neg = new AliAODPidHF();

  if (source.fV0daughtersCuts) AddTrackCutsV0daughters(source.fV0daughtersCuts);
  else fV0daughtersCuts = new AliESDtrackCuts();

}
//--------------------------------------------------------------------------
AliRDHFCutsLctoV0 &AliRDHFCutsLctoV0::operator=(const AliRDHFCutsLctoV0 &source)
{
  //
  // assignment operator
  //

  if (this != &source) {

    AliRDHFCuts::operator=(source);
    fPidSelectionFlag = source.fPidSelectionFlag;
    delete fPidHFV0pos;
    fPidHFV0pos = new AliAODPidHF(*(source.fPidHFV0pos));
    delete fPidHFV0neg;
    fPidHFV0neg = new AliAODPidHF(*(source.fPidHFV0neg));

    delete fV0daughtersCuts;
    fV0daughtersCuts = new AliESDtrackCuts(*(source.fV0daughtersCuts));

    fV0Type  = source.fV0Type;

  }

  return *this;
}


//---------------------------------------------------------------------------
AliRDHFCutsLctoV0::~AliRDHFCutsLctoV0() {
 //
 //  Default Destructor
 //

 if (fPidHFV0pos) {
  delete fPidHFV0pos;
  fPidHFV0pos=0;
 }
 if (fPidHFV0neg) {
  delete fPidHFV0neg;
  fPidHFV0neg=0;
 }

 if (fV0daughtersCuts) {
  delete fV0daughtersCuts;
  fV0daughtersCuts=0;
 }

}

//---------------------------------------------------------------------------
void AliRDHFCutsLctoV0::GetCutVarsForOpt(AliAODRecoDecayHF *d,Float_t *vars,Int_t nvars,Int_t *pdgdaughters) {
  //
  // Fills in vars the values of the variables
  //

  if (pdgdaughters[0]==-9999) return; // dummy

  if (nvars!=fnVarsForOpt) {
    printf("AliRDHFCutsLctoV0::GetCutVarsForOpt: wrong number of variables\n");
    return;
  }

  Double_t mLcPDG  = TDatabasePDG::Instance()->GetParticle(4122)->Mass();
  Double_t mk0sPDG = TDatabasePDG::Instance()->GetParticle(310)->Mass();
  Double_t mLPDG   = TDatabasePDG::Instance()->GetParticle(3122)->Mass();

  AliAODRecoCascadeHF *dd = (AliAODRecoCascadeHF*)d;

  // Get the v0 and all daughter tracks
  AliAODTrack *bachelorTrack = (AliAODTrack*)dd->GetBachelor();
  AliAODv0 *v0 = (AliAODv0*)dd->Getv0();
  AliAODTrack *v0positiveTrack = (AliAODTrack*)dd->Getv0PositiveTrack();
  AliAODTrack *v0negativeTrack = (AliAODTrack*)dd->Getv0NegativeTrack();

  Int_t iter=-1;
  // cut on cascade mass, if K0S + p
  if (fVarsForOpt[0]) {
    iter++;
    vars[iter]=TMath::Abs(dd->InvMassLctoK0sP()-mLcPDG);
  }
  // cut on cascade mass, if Lambda/LambdaBar + pi
  if (fVarsForOpt[1]) {
    iter++;
    vars[iter]=TMath::Abs(dd->InvMassLctoLambdaPi()-mLcPDG);
  }

  // cut on V0 mass if K0S
  if (fVarsForOpt[2]) {
    iter++;
    vars[iter]=TMath::Abs(v0->MassK0Short()-mk0sPDG);
  }

  // cut on V0 mass if Lambda/LambdaBar
  if (fVarsForOpt[3]) {

    if (bachelorTrack->Charge()==1) {
      iter++;
      vars[iter]=TMath::Abs(v0->MassLambda()-mLPDG);
    } else if (bachelorTrack->Charge()==-1) {
      iter++;
      vars[iter]=TMath::Abs(v0->MassAntiLambda()-mLPDG);
    }

  }

  // cut bachelor min pt
  if (fVarsForOpt[4]) {
    iter++;
    vars[iter]=bachelorTrack->Pt();
  }

  // cut on V0-positive min pt
  if (fVarsForOpt[5]) {
    iter++;
    vars[iter]=v0positiveTrack->Pt();
  }

  // cut on V0-negative min pt
  if (fVarsForOpt[6]) {
    iter++;
    vars[iter]=v0negativeTrack->Pt();
  }

  // cut on cascade dca (prong-to-prong)
  if (fVarsForOpt[7]) {
    iter++;
    vars[iter]=dd->GetDCA(); // prong-to-prong cascade DCA
  }

  // cut on V0 dca (prong-to-prong)
  if (fVarsForOpt[8]) {
    iter++;
    vars[iter]=v0->GetDCA(); // prong-to-prong V0 DCA
  }

  // cut on V0 cosPA wrt PV
  if (fVarsForOpt[9]) {
    iter++;
    vars[iter]=dd->CosV0PointingAngle(); // cosine of V0 pointing angle wrt primary vertex
  }

  // cut on bachelor transverse impact parameter wrt PV
  if (fVarsForOpt[10]) {
    iter++;
    vars[iter]=dd->Getd0Prong(0); // bachelor transverse impact parameter wrt primary vertex
  }

  // cut on V0 transverse impact parameter wrt PV
  if (fVarsForOpt[11]) {
    iter++;
    vars[iter]=dd->Getd0Prong(1); // V0 transverse impact parameter wrt primary vertex
  }

  // cut on K0S invariant mass veto
  if (fVarsForOpt[12]) {
    iter++;
    vars[iter]=TMath::Abs(v0->MassK0Short()-mk0sPDG); // K0S invariant mass veto
  }

  // cut on Lambda/LambdaBar invariant mass veto
  if (fVarsForOpt[13]) {

    if (bachelorTrack->Charge()==1) {
      iter++;
      vars[iter]=TMath::Abs(v0->MassLambda()-mLPDG);
    } else if (bachelorTrack->Charge()==-1) {
      iter++;
      vars[iter]=TMath::Abs(v0->MassAntiLambda()-mLPDG);
    }

  }

  // cut on gamma invariant mass veto
  if (fVarsForOpt[14]) {
    iter++;
    vars[iter]= v0->InvMass2Prongs(0,1,11,11); // gamma invariant mass veto
  }


  // cut on V0 pT min
  if (fVarsForOpt[15]) {
    iter++;
    vars[iter]= v0->Pt(); // V0 pT min
  }


  return;
}
//---------------------------------------------------------------------------
Int_t AliRDHFCutsLctoV0::IsSelected(TObject* obj,Int_t selectionLevel) {
  //
  // Apply selection
  //

  if (!fCutsRD) {
    AliFatal("Cut matrice not inizialized. Exit...");
    return 0;
  }

  AliAODRecoCascadeHF* d = (AliAODRecoCascadeHF*)obj;
  if (!d) {
    AliDebug(2,"AliAODRecoCascadeHF null");
    return 0;
  }

  if (!d->GetSecondaryVtx()) {
    AliDebug(2,"No secondary vertex for cascade");
    return 0;
  }

  if (d->GetNDaughters()!=2) {
    AliDebug(2,Form("No 2 daughters for current cascade (nDaughters=%d)",d->GetNDaughters()));
    return 0;
  }

  AliAODv0 * v0 = dynamic_cast<AliAODv0*>(d->Getv0());
  AliAODTrack * bachelorTrack = dynamic_cast<AliAODTrack*>(d->GetBachelor());
  if (!v0 || !bachelorTrack) {
    AliDebug(2,"No V0 or no bachelor for current cascade");
    return 0;
  }

  if (bachelorTrack->GetID()<0) {
    AliDebug(2,Form("Bachelor has negative ID %d",bachelorTrack->GetID()));
    return 0;
  }

  if (!v0->GetSecondaryVtx()) {
    AliDebug(2,"No secondary vertex for V0 by cascade");
    return 0;
  }

  if (v0->GetNDaughters()!=2) {
    AliDebug(2,Form("No 2 daughters for V0 of current cascade (onTheFly=%d, nDaughters=%d)",v0->GetOnFlyStatus(),v0->GetNDaughters()));
    return 0;
  }


  // Get the V0 daughter tracks
  AliAODTrack *v0positiveTrack = dynamic_cast<AliAODTrack*>(d->Getv0PositiveTrack());
  AliAODTrack *v0negativeTrack = dynamic_cast<AliAODTrack*>(d->Getv0NegativeTrack());
  if (!v0positiveTrack || !v0negativeTrack ) {
    AliDebug(2,"No V0 daughters' objects");
    return 0;
  }

  if (v0positiveTrack->GetID()<0 || v0negativeTrack->GetID()<0) {
    AliDebug(2,Form("At least one of V0 daughters has negative ID %d %d",v0positiveTrack->GetID(),v0negativeTrack->GetID()));
    return 0;
  }

  //if(fUseTrackSelectionWithFilterBits && d->HasBadDaughters()) return 0;
  if ( fUseTrackSelectionWithFilterBits && !(bachelorTrack->TestFilterMask(BIT(4))) ) {
    AliDebug(2,"Check on the bachelor FilterBit: no BIT(4). Candidate rejected.");
    return 0;
  }

  Int_t returnvalueTrack = 7;

  // selection on daughter tracks
  if (selectionLevel==AliRDHFCuts::kAll ||
      selectionLevel==AliRDHFCuts::kTracks) {

    if (!AreLctoV0DaughtersSelected(d)) return 0;

  }

  Bool_t okLck0sp=kTRUE, okLcLpi=kTRUE, okLcLBarpi=kTRUE;

  // selection on candidate
  if (selectionLevel==AliRDHFCuts::kAll ||
      selectionLevel==AliRDHFCuts::kCandidate) {

    Double_t pt = d->Pt();
    Int_t ptbin = PtBin(pt);

    Double_t mLcPDG  = TDatabasePDG::Instance()->GetParticle(4122)->Mass();
    Double_t mk0sPDG = TDatabasePDG::Instance()->GetParticle(310)->Mass();
    Double_t mLPDG   = TDatabasePDG::Instance()->GetParticle(3122)->Mass();

    // K0S + p
    Double_t mk0s    = v0->MassK0Short();
    Double_t mLck0sp = d->InvMassLctoK0sP();

    // Lambda + pi
    Double_t mlambda  = v0->MassLambda();
    Double_t malambda = v0->MassAntiLambda();
    Double_t mLcLpi   = d->InvMassLctoLambdaPi();

    Bool_t okK0spipi=kTRUE, okLppi=kTRUE, okLBarpip=kTRUE;
    Bool_t isNotK0S = kTRUE, isNotLambda = kTRUE, isNotLambdaBar = kTRUE, isNotGamma = kTRUE;

    // cut on Lc mass with K0S+p hypothesis
    if (TMath::Abs(mLck0sp-mLcPDG) > fCutsRD[GetGlobalIndex(0,ptbin)]) {
      okLck0sp = kFALSE;
      AliDebug(4,Form(" cascade mass is %2.2e and does not correspond to Lambda_c into K0S+p cut",mLck0sp));
    }

    // cuts on the V0 mass: K0S case
    if (TMath::Abs(mk0s-mk0sPDG) > fCutsRD[GetGlobalIndex(2,ptbin)]) {
      okK0spipi = kFALSE;
      AliDebug(4,Form(" V0 mass is %2.2e and does not correspond to K0S cut",mk0s));
    }

    // cut on Lc mass with Lambda+pi hypothesis
    if (TMath::Abs(mLcLpi-mLcPDG) > fCutsRD[GetGlobalIndex(1,ptbin)]) {
      okLcLpi = kFALSE;
      okLcLBarpi = kFALSE;
      AliDebug(4,Form(" cascade mass is %2.2e and does not correspond to Lambda_c into Lambda+pi cut",mLcLpi));
    }

    // cuts on the V0 mass: Lambda/LambdaBar case
    //if ( !(bachelorTrack->Charge()==+1 && TMath::Abs(mlambda-mLPDG) <= fCutsRD[GetGlobalIndex(3,ptbin)] ) ) {
    if ( TMath::Abs(mlambda-mLPDG) > fCutsRD[GetGlobalIndex(3,ptbin)] ) {
      okLppi = kFALSE;
      AliDebug(4,Form(" V0 mass is %2.2e and does not correspond to LambdaBar cut",mlambda));
    }

    //if ( !(bachelorTrack->Charge()==-1 && TMath::Abs(malambda-mLPDG) <= fCutsRD[GetGlobalIndex(3,ptbin)] ) ) {
    if ( TMath::Abs(malambda-mLPDG) > fCutsRD[GetGlobalIndex(3,ptbin)] ) {
      okLBarpip = kFALSE;
      AliDebug(4,Form(" V0 mass is %2.2e and does not correspond to LambdaBar cut",malambda));
    }

    // cut on K0S invariant mass veto
    if (TMath::Abs(v0->MassK0Short()-mk0sPDG) < fCutsRD[GetGlobalIndex(12,ptbin)]) { // K0S invariant mass veto
      AliDebug(4,Form(" veto on K0S invariant mass doesn't pass the cut"));
      //return 0;
      isNotK0S=kFALSE;
    }

    // cut on Lambda/LambdaBar invariant mass veto
    if (TMath::Abs(v0->MassLambda()-mLPDG) < fCutsRD[GetGlobalIndex(13,ptbin)]) { // Lambda invariant mass veto
      AliDebug(4,Form(" veto on Lambda invariant mass doesn't pass the cut"));
      isNotLambda=kFALSE;
      //return 0;
    }
      if (TMath::Abs(v0->MassAntiLambda()-mLPDG) < fCutsRD[GetGlobalIndex(13,ptbin)] ) { // LambdaBar invariant mass veto
      AliDebug(4,Form(" veto on LambdaBar invariant mass doesn't pass the cut"));
      isNotLambdaBar=kFALSE;
      //return 0;
    }

    // cut on gamma invariant mass veto
    if (v0->InvMass2Prongs(0,1,11,11) < fCutsRD[GetGlobalIndex(14,ptbin)]) { // K0S invariant mass veto
      AliDebug(4,Form(" veto on gamma invariant mass doesn't pass the cut"));
      isNotGamma=kFALSE;
      //return 0;
    }

    okLck0sp   = okLck0sp   && okK0spipi && isNotLambda && isNotLambdaBar && isNotGamma;
    okLcLpi    = okLcLpi    && okLppi    && isNotK0S    && isNotLambdaBar && isNotGamma;
    okLcLBarpi = okLcLBarpi && okLBarpip && isNotK0S    && isNotLambda    && isNotGamma;

    if (!okLck0sp && !okLcLpi && !okLcLBarpi) return 0;

    // cuts on the minimum pt of the tracks
    if (TMath::Abs(bachelorTrack->Pt()) < fCutsRD[GetGlobalIndex(4,ptbin)]) {
      AliDebug(4,Form(" bachelor track Pt=%2.2e > %2.2e",bachelorTrack->Pt(),fCutsRD[GetGlobalIndex(4,ptbin)]));
      return 0;
    }
    if (TMath::Abs(v0positiveTrack->Pt()) < fCutsRD[GetGlobalIndex(5,ptbin)]) {
      AliDebug(4,Form(" V0-positive track Pt=%2.2e > %2.2e",v0positiveTrack->Pt(),fCutsRD[GetGlobalIndex(5,ptbin)]));
      return 0;
    }
    if (TMath::Abs(v0negativeTrack->Pt()) < fCutsRD[GetGlobalIndex(6,ptbin)]) {
      AliDebug(4,Form(" V0-negative track Pt=%2.2e > %2.2e",v0negativeTrack->Pt(),fCutsRD[GetGlobalIndex(6,ptbin)]));
      return 0;
    }

    // cut on cascade dca (prong-to-prong)
    if ( TMath::Abs(d->GetDCA()) > fCutsRD[GetGlobalIndex(7,ptbin)] ) { // prong-to-prong cascade DCA
      AliDebug(4,Form(" cascade tracks DCA don't pass the cut"));
      return 0;
    }

    // cut on V0 dca (prong-to-prong)
    if ( TMath::Abs(v0->GetDCA()) > fCutsRD[GetGlobalIndex(8,ptbin)] ) { // prong-to-prong V0 DCA
      AliDebug(4,Form(" V0 DCA don't pass the cut"));
      return 0;
    }

    // cut on V0 cosine of pointing angle wrt PV
    if (d->CosV0PointingAngle() < fCutsRD[GetGlobalIndex(9,ptbin)]) { // cosine of V0 pointing angle wrt primary vertex
      AliDebug(4,Form(" V0 cosine of pointing angle doesn't pass the cut"));
      return 0;
    }

    // cut on bachelor transverse impact parameter wrt PV
    if (TMath::Abs(d->Getd0Prong(0)) > fCutsRD[GetGlobalIndex(10,ptbin)]) { // bachelor transverse impact parameter wrt PV
      AliDebug(4,Form(" bachelor transverse impact parameter doesn't pass the cut"));
      return 0;
    }

    // cut on V0 transverse impact parameter wrt PV
    if (TMath::Abs(d->Getd0Prong(1)) > fCutsRD[GetGlobalIndex(11,ptbin)]) { // V0 transverse impact parameter wrt PV
      AliDebug(4,Form(" V0 transverse impact parameter doesn't pass the cut"));
      return 0;
    }

    // cut on V0 pT min
    if (v0->Pt() < fCutsRD[GetGlobalIndex(15,ptbin)]) { // V0 pT min
      AliDebug(4,Form(" V0 track Pt=%2.2e > %2.2e",v0->Pt(),fCutsRD[GetGlobalIndex(15,ptbin)]));
      return 0;
    }

  }

  Int_t returnvalue = okLck0sp+2*okLcLBarpi+4*okLcLpi;
  /*
    retvalue case
          1  Lc->K0S + p
          2  Lc->LambdaBar + pi
          3  Lc->K0S + p AND Lc->LambdaBar + pi
          4  Lc->Lambda + pi
          5  Lc->K0S + p AND Lc->Lambda + pi
          6  Lc->LambdaBar + pi AND Lc->Lambda + pi
          7  Lc->K0S + p AND Lc->LambdaBar + pi AND Lc->Lambda + pi
  */

  Int_t returnvaluePID = 7;

  // selection on candidate
  if (selectionLevel==AliRDHFCuts::kAll ||
      selectionLevel==AliRDHFCuts::kCandidate ||
      selectionLevel==AliRDHFCuts::kPID )
    returnvaluePID = IsSelectedPID(d);

  //if (fUsePID && returnvaluePID==0) return 0;

  Int_t returnvalueTot = 0;
  if ( fUsePID )
    returnvalueTot = CombineCuts(returnvalueTrack,returnvalue,returnvaluePID);
  else
    returnvalueTot = CombineCuts(returnvalueTrack,returnvalue,7);

  return returnvalueTot;

}
//---------------------------------------------------------------------------
Int_t AliRDHFCutsLctoV0::IsSelectedPID(AliAODRecoDecayHF* obj) {

  // fPidHF -> PID object for bachelor
  // fPidHFV0pos -> PID object for positive V0 daughter
  // fPidHFV0neg -> PID object for negative V0 daughter

  if (!fUsePID || !obj) {
    AliDebug(2,"PID selection inactive. Candidate accepted.");
    return 7; // all hypothesis are valid
  }

  if (fPidHF->GetPidResponse()==0x0 ||
      fPidHFV0pos->GetPidResponse()==0x0 ||
      fPidHFV0neg->GetPidResponse()==0x0) {
    AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
    AliInputEventHandler *inputHandler=(AliInputEventHandler*)mgr->GetInputEventHandler();
    AliPIDResponse *pidResp=inputHandler->GetPIDResponse();
    fPidHF->SetPidResponse(pidResp);
    fPidHFV0pos->SetPidResponse(pidResp);
    fPidHFV0neg->SetPidResponse(pidResp);
    fPidHF->SetOldPid(kFALSE);
    fPidHFV0pos->SetOldPid(kFALSE);
    fPidHFV0neg->SetOldPid(kFALSE);
  }

  AliAODRecoCascadeHF *objD = (AliAODRecoCascadeHF*)obj;

  Bool_t isPeriodd = fPidHF->GetOnePad();
  Bool_t isMC = fPidHF->GetMC();

  if (isPeriodd) {
    fPidHFV0pos->SetOnePad(kTRUE);
    fPidHFV0neg->SetOnePad(kTRUE);
  }
  if (isMC) {
    fPidHFV0neg->SetMC(kTRUE);
    fPidHFV0pos->SetMC(kTRUE);
  }

  AliAODTrack *bachelor = (AliAODTrack*)objD->GetBachelor();
  AliAODTrack *v0Pos = (AliAODTrack*)objD->Getv0PositiveTrack();
  AliAODTrack *v0Neg = (AliAODTrack*)objD->Getv0NegativeTrack();

  if (!bachelor || !v0Pos || !v0Neg) return 0;

  Bool_t okLcK0Sp = kTRUE; // K0S case
  Bool_t okLcLambdaBarPi = kTRUE; // LambdaBar case
  Bool_t okLcLambdaPi = kTRUE; // Lambda case

  CheckPID(bachelor,v0Neg,v0Pos,okLcK0Sp,okLcLambdaBarPi,okLcLambdaPi);

  Int_t returnvalue = okLcK0Sp+2*okLcLambdaBarPi+4*okLcLambdaPi;

  return returnvalue;
}
//-----------------------
void AliRDHFCutsLctoV0::CheckPID(AliAODTrack *bachelor, AliAODTrack *v0Neg, AliAODTrack *v0Pos,
				 Bool_t &isBachelorID1, Bool_t &isV0NegID2, Bool_t &isV0PosID4) {
  // identification strategy

  Int_t trackIDtof = -1;
  Int_t trackIDtpc = -1;

  Bool_t dummy1 = kFALSE;
  Bool_t dummy2 = kFALSE;
  Bool_t dummy4 = kFALSE;

  Int_t tpcID = -1;
  Int_t tofID = -1;
  Double_t nTPCsigmasPr = -999;
  Double_t nTOFsigmasPr = -999;

  Bool_t trackIDtofB = -1;
  Bool_t trackIDtpcB = -1;

  switch (fPidSelectionFlag) {

  case 0:

    // identify bachelor
    trackIDtof = fPidHF->ApplyPidTOFRaw(bachelor,4);
    trackIDtpc = fPidHF->ApplyPidTPCRaw(bachelor,4);
    AliDebug(1,Form(" fPidHF->ApplyPidTOFRaw(bachelor,4)=%d fPidHF->ApplyPidTPCRaw(bachelor,4)=%d",trackIDtof,trackIDtpc));
    isBachelorID1 = (trackIDtof==4) && (trackIDtpc==4); // K0S case
    //isBachelorID2 = (fPidHF->ApplyPidTPCRaw(bachelor,2)==2) && (fPidHF->ApplyPidTOFRaw(bachelor,2)==2); // LambdaBar case
    //isBachelorID4 = isBachelorID2; // Lambda case

    // identify V0neg
    trackIDtof = fPidHFV0neg->ApplyPidTOFRaw(v0Neg,4);
    trackIDtpc = fPidHFV0neg->ApplyPidTPCRaw(v0Neg,4);
    AliDebug(1,Form(" fPidHFV0neg->ApplyPidTOFRaw(v0Neg,4)=%d fPidHFV0neg->ApplyPidTPCRaw(v0Neg,4)=%d",trackIDtof,trackIDtpc));
    //isV0NegID1 = (fPidHFV0neg->ApplyPidTPCRaw(v0Neg,2)==2) && (fPidHFV0neg->ApplyPidTOFRaw(v0Neg,2)==2); // K0S case
    isV0NegID2 = (trackIDtof==4) && (trackIDtpc==4); // LambdaBar case
    //isV0NegID4 = isV0NegID1; // Lambda case

    // identify V0pos
    trackIDtof = fPidHFV0pos->ApplyPidTOFRaw(v0Pos,4);
    trackIDtpc = fPidHFV0pos->ApplyPidTPCRaw(v0Pos,4);
    AliDebug(1,Form(" fPidHFV0pos->ApplyPidTOFRaw(v0Pos,4)=%d fPidHFV0pos->ApplyPidTPCRaw(v0POS,4)=%d",trackIDtof,trackIDtpc));
    //isV0PosID1 = (fPidHFV0pos->ApplyPidTPCRaw(v0Pos,2)==2) && (fPidHFV0pos->ApplyPidTOFRaw(v0Pos,2)==2); // K0S case
    //isV0PosID2 = isV0PosID1; // LambdaBar case
    isV0PosID4 = (trackIDtof==4) && (trackIDtpc==4); // Lambda case

    break;
  case 1:

    // identify bachelor
    trackIDtof = fPidHF->ApplyPidTOFRaw(bachelor,4);
    trackIDtpc = fPidHF->ApplyPidTPCRaw(bachelor,4);
    AliDebug(1,Form(" fPidHF->ApplyPidTOFRaw(bachelor,4)=%d fPidHFV0->ApplyPidTPCRaw(bachelor,4)=%d",trackIDtof,trackIDtpc));
    isBachelorID1 = ( trackIDtof==4 );
    dummy1 = ( !(fPidHF->CheckTOFPIDStatus(bachelor)) && (trackIDtpc==4) &&
	       fPidHF->IsExcluded(bachelor,2,2.,"TPC") && fPidHF->IsExcluded(bachelor,3,2.,"TPC") ); // K0S case
    isBachelorID1 = isBachelorID1 || dummy1;


    // identify V0neg
    trackIDtof = fPidHFV0neg->ApplyPidTOFRaw(v0Neg,4);
    trackIDtpc = fPidHFV0neg->ApplyPidTPCRaw(v0Neg,4);
    AliDebug(1,Form(" fPidHFV0neg->ApplyPidTOFRaw(v0Neg,4)=%d fPidHFV0neg->ApplyPidTPCRaw(v0Neg,4)=%d",trackIDtof,trackIDtpc));
    isV0NegID2    = ( trackIDtof==4 );
    dummy2 = ( !(fPidHFV0neg->CheckTOFPIDStatus(v0Neg)) && (trackIDtpc==4) &&
	       fPidHFV0neg->IsExcluded(v0Neg,2,2.,"TPC") && fPidHFV0neg->IsExcluded(v0Neg,3,2.,"TPC") ); // LambdaBar case
    isV0NegID2 = isV0NegID2 || dummy2;


    // identify V0pos
    trackIDtof = fPidHFV0pos->ApplyPidTOFRaw(v0Pos,4);
    trackIDtpc = fPidHFV0pos->ApplyPidTPCRaw(v0Pos,4);
    AliDebug(1,Form(" fPidHFV0pos->ApplyPidTOFRaw(v0Pos,4)=%d fPidHFV0pos->ApplyPidTPCRaw(v0Pos,4)=%d",trackIDtof,trackIDtpc));
    isV0PosID4    = ( trackIDtof==4 );
    dummy4 = ( !(fPidHFV0pos->CheckTOFPIDStatus(v0Pos)) && (trackIDtpc==4) &&
	       fPidHFV0pos->IsExcluded(v0Pos,2,2.,"TPC") && fPidHFV0pos->IsExcluded(v0Pos,3,2.,"TPC") ); // Lambda case
    isV0PosID4 = isV0PosID4 || dummy4;


    break;
  case 2:

    // identify bachelor
    nTOFsigmasPr = -999;
    tofID = fPidHF->GetnSigmaTOF(bachelor,4,nTOFsigmasPr);
    nTPCsigmasPr = -999;
    tpcID = fPidHF->GetnSigmaTPC(bachelor,4,nTPCsigmasPr);
    trackIDtofB = (tofID==1) && ( (bachelor->P()>=1.0 && bachelor->P()<2.5 && TMath::Abs(nTOFsigmasPr)<3) ||
				  (bachelor->P()>=2.5 && nTOFsigmasPr>-2 && nTOFsigmasPr<3) );
    trackIDtpcB = (tpcID==1) && ( (bachelor->P()<1.0 && TMath::Abs(nTPCsigmasPr)<2) ||
				  (bachelor->P()>=1.0 && TMath::Abs(nTPCsigmasPr)<3) );
    AliDebug(1,Form(" trackIDtofB=%d trackIDtpcB=%d",trackIDtofB,trackIDtpcB));
    isBachelorID1 = (bachelor->P()<1 && trackIDtpcB) || (bachelor->P()>=1 && trackIDtpcB && trackIDtofB); // K0S case

    // identify V0neg
    nTOFsigmasPr = -999;
    tofID = fPidHFV0neg->GetnSigmaTOF(v0Neg,4,nTOFsigmasPr);
    nTPCsigmasPr = -999;
    tpcID = fPidHFV0neg->GetnSigmaTPC(v0Neg,4,nTPCsigmasPr);
    trackIDtofB = (tofID==1) && ( (v0Neg->P()>=1.0 && v0Neg->P()<2.5 && TMath::Abs(nTOFsigmasPr)<3) ||
				  (v0Neg->P()>=2.5 && nTOFsigmasPr>-2 && nTOFsigmasPr<3) );
    trackIDtpcB = (tpcID==1) && ( (v0Neg->P()<1.0 && TMath::Abs(nTPCsigmasPr)<2) ||
				  (v0Neg->P()>=1.0 && TMath::Abs(nTPCsigmasPr)<3) );
    AliDebug(1,Form(" trackIDtofB=%d trackIDtpcB=%d",trackIDtofB,trackIDtpcB));
    isV0NegID2 = (v0Neg->P()<1 && trackIDtpcB) || (v0Neg->P()>=1 && trackIDtpcB && trackIDtofB); // LambdaBar case
    
    // identify V0pos
    nTOFsigmasPr = -999;
    tofID = fPidHFV0pos->GetnSigmaTOF(v0Pos,4,nTOFsigmasPr);
    nTPCsigmasPr = -999;
    tpcID = fPidHFV0pos->GetnSigmaTPC(v0Pos,4,nTPCsigmasPr);
    trackIDtofB = (tofID==1) && ( (v0Pos->P()>=1.0 && v0Pos->P()<2.5 && TMath::Abs(nTOFsigmasPr)<3) ||
				  (v0Pos->P()>=2.5 && nTOFsigmasPr>-2 && nTOFsigmasPr<3) );
    trackIDtpcB = (tpcID==1) && ( (v0Pos->P()<1.0 && TMath::Abs(nTPCsigmasPr)<2) ||
				  (v0Pos->P()>=1.0 && TMath::Abs(nTPCsigmasPr)<3) );
    AliDebug(1,Form(" trackIDtofB=%d trackIDtpcB=%d",trackIDtofB,trackIDtpcB));
    isV0PosID4 = (v0Pos->P()<1 && trackIDtpcB) || (v0Pos->P()>=1 && trackIDtpcB && trackIDtofB); // Lambda case

    break;
  case 3:

    // identify bachelor
    nTOFsigmasPr = -999;
    tofID = fPidHF->GetnSigmaTOF(bachelor,4,nTOFsigmasPr);
    nTPCsigmasPr = -999;
    tpcID = fPidHF->GetnSigmaTPC(bachelor,4,nTPCsigmasPr);
    trackIDtofB = (tofID==1) && ( (bachelor->P()>=1.0 && bachelor->P()<2.5 && TMath::Abs(nTOFsigmasPr)<3) ||
				  (bachelor->P()>=2.5 && nTOFsigmasPr>-2 && nTOFsigmasPr<3) );
    trackIDtpcB = (tpcID==1) && ( (bachelor->P()<1.0 && TMath::Abs(nTPCsigmasPr)<2) ||
				  (bachelor->P()>=1.0 && bachelor->P()<3.0 && TMath::Abs(nTPCsigmasPr)<3) ||
				  (bachelor->P()>=3.0 && nTPCsigmasPr>-3 && nTPCsigmasPr<2) );
    AliDebug(1,Form(" trackIDtofB=%d trackIDtpcB=%d",trackIDtofB,trackIDtpcB));
    isBachelorID1 = (bachelor->P()<1 && trackIDtpcB) || (bachelor->P()>=1 && trackIDtpcB && trackIDtofB); // K0S case

    // identify V0neg
    nTOFsigmasPr = -999;
    tofID = fPidHFV0neg->GetnSigmaTOF(v0Neg,4,nTOFsigmasPr);
    nTPCsigmasPr = -999;
    tpcID = fPidHFV0neg->GetnSigmaTPC(v0Neg,4,nTPCsigmasPr);
    trackIDtofB = (tofID==1) && ( (v0Neg->P()>=1.0 && v0Neg->P()<2.5 && TMath::Abs(nTOFsigmasPr)<3) ||
				  (v0Neg->P()>=2.5 && nTOFsigmasPr>-2 && nTOFsigmasPr<3) );
    trackIDtpcB = (tpcID==1) && ( (v0Neg->P()<1.0 && TMath::Abs(nTPCsigmasPr)<2) ||
				  (v0Neg->P()>=1.0 && v0Neg->P()<3.0 && TMath::Abs(nTPCsigmasPr)<3) ||
				  (v0Neg->P()>=3.0 && nTPCsigmasPr>-3 && nTPCsigmasPr<2) );
    AliDebug(1,Form(" trackIDtofB=%d trackIDtpcB=%d",trackIDtofB,trackIDtpcB));
    isV0NegID2 = (v0Neg->P()<1 && trackIDtpcB) || (v0Neg->P()>=1 && trackIDtpcB && trackIDtofB); // LambdaBar case

    // identify V0pos
    nTOFsigmasPr = -999;
    tofID = fPidHFV0pos->GetnSigmaTOF(v0Pos,4,nTOFsigmasPr);
    nTPCsigmasPr = -999;
    tpcID = fPidHFV0pos->GetnSigmaTPC(v0Pos,4,nTPCsigmasPr);
    trackIDtofB = (tofID==1) && ( (v0Pos->P()>=1.0 && v0Pos->P()<2.5 && TMath::Abs(nTOFsigmasPr)<3) ||
				  (v0Pos->P()>=2.5 && nTOFsigmasPr>-2 && nTOFsigmasPr<3) );
    trackIDtpcB = (tpcID==1) && ( (v0Pos->P()<1.0 && TMath::Abs(nTPCsigmasPr)<2) ||
				  (v0Pos->P()>=1.0 && v0Pos->P()<3.0 && TMath::Abs(nTPCsigmasPr)<3) ||
				  (v0Pos->P()>=3.0 && nTPCsigmasPr>-3 && nTPCsigmasPr<2) );
    AliDebug(1,Form(" trackIDtofB=%d trackIDtpcB=%d",trackIDtofB,trackIDtpcB));
    isV0PosID4 = (v0Pos->P()<1 && trackIDtpcB) || (v0Pos->P()>=1 && trackIDtpcB && trackIDtofB); // Lambda case

    break;

  }

}
//----------------
Int_t AliRDHFCutsLctoV0::CombineCuts(Int_t returnvalueTrack, Int_t returnvalue, Int_t returnvaluePID) const {
  //
  // combine track selection, topological cuts and PID
  //

 Int_t returnvalueTot=returnvalueTrack&returnvalue;
 returnvalueTot=returnvalueTot&returnvaluePID;

 return returnvalueTot;
}

//----------------------------------
Int_t AliRDHFCutsLctoV0::IsSelectedSingleCut(TObject* obj, Int_t selectionLevel, Int_t cutIndex) {
  //
  // Apply selection on single cut
  //

  if (!fCutsRD) {
    AliDebug(2,"Cut matrice not inizialized. Exit...");
    return 0;
  }

  AliAODRecoCascadeHF* d = (AliAODRecoCascadeHF*)obj;
  if (!d) {
    AliDebug(2,"AliAODRecoCascadeHF null");
    return 0;
  }

  if (!d->GetSecondaryVtx()) {
    AliDebug(2,"No secondary vertex for cascade");
    return 0;
  }

  if (d->GetNDaughters()!=2) {
    AliDebug(2,Form("No 2 daughters for current cascade (nDaughters=%d)",d->GetNDaughters()));
    return 0;
  }

  AliAODv0 * v0 = dynamic_cast<AliAODv0*>(d->Getv0());
  AliAODTrack * bachelorTrack = dynamic_cast<AliAODTrack*>(d->GetBachelor());
  if (!v0 || !bachelorTrack) {
    AliDebug(2,"No V0 or no bachelor for current cascade");
    return 0;
  }

  if (bachelorTrack->GetID()<0) {
    AliDebug(2,Form("Bachelor has negative ID %d",bachelorTrack->GetID()));
    return 0;
  }

  if (!v0->GetSecondaryVtx()) {
    AliDebug(2,"No secondary vertex for V0 by cascade");
    return 0;
  }

  if (v0->GetNDaughters()!=2) {
    AliDebug(2,Form("No 2 daughters for V0 of current cascade (onTheFly=%d, nDaughters=%d)",v0->GetOnFlyStatus(),v0->GetNDaughters()));
    return 0;
  }


  // Get the V0 daughter tracks
  AliAODTrack *v0positiveTrack = dynamic_cast<AliAODTrack*>(d->Getv0PositiveTrack());
  AliAODTrack *v0negativeTrack = dynamic_cast<AliAODTrack*>(d->Getv0NegativeTrack());
  if (!v0positiveTrack || !v0negativeTrack ) {
    AliDebug(2,"No V0 daughters' objects");
    return 0;
  }

  if (v0positiveTrack->GetID()<0 || v0negativeTrack->GetID()<0) {
    AliDebug(2,Form("At least one of V0 daughters has negative ID %d %d",v0positiveTrack->GetID(),v0negativeTrack->GetID()));
    return 0;
  }

  //if(fUseTrackSelectionWithFilterBits && d->HasBadDaughters()) return 0;
  if ( fUseTrackSelectionWithFilterBits && !(bachelorTrack->TestFilterMask(BIT(4))) ) {
    AliDebug(2,"Check on the bachelor FilterBit: no BIT(4). Candidate rejected.");
    return 0;
  }


  // selection on daughter tracks
  if (selectionLevel==AliRDHFCuts::kAll ||
      selectionLevel==AliRDHFCuts::kTracks) {

    if (!AreLctoV0DaughtersSelected(d)) return 0;

  }

  Bool_t okLck0sp=kFALSE, okLcLpi=kFALSE, okLcLBarpi=kFALSE;

  // selection on candidate
  if (selectionLevel==AliRDHFCuts::kAll ||
      selectionLevel==AliRDHFCuts::kCandidate) {

    Double_t pt = d->Pt();
    Int_t ptbin = PtBin(pt);

    Double_t mLcPDG  = TDatabasePDG::Instance()->GetParticle(4122)->Mass();
    Double_t mk0sPDG = TDatabasePDG::Instance()->GetParticle(310)->Mass();
    Double_t mLPDG   = TDatabasePDG::Instance()->GetParticle(3122)->Mass();

    // K0S + p
    Double_t mk0s    = v0->MassK0Short();
    Double_t mLck0sp = d->InvMassLctoK0sP();

    // Lambda + pi
    Double_t mlambda  = v0->MassLambda();
    Double_t malambda = v0->MassAntiLambda();
    Double_t mLcLpi   = d->InvMassLctoLambdaPi();

    switch (cutIndex) {
    case 0:
      // cut on Lc mass with K0S+p hypothesis
      okLck0sp   = TMath::Abs(mLck0sp-mLcPDG)<=fCutsRD[GetGlobalIndex(0,ptbin)];
      okLcLpi    = kFALSE;
      okLcLBarpi = kFALSE;
      break;
    case 1:
      // cut on Lc mass with Lambda+pi hypothesis
      okLck0sp   = kFALSE;
      okLcLpi    = TMath::Abs(mLcLpi-mLcPDG)<=fCutsRD[GetGlobalIndex(1,ptbin)];
      okLcLBarpi = okLcLpi;
      break;
    case 2:
      // cuts on the V0 mass: K0S case
      okLck0sp   = TMath::Abs(mk0s-mk0sPDG)<=fCutsRD[GetGlobalIndex(2,ptbin)];
      okLcLpi    = kFALSE;
      okLcLBarpi = kFALSE;
      break;
    case 3:
      // cuts on the V0 mass: Lambda/LambdaBar case
      okLck0sp   = kFALSE;
      okLcLpi    = TMath::Abs(mlambda-mLPDG)<=fCutsRD[GetGlobalIndex(3,ptbin)];
      //okLcLpi    = okLcLpi && (bachelorTrack->Charge()==+1);
      okLcLBarpi = TMath::Abs(malambda-mLPDG)<=fCutsRD[GetGlobalIndex(3,ptbin)];
      //okLcLBarpi = okLcLBarpi && (bachelorTrack->Charge()==-1);
      break;
    case 4:
      // cuts on the minimum pt of bachelor
      okLck0sp   = TMath::Abs(bachelorTrack->Pt())>=fCutsRD[GetGlobalIndex(4,ptbin)];
      okLcLpi    = okLck0sp;
      okLcLBarpi = okLck0sp;
      break;
    case 5:
      // cuts on the minimum pt of positive V0daughter
      okLck0sp   = TMath::Abs(v0positiveTrack->Pt())>=fCutsRD[GetGlobalIndex(5,ptbin)];
      okLcLpi    = okLck0sp;
      okLcLBarpi = okLck0sp;
      break;
    case 6:
      // cuts on the minimum pt of negative V0daughter
      okLck0sp   = TMath::Abs(v0negativeTrack->Pt())>=fCutsRD[GetGlobalIndex(6,ptbin)];
      okLcLpi    = okLck0sp;
      okLcLBarpi = okLck0sp;
      break;
    case 7:
      // cut on cascade dca
      okLck0sp   = TMath::Abs(d->GetDCA())<=fCutsRD[GetGlobalIndex(7,ptbin)];
      okLcLpi    = okLck0sp;
      okLcLBarpi = okLck0sp;
      break;
    case 8:
      // cut on V0 dca
      okLck0sp   = TMath::Abs(v0->GetDCA())<=fCutsRD[GetGlobalIndex(8,ptbin)];
      okLcLpi    = okLck0sp;
      okLcLBarpi = okLck0sp;
      break;
    case 9:
      // cut on V0 cosine of pointing angle wrt PV
      okLck0sp   = d->CosV0PointingAngle()>=fCutsRD[GetGlobalIndex(9,ptbin)];
      okLcLpi    = okLck0sp;
      okLcLBarpi = okLck0sp;
      break;
    case 10:
      // cut on bachelor transverse impact parameter wrt PV
      okLck0sp   = TMath::Abs(d->Getd0Prong(0))<=fCutsRD[GetGlobalIndex(10,ptbin)];
      okLcLpi    = okLck0sp;
      okLcLBarpi = okLck0sp;
      break;
    case 11:
      // cut on V0 transverse impact parameter wrt PV
      okLck0sp   = TMath::Abs(d->Getd0Prong(1))<=fCutsRD[GetGlobalIndex(11,ptbin)];
      okLcLpi    = okLck0sp;
      okLcLBarpi = okLck0sp;
      break;
    case 12:
      // cut on K0S invariant mass veto
      okLcLpi    = TMath::Abs(mk0s-mk0sPDG)>=fCutsRD[GetGlobalIndex(12,ptbin)];
      okLcLBarpi = TMath::Abs(mk0s-mk0sPDG)>=fCutsRD[GetGlobalIndex(12,ptbin)];
      break;
    case 13:
      // cut on Lambda/LambdaBar invariant mass veto
      okLck0sp   = (TMath::Abs(mlambda-mLPDG)>=fCutsRD[GetGlobalIndex(13,ptbin)] &&
		    TMath::Abs(malambda-mLPDG)>=fCutsRD[GetGlobalIndex(13,ptbin)]);
      break;
    case 14:
      // cut on gamma invariant mass veto
      okLck0sp   = v0->InvMass2Prongs(0,1,11,11)>=fCutsRD[GetGlobalIndex(14,ptbin)];
      okLcLpi    = okLck0sp;
      okLcLBarpi = okLck0sp;
      break;
    case 15:
      // cut on V0 pT min
      okLck0sp   = v0->Pt()>=fCutsRD[GetGlobalIndex(15,ptbin)];
      okLcLpi    = okLck0sp;
      okLcLBarpi = okLck0sp;
      break;
    }
  }

  Int_t returnvalue = okLck0sp+2*okLcLBarpi+4*okLcLpi;
  /*
    retvalue case
          1  Lc->K0S + p
          2  Lc->LambdaBar + pi
          3  Lc->K0S + p AND Lc->LambdaBar + pi
          4  Lc->Lambda + pi
          5  Lc->K0S + p AND Lc->Lambda + pi
          6  Lc->LambdaBar + pi AND Lc->Lambda + pi
          7  Lc->K0S + p AND Lc->LambdaBar + pi AND Lc->Lambda + pi
  */


  /*
  Int_t returnvaluePID = 7;

  // selection on PID
  if (selectionLevel==AliRDHFCuts::kAll ||
      selectionLevel==AliRDHFCuts::kCandidate ||
      selectionLevel==AliRDHFCuts::kPID )
    returnvaluePID = IsSelectedPID(d);
  */

  Int_t returnvalueTot = 0;
  //if ( fUsePID )
  //returnvalueTot = CombineCuts(returnvalue,returnvaluePID);
  //else
  returnvalueTot = returnvalue;

  return returnvalueTot;

}
//----------------------------------
void AliRDHFCutsLctoV0::SetStandardCutsPP2010() {

 SetName("LctoV0ProductionCuts");
 SetTitle("Production cuts for Lc->V0+bachelor analysis");

  AliESDtrackCuts* esdTrackCuts=new AliESDtrackCuts();
  esdTrackCuts->SetRequireSigmaToVertex(kFALSE);
  //default
  esdTrackCuts->SetRequireTPCRefit(kTRUE);
  esdTrackCuts->SetRequireITSRefit(kTRUE);
  esdTrackCuts->SetMinNClustersITS(0);//(4); // default is 5
  esdTrackCuts->SetMinNClustersTPC(70);
  //esdTrackCuts->SetClusterRequirementITS(AliESDtrackCuts::kSPD,
  //				           AliESDtrackCuts::kAny);
  // default is kBoth, otherwise kAny
  esdTrackCuts->SetMinDCAToVertexXY(0.);
  esdTrackCuts->SetPtRange(0.3,1.e10);
  //esdTrackCuts->SetEtaRange(-0.8,+0.8);
  esdTrackCuts->SetAcceptKinkDaughters(kFALSE);
  AddTrackCuts(esdTrackCuts);


  AliESDtrackCuts* esdTrackCutsV0daughters=new AliESDtrackCuts();
  esdTrackCutsV0daughters->SetRequireSigmaToVertex(kFALSE);
  //default
  esdTrackCutsV0daughters->SetRequireTPCRefit(kTRUE);
  esdTrackCutsV0daughters->SetRequireITSRefit(kFALSE);//(kTRUE);
  esdTrackCutsV0daughters->SetMinNClustersITS(0);//(4); // default is 5
  esdTrackCutsV0daughters->SetMinNClustersTPC(70);
  //esdTrackCutsV0daughters->SetClusterRequirementITS(AliESDtrackCuts::kSPD,
  //					              AliESDtrackCuts::kAny);
  // default is kBoth, otherwise kAny
  esdTrackCutsV0daughters->SetMinDCAToVertexXY(0.);
  esdTrackCutsV0daughters->SetPtRange(0.,1.e10);
  esdTrackCutsV0daughters->SetAcceptKinkDaughters(kFALSE);
  AddTrackCutsV0daughters(esdTrackCutsV0daughters);

  const Int_t nptbins=1;
  Float_t* ptbins;
  ptbins=new Float_t[nptbins+1];
  ptbins[0]=0.;
  ptbins[1]=99999999.;

  SetPtBins(nptbins+1,ptbins);
  SetPtBins(nptbins+1,ptbins);

  const Int_t nvars=17;

  Float_t** prodcutsval;
  prodcutsval=new Float_t*[nvars];
  for(Int_t ic=0;ic<nvars;ic++){prodcutsval[ic]=new Float_t[nptbins];}
  for(Int_t ipt2=0;ipt2<nptbins;ipt2++){
   prodcutsval[0][ipt2]=1.;    // inv. mass if K0S [GeV/c2]
   prodcutsval[1][ipt2]=1.;    // inv. mass if Lambda [GeV/c2]
   prodcutsval[2][ipt2]=0.05;  // inv. mass V0 if K0S [GeV/c2]
   prodcutsval[3][ipt2]=0.05;  // inv. mass V0 if Lambda [GeV/c2]
   prodcutsval[4][ipt2]=0.3;   // pT min bachelor track [GeV/c] // AOD by construction
   prodcutsval[5][ipt2]=0.;    // pT min V0-positive track [GeV/c]
   prodcutsval[6][ipt2]=0.;    // pT min V0-negative track [GeV/c]
   prodcutsval[7][ipt2]=1000.; // dca cascade cut [cm]
   prodcutsval[8][ipt2]=1.5;   // dca V0 cut [nSigma] // it's 1.5 x offline V0s
   prodcutsval[9][ipt2]=-1.;   // cosPA V0 cut // it's 0.90 x offline V0s at reconstruction level, 0.99 at filtering level
   prodcutsval[10][ipt2]=3.;   // d0 max bachelor wrt PV [cm]
   prodcutsval[11][ipt2]=1000.;// d0 max V0 wrt PV [cm]
   prodcutsval[12][ipt2]=0.;   // mass K0S veto [GeV/c2]
   prodcutsval[13][ipt2]=0.;   // mass Lambda/LambdaBar veto [GeV/c2]
   prodcutsval[14][ipt2]=0.;   // mass Gamma veto [GeV/c2]
   prodcutsval[15][ipt2]=0.;   // pT min V0 track [GeV/c]
   prodcutsval[16][ipt2]=0.;   // V0 type cut
  }
  SetCuts(nvars,nptbins,prodcutsval);

  SetGlobalIndex(nvars,nptbins);
  SetPtBins(nptbins+1,ptbins);


  //pid settings
  //1. bachelor: default one
  AliAODPidHF* pidObjBachelor = new AliAODPidHF();
  Double_t sigmasBac[5]={3.,1.,1.,3.,3.}; // 0, 1(A), 2(A) -> TPC; 3 -> TOF; 4 -> ITS
  pidObjBachelor->SetSigma(sigmasBac);
  pidObjBachelor->SetAsym(kFALSE);
  pidObjBachelor->SetMatch(1);
  pidObjBachelor->SetTPC(kTRUE);
  pidObjBachelor->SetTOF(kTRUE);
  pidObjBachelor->SetTOFdecide(kFALSE);
  SetPidHF(pidObjBachelor);

  //2. V0pos
  AliAODPidHF* pidObjV0pos = new AliAODPidHF();
  Double_t sigmasV0pos[5]={3.,1.,1.,3.,3.}; // 0, 1(A), 2(A) -> TPC; 3 -> TOF; 4 -> ITS
  pidObjV0pos->SetSigma(sigmasV0pos);
  pidObjV0pos->SetAsym(kFALSE);
  pidObjV0pos->SetMatch(1);
  pidObjV0pos->SetTPC(kTRUE);
  pidObjV0pos->SetTOF(kTRUE);
  pidObjV0pos->SetTOFdecide(kFALSE);
  SetPidV0pos(pidObjV0pos);

  //2. V0neg
  AliAODPidHF* pidObjV0neg = new AliAODPidHF();
  Double_t sigmasV0neg[5]={3.,1.,1.,3.,3.}; // 0, 1(A), 2(A) -> TPC; 3 -> TOF; 4 -> ITS
  pidObjV0neg->SetSigma(sigmasV0neg);
  pidObjV0neg->SetAsym(kFALSE);
  pidObjV0neg->SetMatch(1);
  pidObjV0neg->SetTPC(kTRUE);
  pidObjV0neg->SetTOF(kTRUE);
  pidObjV0neg->SetTOFdecide(kFALSE);
  SetPidV0neg(pidObjV0neg);

  SetUsePID(kFALSE);//(kTRUE);

  //PrintAll();

 for(Int_t iiv=0;iiv<nvars;iiv++){
  delete [] prodcutsval[iiv];
 }
 delete [] prodcutsval;
 prodcutsval=NULL;
 delete [] ptbins;
 ptbins=NULL;


 delete pidObjBachelor;
 pidObjBachelor=NULL;

 delete pidObjV0pos;
 pidObjV0pos=NULL;

 delete pidObjV0neg;
 pidObjV0neg=NULL;

 return;
}
//------------------
void AliRDHFCutsLctoV0::SetStandardCutsPbPb2010() {

 SetName("LctoV0ProductionCuts");
 SetTitle("Production cuts for Lc->V0+bachelor analysis");

 SetStandardCutsPP2010();

 return;
}
//------------------
void AliRDHFCutsLctoV0::SetStandardCutsPbPb2011() {

  // Default 2010 PbPb cut object
  SetStandardCutsPbPb2010();

  //
  // Enable all 2011 PbPb run triggers
  //
  SetTriggerClass("");
  ResetMaskAndEnableMBTrigger();
  EnableCentralTrigger();
  EnableSemiCentralTrigger();

}
//-----------------------
Int_t AliRDHFCutsLctoV0::GetV0Type(){

  const Int_t nvars = this->GetNVars() ;
  //Float_t *vArray =GetCuts();
  //fV0Type = vArray[nvars-1];
  fV0Type = (this->GetCuts())[nvars-1];
  //this->GetCuts(vArray);
  TString *sVarNames=GetVarNames();

  if(sVarNames[nvars-1].Contains("V0 type")) return (Int_t)fV0Type;
  else {AliInfo("AliRDHFCutsLctoV0 Last variable is not the V0 type!!!"); return -999;}
}

//---------------------------------------------------------------------------
void AliRDHFCutsLctoV0::PrintAll() const {
  //
  // print all cuts values
  // 

  printf("Minimum vtx type %d\n",fMinVtxType);
  printf("Minimum vtx contr %d\n",fMinVtxContr);
  printf("Max vtx red chi2 %f\n",fMaxVtxRedChi2);
  printf("Min SPD mult %d\n",fMinSPDMultiplicity);
  printf("Use PID %d (PID selection flag = %d) OldPid=%d\n",(Int_t)fUsePID,(Int_t)fPidSelectionFlag,fPidHF ? fPidHF->GetOldPid() : -1);
  printf("Remove daughters from vtx %d\n",(Int_t)fRemoveDaughtersFromPrimary);
  printf("Recompute primary vertex %d\n",(Int_t)fRecomputePrimVertex);
  printf("Physics selection: %s\n",fUsePhysicsSelection ? "Yes" : "No");
  printf("Pileup rejection: %s\n",(fOptPileup > 0) ? "Yes" : "No");
  printf("UseTrackSelectionWithFilterBits: %s\n",fUseTrackSelectionWithFilterBits ? "Yes" : "No");
  printf("Reject kink: %s\n",fKinkReject ? "Yes" : "No");
  if(fOptPileup==1) printf(" -- Reject pileup event");
  if(fOptPileup==2) printf(" -- Reject tracks from pileup vtx");
  if(fUseCentrality>0) {
    TString estimator="";
    if(fUseCentrality==1) estimator = "V0";
    if(fUseCentrality==2) estimator = "Tracks";
    if(fUseCentrality==3) estimator = "Tracklets";
    if(fUseCentrality==4) estimator = "SPD clusters outer"; 
    printf("Centrality class considered: %.1f-%.1f, estimated with %s",fMinCentrality,fMaxCentrality,estimator.Data());
  }
  if(fIsCandTrackSPDFirst) printf("Check for candidates with pt < %2.2f, that daughters fullfill kFirst criteria\n",fMaxPtCandTrackSPDFirst);

  if(fVarNames){
    cout<<"Array of variables"<<endl;
    for(Int_t iv=0;iv<fnVars;iv++){
      cout<<fVarNames[iv]<<"\t";
    }
    cout<<endl;
  }
  if(fVarsForOpt){
    cout<<"Array of optimization"<<endl;
    for(Int_t iv=0;iv<fnVars;iv++){
      cout<<fVarsForOpt[iv]<<"\t";
    }
    cout<<endl;
  }
  if(fIsUpperCut){
    cout<<"Array of upper/lower cut"<<endl;
   for(Int_t iv=0;iv<fnVars;iv++){
     cout<<fIsUpperCut[iv]<<"\t";
   }
   cout<<endl;
  }
  if(fPtBinLimits){
    cout<<"Array of ptbin limits"<<endl;
    for(Int_t ib=0;ib<fnPtBinLimits;ib++){
      cout<<fPtBinLimits[ib]<<"\t";
    }
    cout<<endl;
  }
  if(fCutsRD){
    cout<<"Matrix of cuts"<<endl;
   for(Int_t iv=0;iv<fnVars;iv++){
     for(Int_t ib=0;ib<fnPtBins;ib++){
       cout<<"fCutsRD["<<iv<<"]["<<ib<<"] = "<<fCutsRD[GetGlobalIndex(iv,ib)]<<"\t";
     } 
     cout<<endl;
   }
   cout<<endl;
  }

  if (fTrackCuts) {
    Float_t eta1=0, eta2=0; fTrackCuts->GetEtaRange(eta1,eta2);
    cout << " etaRange for Bachelor: [" << eta1 << "," << eta2 << "]\n";
  }
  if (fV0daughtersCuts) {
    Float_t eta3=0, eta4=0; fV0daughtersCuts->GetEtaRange(eta3,eta4);
    cout << " etaRange for V0daughters: [" << eta3 << "," << eta4 << "]\n";
  }
  return;

}

//-------------------------
Bool_t AliRDHFCutsLctoV0::IsInFiducialAcceptance(Double_t pt, Double_t y) const
{
  //
  //
  // Checking if Lc is in fiducial acceptance region
  //
  //

  if(fMaxRapidityCand>-998.){
    if(TMath::Abs(y) > fMaxRapidityCand) return kFALSE;
    else return kTRUE;
  }

  if(pt > 5.) {
    // applying cut for pt > 5 GeV
    AliDebug(2,Form("pt of Lambda_c = %f (> 5), cutting at |y| < 0.8",pt));
    if (TMath::Abs(y) > 0.8) return kFALSE;

  } else {
    // appliying smooth cut for pt < 5 GeV
    Double_t maxFiducialY = -0.2/15*pt*pt+1.9/15*pt+0.5;
    Double_t minFiducialY = 0.2/15*pt*pt-1.9/15*pt-0.5;
    AliDebug(2,Form("pt of Lambda_c = %f (< 5), cutting  according to the fiducial zone [%f, %f]\n",pt,minFiducialY,maxFiducialY));
    if (y < minFiducialY || y > maxFiducialY) return kFALSE;
  }
  //
  return kTRUE;
}
//---------------------------------------------------------------------------
Bool_t AliRDHFCutsLctoV0::AreLctoV0DaughtersSelected(AliAODRecoDecayHF *dd) const{
  //
  // Daughter tracks selection
  //

  AliAODRecoCascadeHF* d = (AliAODRecoCascadeHF*)dd;
  if (!d) {
    AliDebug(2,"AliAODRecoCascadeHF null");
    return kFALSE;
  }

  if (!fTrackCuts) {
    AliFatal("Cut object is not defined for bachelor. Candidate accepted.");
    return kFALSE;
  }

  AliAODTrack * bachelorTrack = dynamic_cast<AliAODTrack*>(d->GetBachelor());
  if (!bachelorTrack) return kFALSE;

  if (fIsCandTrackSPDFirst && d->Pt()<fMaxPtCandTrackSPDFirst) {
      if(!bachelorTrack->HasPointOnITSLayer(0)) return kFALSE;
  }

  if (fKinkReject != (!(fTrackCuts->GetAcceptKinkDaughters())) ) {
    AliError(Form("Not compatible setting: fKinkReject=%1d - fTrackCuts->GetAcceptKinkDaughters()=%1d",fKinkReject, fTrackCuts->GetAcceptKinkDaughters()));
    return kFALSE;
  }

  AliAODVertex *vAOD = d->GetPrimaryVtx();
  Double_t pos[3]; vAOD->GetXYZ(pos);
  Double_t cov[6]; vAOD->GetCovarianceMatrix(cov);
  const AliESDVertex vESD(pos,cov,100.,100);

  if (!IsDaughterSelected(bachelorTrack,&vESD,fTrackCuts)) return kFALSE;

  if (!fV0daughtersCuts) {
    AliFatal("Cut object is not defined for V0daughters. Candidate accepted.");
    return kFALSE;
  }

  AliAODv0 * v0 = dynamic_cast<AliAODv0*>(d->Getv0());
  if (!v0) return kFALSE;
  AliAODTrack *v0positiveTrack = dynamic_cast<AliAODTrack*>(d->Getv0PositiveTrack());
  if (!v0positiveTrack) return kFALSE;
  AliAODTrack *v0negativeTrack = dynamic_cast<AliAODTrack*>(d->Getv0NegativeTrack());
  if (!v0negativeTrack) return kFALSE;


  Float_t etaMin=0, etaMax=0; fV0daughtersCuts->GetEtaRange(etaMin,etaMax);
  if ( (v0positiveTrack->Eta()<=etaMin || v0positiveTrack->Eta()>=etaMax) ||
       (v0negativeTrack->Eta()<=etaMin || v0negativeTrack->Eta()>=etaMax) ) return kFALSE;
  Float_t ptMin=0, ptMax=0; fV0daughtersCuts->GetPtRange(ptMin,ptMax);
  if ( (v0positiveTrack->Pt()<=ptMin || v0positiveTrack->Pt()>=ptMax) ||
       (v0negativeTrack->Pt()<=ptMin || v0negativeTrack->Pt()>=ptMax) ) return kFALSE;

  // Condition on nTPCclusters
  if (fV0daughtersCuts->GetMinNClusterTPC()>0) {
    if ( ( ( v0positiveTrack->GetTPCClusterInfo(2,1) ) < fV0daughtersCuts->GetMinNClusterTPC() ) || 
	 ( ( v0negativeTrack->GetTPCClusterInfo(2,1) ) < fV0daughtersCuts->GetMinNClusterTPC() ) ) return kFALSE;
  }

  // kTPCrefit status
  if (v0->GetOnFlyStatus()==kFALSE) { // only for offline V0s
    if (fV0daughtersCuts->GetRequireTPCRefit()) {
      if( !(v0positiveTrack->GetStatus() & AliESDtrack::kTPCrefit)) return kFALSE;
      if( !(v0negativeTrack->GetStatus() & AliESDtrack::kTPCrefit)) return kFALSE;
    }
  }
  // kink condition
  if (!fV0daughtersCuts->GetAcceptKinkDaughters()) {
    AliAODVertex *maybeKinkPos = (AliAODVertex*)v0positiveTrack->GetProdVertex();
    AliAODVertex *maybeKinkNeg = (AliAODVertex*)v0negativeTrack->GetProdVertex();
    if (maybeKinkPos->GetType()==AliAODVertex::kKink ||
	maybeKinkNeg->GetType()==AliAODVertex::kKink) return kFALSE;
  }
  // Findable clusters > 0 condition - from V0 analysis
  //if( v0positiveTrack->GetTPCNclsF()<=0 || v0negativeTrack->GetTPCNclsF()<=0 ) return kFALSE;
  /*
    Float_t lPosTrackCrossedRows = v0positiveTrack->GetTPCClusterInfo(2,1);
    Float_t lNegTrackCrossedRows = v0positiveTrack->GetTPCClusterInfo(2,1);
    fTreeVariableLeastNbrCrossedRows = (Int_t) lPosTrackCrossedRows;
    if( lNegTrackCrossedRows < fTreeVariableLeastNbrCrossedRows )
    fTreeVariableLeastNbrCrossedRows = (Int_t) lNegTrackCrossedRows;
    //Compute ratio Crossed Rows / Findable clusters
    //Note: above test avoids division by zero!
    Float_t lPosTrackCrossedRowsOverFindable = lPosTrackCrossedRows / ((double)(pTrack->GetTPCNclsF()));
    Float_t lNegTrackCrossedRowsOverFindable = lNegTrackCrossedRows / ((double)(nTrack->GetTPCNclsF()));
    fTreeVariableLeastRatioCrossedRowsOverFindable = lPosTrackCrossedRowsOverFindable;
    if( lNegTrackCrossedRowsOverFindable < fTreeVariableLeastRatioCrossedRowsOverFindable )
    fTreeVariableLeastRatioCrossedRowsOverFindable = lNegTrackCrossedRowsOverFindable;
    //Lowest Cut Level for Ratio Crossed Rows / Findable = 0.8, set here
    if ( fTreeVariableLeastRatioCrossedRowsOverFindable < 0.8 ) return kFALSE;
  */

  return kTRUE;

}
