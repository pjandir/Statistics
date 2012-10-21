
#include "TRandom2.h"
#include "TFile.h"
#include "TPad.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TTree.h"
#include "TF1.h"
#include "TH1F.h"
#include "RooGaussian.h"
#include "RooProdPdf.h"
#include "RooAbsPdf.h"
#include "RooWorkspace.h"
#include "RooDataSet.h"
#include "RooArgSet.h"
#include "RooRealVar.h"
#include "RooMCStudy.h"
#include "RooNLLVar.h"
#include "RooPlot.h"
#include "RooFitResult.h"
#include "RooStats/ModelConfig.h"
#include "RooMinuit.h"

#include <string.h>

  using namespace RooFit;
  using namespace RooStats;

  //==============================================================================================

   void ws_constrained_profile3D( const char* wsfile = "rootfiles/ws-met3-ht3-v2.root",
                                   const char* new_poi_name = "n_M2_H2_1b",
                                   const char* ignore_observable_name = "N_0lep_M2_H2_1b",
                                   int npoiPoints = 10,
                                   double poiMinVal = -1.,
                                   double poiMaxVal = -1.,
                                   double constraintWidth = 4.,
                                   int verbLevel=0 ) {





     //--- Tell RooFit to shut up about anything less important than an ERROR.
      RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR) ;



       if ( verbLevel > 0 ) { printf("\n\n Verbose level : %d\n\n", verbLevel) ; }


       TFile* wstf = new TFile( wsfile ) ;

       RooWorkspace* ws = dynamic_cast<RooWorkspace*>( wstf->Get("ws") );

       if ( verbLevel > 0 ) { ws->Print() ; }






       RooDataSet* rds = (RooDataSet*) ws->obj( "ra2b_observed_rds" ) ;

       if ( verbLevel > 0 ) {
          printf("\n\n\n  ===== RooDataSet ====================\n\n") ;
          rds->Print() ;
          rds->printMultiline(cout, 1, kTRUE, "") ;
       }





       ModelConfig* modelConfig = (ModelConfig*) ws->obj( "SbModel" ) ;
       RooAbsPdf* likelihood = modelConfig->GetPdf() ;

       RooRealVar* rrv_mu_susy_all0lep = ws->var("mu_susy_all0lep") ;
       if ( rrv_mu_susy_all0lep == 0x0 ) {
          printf("\n\n\n *** can't find mu_susy_all0lep in workspace.  Quitting.\n\n\n") ;
          return ;
       }





       //-- do BG only for now.
       rrv_mu_susy_all0lep->setVal(0.) ;
       rrv_mu_susy_all0lep->setConstant( kTRUE ) ;








       //-- do a prefit.

       printf("\n\n\n ====== Pre fit with unmodified nll var.\n\n") ;

       RooFitResult* dataFitResultSusyFixed = likelihood->fitTo(*rds, Save(true),Hesse(false),Minos(false),Strategy(1),PrintLevel(verbLevel));
       int dataSusyFixedFitCovQual = dataFitResultSusyFixed->covQual() ;
       if ( dataSusyFixedFitCovQual < 2 ) { printf("\n\n\n *** Failed fit!  Cov qual %d.  Quitting.\n\n", dataSusyFixedFitCovQual ) ; return ; }
       double dataFitSusyFixedNll = dataFitResultSusyFixed->minNll() ;

       if ( verbLevel > 0 ) {
          dataFitResultSusyFixed->Print("v") ;
       }

       printf("\n\n Nll value, from fit result : %.3f\n\n", dataFitSusyFixedNll ) ;

       delete dataFitResultSusyFixed ;






       //-- find the new POI parameter.
       RooAbsReal* new_poi_rar(0x0) ;

       new_poi_rar = ws->var( new_poi_name ) ;
       if ( new_poi_rar == 0x0 ) {
          printf("\n\n New POI %s is not a variable.  Trying function.\n\n", new_poi_name ) ;
          new_poi_rar = ws->function( new_poi_name ) ;
          if ( new_poi_rar == 0x0 ) {
             printf("\n\n New POI %s is not a function.  I quit.\n\n", new_poi_name ) ;
             return ;
          }
       } else {
          printf("\n\n     New POI %s is a variable with current value %.1f.\n\n", new_poi_name, new_poi_rar->getVal() ) ;
       }

       double startPoiVal = new_poi_rar->getVal() ;

       if ( npoiPoints <=0 ) {
          printf("\n\n Quitting now.\n\n" ) ;
          return ;
       }


      //--- The RooNLLVar is NOT equivalent to what minuit uses.
  //   RooNLLVar* nll = new RooNLLVar("nll","nll", *likelihood, *rds ) ;
  //   printf("\n\n Nll value, from construction : %.3f\n\n", nll->getVal() ) ;

      //--- output of createNLL IS what minuit uses, so use that.
       RooAbsReal* nll = likelihood -> createNLL( *rds, Verbose(true) ) ;

       RooRealVar* rrv_poiValue = new RooRealVar( "poiValue", "poiValue", 0., -10000., 10000. ) ;
   /// rrv_poiValue->setVal( poiMinVal ) ;
   /// rrv_poiValue->setConstant(kTRUE) ;

       RooRealVar* rrv_constraintWidth = new RooRealVar("constraintWidth","constraintWidth", 0.1, 0.1, 1000. ) ;
       rrv_constraintWidth -> setVal( constraintWidth ) ;
       rrv_constraintWidth -> setConstant(kTRUE) ;




       if ( verbLevel > 0 ) {
          printf("\n\n ======= debug likelihood print\n\n") ;
          likelihood->Print("v") ;
          printf("\n\n ======= debug nll print\n\n") ;
          nll->Print("v") ;
       }






    //----------------------------------------------------------------------------------------------

       RooMinuit* rminuit( 0x0 ) ;

       RooFormulaVar* plot_var( 0x0 ) ;


       RooRealVar* rrv_obs(0x0) ;

       if ( strcmp( ignore_observable_name, "none" ) != 0 ) {

          char pdfName[1000] ;
          sprintf( pdfName, "pdf_%s", ignore_observable_name ) ;
          RooAbsPdf* pdf_ignore = ws -> pdf( pdfName ) ;

          if ( pdf_ignore == 0x0 ) {
             printf("\n\n *** Told to ignore %s but can't find pdf %s.\n\n", ignore_observable_name, pdfName ) ;
             return ;
          }

          rrv_obs = ws -> var( ignore_observable_name ) ;
          if ( rrv_obs == 0x0 ) {
             printf("\n\n *** Can't find RooRealVar for observable %s\n\n", ignore_observable_name ) ;
             return ;
          }
          rrv_obs->setConstant(kTRUE) ;


          printf("\n\n Ignoring observable %s by removing pdf %s\n\n", ignore_observable_name, pdfName ) ;

          char minuit_formula[10000] ;
          sprintf( minuit_formula, "%s+%s*(%s-%s)*(%s-%s)+log(%s)",
                 nll->GetName(),
                 rrv_constraintWidth->GetName(),
                 new_poi_rar->GetName(), rrv_poiValue->GetName(),
                 new_poi_rar->GetName(), rrv_poiValue->GetName(),
                 pdf_ignore->GetName()
                 ) ;

          printf("\n\n Creating new minuit variable with formula: %s\n\n", minuit_formula ) ;
          RooFormulaVar* new_minuit_var = new RooFormulaVar("new_minuit_var", minuit_formula,
              RooArgList( *nll,
                          *rrv_constraintWidth,
                          *new_poi_rar, *rrv_poiValue,
                          *new_poi_rar, *rrv_poiValue,
                          *pdf_ignore
                          ) ) ;

          printf("\n\n Current value is %.2f\n\n",
               new_minuit_var->getVal() ) ;

          rminuit = new RooMinuit( *new_minuit_var ) ;




          char plotvar_formula[10000] ;
          sprintf( plotvar_formula, "%s+log(%s)",
                 nll->GetName(),
                 pdf_ignore->GetName()
                 ) ;

          printf("\n\n Creating new plot variable with formula: %s\n\n", plotvar_formula ) ;
          plot_var = new RooFormulaVar("plot_var", plotvar_formula,
              RooArgList( *nll,
                          *pdf_ignore
                          ) ) ;

          printf("\n\n Current value is %.2f\n\n",
               plot_var->getVal() ) ;


       } else {

          char minuit_formula[10000] ;
          sprintf( minuit_formula, "%s+%s*(%s-%s)*(%s-%s)",
                 nll->GetName(),
                 rrv_constraintWidth->GetName(),
                 new_poi_rar->GetName(), rrv_poiValue->GetName(),
                 new_poi_rar->GetName(), rrv_poiValue->GetName()
                 ) ;

          printf("\n\n Creating new minuit variable with formula: %s\n\n", minuit_formula ) ;
          RooFormulaVar* new_minuit_var = new RooFormulaVar("new_minuit_var", minuit_formula,
              RooArgList( *nll,
                          *rrv_constraintWidth,
                          *new_poi_rar, *rrv_poiValue,
                          *new_poi_rar, *rrv_poiValue
                          ) ) ;

          printf("\n\n Current value is %.2f\n\n",
               new_minuit_var->getVal() ) ;

          rminuit = new RooMinuit( *new_minuit_var ) ;




          char plotvar_formula[10000] ;
          sprintf( plotvar_formula, "%s",
                 nll->GetName()
                 ) ;

          printf("\n\n Creating new plot variable with formula: %s\n\n", plotvar_formula ) ;
          plot_var = new RooFormulaVar("plot_var", plotvar_formula,
              RooArgList( *nll
                          ) ) ;

          printf("\n\n Current value is %.2f\n\n",
               plot_var->getVal() ) ;


       }


       rminuit->setPrintLevel(verbLevel-1) ;
       if ( verbLevel <=0 ) { rminuit->setNoWarn() ; }

    //----------------------------------------------------------------------------------------------

       //-- If POI range is -1 to -1, automatically determine the range using the set value.

       if ( poiMinVal < 0. && poiMaxVal < 0. ) {

          printf("\n\n Automatic determination of scan range.\n\n") ;

          if ( startPoiVal <= 0. ) {
             printf("\n\n *** POI starting value zero or negative %g.  Quit.\n\n\n", startPoiVal ) ;
             return ;
          }

          poiMinVal = startPoiVal - 3.5 * sqrt(startPoiVal) ;
          poiMaxVal = startPoiVal + 6.0 * sqrt(startPoiVal) ;

          if ( poiMinVal < 0. ) { poiMinVal = 0. ; }

          printf("    Start val = %g.   Scan range:   %g  to  %g\n\n", startPoiVal, poiMinVal, poiMaxVal ) ;


       }



    //----------------------------------------------------------------------------------------------


       double poiVals[1000] ;
       double nllVals[1000] ;
       double minNllVal(1.e9) ;



       for ( int poivi=0; poivi < npoiPoints ; poivi++ ) {

          double poiValue = poiMinVal + poivi*(poiMaxVal-poiMinVal)/(1.*npoiPoints) ;

          rrv_poiValue -> setVal( poiValue ) ;
          rrv_poiValue -> setConstant( kTRUE ) ;


       //+++++++++++++++++++++++++++++++++++

          rminuit->migrad() ;
          rminuit->hesse() ;
          RooFitResult* rfr = rminuit->save() ;

       //+++++++++++++++++++++++++++++++++++


          if ( verbLevel > 0 ) { rfr->Print("v") ; }


          float fit_minuit_var_val = rfr->minNll() ;

          printf(" %02d : poi constraint = %.2f : allvars : MinuitVar, createNLL, PV, POI :    %.5f   %.5f   %.5f   %.5f\n",
                poivi, rrv_poiValue->getVal(), fit_minuit_var_val, nll->getVal(), plot_var->getVal(), new_poi_rar->getVal() ) ;
          cout << flush ;

          poiVals[poivi] = new_poi_rar->getVal() ;
          nllVals[poivi] = plot_var->getVal() ;

          if ( nllVals[poivi] < minNllVal ) { minNllVal = nllVals[poivi] ; }

          delete rfr ;


       } // poivi

       double nllDiffVals[1000] ;

       double poiAtMinlnL(-1.) ;
       double poiAtMinusDelta1(-1.) ;
       double poiAtPlusDelta1(-1.) ;
       for ( int poivi=0; poivi < npoiPoints ; poivi++ ) {
          nllDiffVals[poivi] = nllVals[poivi] - minNllVal ;
          double poiValue = poiMinVal + poivi*(poiMaxVal-poiMinVal)/(1.*npoiPoints) ;
          if ( nllDiffVals[poivi] < 0.01 ) { poiAtMinlnL = poiValue ; }
          if ( poiAtMinusDelta1 < 0. && nllDiffVals[poivi] < 1.5 ) { poiAtMinusDelta1 = poiValue ; }
          if ( poiAtMinlnL > 0. && poiAtPlusDelta1 < 0. && nllDiffVals[poivi] > 1.0 ) { poiAtPlusDelta1 = poiValue ; }
       } // poivi

       printf("\n\n Estimates for poi at delta ln L = -1, 0, +1:  %g ,   %g ,   %g\n\n", poiAtMinusDelta1, poiAtMinlnL, poiAtPlusDelta1 ) ;



       TCanvas* cscan = (TCanvas*) gDirectory->FindObject("cscan") ;
       if ( cscan == 0x0 ) {
          printf("\n Creating canvas.\n\n") ;
          cscan = new TCanvas("cscan","Delta nll") ;
       }
       TGraph* graph = new TGraph( npoiPoints, poiVals, nllDiffVals ) ;
       char gname[1000] ;
       sprintf( gname, "scan_%s", new_poi_name ) ;
       graph->SetName( gname ) ;


       double poiBest(-1.) ;
       double poiMinus1stdv(-1.) ;
       double poiPlus1stdv(-1.) ;
       double deltalnLMin(1e9) ;
       if ( poiAtMinusDelta1 >= 0. && poiAtPlusDelta1 > 0. ) {
          graph->Fit("pol5","", "", poiAtMinusDelta1, poiAtPlusDelta1 ) ;
          TF1* fitFunc = graph->GetFunction("pol5") ;
          if ( fitFunc != 0 ) {
             int npoints(1000) ;
             for ( int fi=0; fi<npoints; fi++ ) {
                double poiVal = poiAtMinusDelta1 + (poiAtPlusDelta1-poiAtMinusDelta1)/(1.*npoints)*fi ;
                double fitdeltalnL = fitFunc->Eval( poiVal ) ;
                if ( poiMinus1stdv < 0. && fitdeltalnL<0.5 ) { poiMinus1stdv = poiVal ; }
                if ( fitdeltalnL < deltalnLMin ) { poiBest = poiVal;  deltalnLMin = fitdeltalnL ; }
                if ( deltalnLMin < 0.3 && poiPlus1stdv < 0. && fitdeltalnL > 0.5 ) { poiPlus1stdv = poiVal ; }
             } // fi.
          }
          printf("\n\n POI estimate :  %g  +%g  -%g    [%g,%g]\n\n",
                  poiBest, (poiPlus1stdv-poiBest), (poiBest-poiMinus1stdv), poiMinus1stdv, poiPlus1stdv ) ;
          if ( rrv_obs != 0 ) {
             printf(" Observable value : %g\n\n", rrv_obs->getVal() ) ;
          }
       } else {
          printf("\n\n *** Scan range insufficient.\n\n\n") ;
       }

       graph->SetLineColor(4) ;
       graph->SetLineWidth(3) ;
       graph->Draw("ACP") ;
       gPad->SetGridx(1) ;
       gPad->SetGridy(1) ;
       cscan->Update() ;

       char hname[1000] ;
       sprintf( hname, "hscanout_%s", new_poi_name ) ;
       TH1F* hsout = new TH1F( hname,"scan results",4,0.,4.) ;
       double obsVal(-1.) ;
       if ( rrv_obs != 0 ) { obsVal = rrv_obs->getVal() ; }
       hsout->SetBinContent(1, obsVal ) ;
       hsout->SetBinContent(2, poiPlus1stdv ) ;
       hsout->SetBinContent(3, poiBest ) ;
       hsout->SetBinContent(4, poiMinus1stdv ) ;
       TAxis* xaxis = hsout->GetXaxis() ;
       xaxis->SetBinLabel(1,"Observed val.") ;
       xaxis->SetBinLabel(2,"Model+1sd") ;
       xaxis->SetBinLabel(3,"Model") ;
       xaxis->SetBinLabel(4,"Model-1sd") ;

       TFile fout("cscan.root","recreate") ;
       graph->Write() ;
       hsout->Write() ;
       fout.Close() ;

       delete ws ;
       wstf->Close() ;
    // delete wstf ;
    // delete rds ;
    // delete modelConfig ;
    // delete likelihood ;

   }

