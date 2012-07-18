/***************************************************************************** 
 * Project: RooFit                                                           * 
 *                                                                           * 
 * This code was autogenerated by RooClassFactory                            * 
 *****************************************************************************/ 

// Your description goes here... 

#include "Riostream.h" 

#include "RooCorrelatedBetaPrimeGeneratorHelper.h" 
#include "RooAbsReal.h" 
#include "RooAbsCategory.h" 
#include <math.h> 
#include "TMath.h" 
#include "Math/Math.h"
#include "Math/QuantFuncMathCore.h"
#include "betaHelperFunctions.h"

ClassImp(RooCorrelatedBetaPrimeGeneratorHelper) 

 RooCorrelatedBetaPrimeGeneratorHelper::RooCorrelatedBetaPrimeGeneratorHelper(const char *name, const char *title, 
                        RooAbsReal& _parameter,
                        RooAbsReal& _alpha,
                        RooAbsReal& _beta) :
   RooAbsPdf(name,title), 
   parameter("parameter","parameter",this,_parameter),
   alpha("alpha","alpha",this,_alpha),
   beta("beta","beta",this,_beta)
 { 
 } 


 RooCorrelatedBetaPrimeGeneratorHelper::RooCorrelatedBetaPrimeGeneratorHelper(const RooCorrelatedBetaPrimeGeneratorHelper& other, const char* name) :  
   RooAbsPdf(other,name), 
   parameter("parameter",this,other.parameter),
   alpha("alpha",this,other.alpha),
   beta("beta",this,other.beta)
 { 
 } 



 Double_t RooCorrelatedBetaPrimeGeneratorHelper::evaluate() const 
 { 
   // ENTER EXPRESSION IN TERMS OF VARIABLE ARGUMENTS HERE 
   return 1.0 ; 
 } 

 Int_t RooCorrelatedBetaPrimeGeneratorHelper::getAnalyticalIntegral(RooArgSet& allVars, RooArgSet& analVars, const char* /*rangeName*/) const 
 {
   // Advertise analytical integral
   return 1 ;
 }
 
 
 Double_t RooCorrelatedBetaPrimeGeneratorHelper::analyticalIntegral(Int_t code, const char* rangeName) const 
 {
   assert(code==1) ;
   return 1. ;
 }


 Int_t RooCorrelatedBetaPrimeGeneratorHelper::getGenerator(const RooArgSet& directVars, RooArgSet &generateVars, Bool_t /*staticInitOK*/) const
 {
   if (matchArgs(directVars,generateVars,alpha,beta)) return 1 ;  
   return 0 ;
 }
 
 void RooCorrelatedBetaPrimeGeneratorHelper::generateEvent(Int_t code)
 {
   assert(code==1) ;
   double newMode;
   double betaNonPrime = ROOT::Math::beta_quantile(parameter,alpha,beta);
   if (betaNonPrime == 1) newMode = RooNumber::infinity();
   else newMode = betaNonPrime/(1-betaNonPrime);
   double newSigma = sqrt( alpha * (alpha + beta - 1 ) / ( pow(beta - 1 , 2 ) * (beta - 2) ) );
   double newAlpha, newBeta;

   betaPrimeModeTransform(newMode , newSigma ,
			  newAlpha , newBeta );
   alpha = newAlpha;
   beta = newBeta;
   return;
 }



