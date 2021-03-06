#include "Riostream.h"

void runcls_ln_ge1btight_t1bbbb_7_ol(int mgl, int mlsp, double mu_susy_sig) {

  gROOT->LoadMacro("ra2bRoostatsClass7.c+") ;

  ra2bRoostatsClass7 ra2b(1,0,2) ; 
  
  ra2b.initialize("an-11-257-v3-files/input-files/byhand-data-ge1b-tight.txt",
                  "an-11-257-v3-files/input-files/signalSyst.T1bbbb.ge1bTight.dat", mgl, mlsp, true, 1.0 ) ;
  
  gROOT->LoadMacro("OLHypoTestInvDemo.c+") ;
  
  double result, exp_res, exp_res_minus, exp_res_plus ;

  stringstream Mgl; Mgl << mgl ;
  stringstream Mlsp; Mlsp << mlsp ;

  TString fname = "partial_results/cls_ge1btight_t1bbb_";
  fname += Mgl.str();
  fname += "_";
  fname += Mlsp.str();
  fname += ".root";

  OLHypoTestInvDemo ("ws.root", "ws", "SbModel", "BModel", 
  		     "ra2b_observed_rds", 0, 3, true, 1, mu_susy_sig, mu_susy_sig, 500,
  		     mgl, mlsp, fname);

}



