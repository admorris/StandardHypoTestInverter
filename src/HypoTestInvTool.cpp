// Adapted from https://root.cern.ch/doc/v608/StandardHypoTestInvDemo_8C.html
// ROOT
#include "TCanvas.h"
#include "TFile.h"
#include "TProofLite.h"
// RooFit
#include "RooRandom.h"
// RooStats
#include "RooStats/HypoTestInverterPlot.h"
#include "RooStats/HypoTestPlot.h"
#include "RooStats/ModelConfig.h"

// Calculators
#include "RooStats/AsymptoticCalculator.h"
#include "RooStats/FrequentistCalculator.h"
#include "RooStats/HybridCalculator.h"
// Test statistics
#include "RooStats/MaxLikelihoodEstimateTestStat.h"
#include "RooStats/NumEventsTestStat.h"
#include "RooStats/ProfileLikelihoodTestStat.h"
#include "RooStats/RatioOfProfiledLikelihoodsTestStat.h"
#include "RooStats/SimpleLikelihoodRatioTestStat.h"
// Local
#include "HypoTestInvTool.h"
namespace RooStats
{
/*********************************************************************************************************************/
	// Constructor stores options and retrieves PDFs and Data
	HypoTestInvTool::HypoTestInvTool(RooWorkspace& workspace, std::string modelSBName, std::string modelBName, std::string dataName, std::string nuisPriorName, const HypoTestInvOptions& _options) : options(_options), pc(workspace, options.NWorkers, "", kFALSE)
	{
		Info("HypoTestInvTool", "Running on the workspace %s", workspace.GetName());
		data = workspace.data(dataName.c_str());
		if(!data)
			Error("HypoTestInvTool", "Data with name %s does not exist", dataName.c_str());
		else
			Info("HypoTestInvTool", "Using data set %s", dataName.c_str());
		if(options.UseVectorStore)
		{
			RooAbsData::setDefaultStorageType(RooAbsData::Vector);
			data->convertToVectorStore() ;
		}
		// get models from WS
		// get the modelConfig out of the file
		bModel = (ModelConfig*) workspace.obj(modelBName.c_str());
		sbModel = (ModelConfig*) workspace.obj(modelSBName.c_str());
		if(!sbModel)
			Error("HypoTestInvTool", "Model with name %s does not exist", modelSBName.c_str());
		// check the model
		if(!sbModel->GetPdf())
			Error("HypoTestInvTool", "Model %s has no PDF", modelSBName.c_str());
		if(!sbModel->GetParametersOfInterest())
			Error("HypoTestInvTool", "Model %s has no POI", modelSBName.c_str());
		if(!sbModel->GetObservables())
			Error("HypoTestInvTool", "Model %s has no observables", modelSBName.c_str());
		if(!sbModel->GetSnapshot())
		{
			Info("HypoTestInvTool", "Model %s has no snapshot - make one using model POI", modelSBName.c_str());
			sbModel->SetSnapshot(*sbModel->GetParametersOfInterest());
		}
		// case of no systematics
		// remove nuisance parameters from model
		if(options.NoSystematics)
		{
			const RooArgSet* nuisPar = sbModel->GetNuisanceParameters();
			if(nuisPar && nuisPar->getSize() > 0)
			{
				Info("HypoTestInvTool", "Switch off all systematics by setting them constant to their initial values");
				RooStats::SetAllConstant(*nuisPar);
			}
			if(bModel)
			{
				const RooArgSet* bnuisPar = bModel->GetNuisanceParameters();
				if(bnuisPar)
					RooStats::SetAllConstant(*bnuisPar);
			}
		}
		if(!bModel || bModel == sbModel)
		{
			Info("HypoTestInvTool", "The background model %s does not exist", modelBName.c_str());
			Info("HypoTestInvTool", "Copy it from ModelConfig %s and set POI to zero", modelSBName.c_str());
			bModel = (ModelConfig*) sbModel->Clone();
			bModel->SetName((modelSBName+"_with_poi_0").c_str());
			RooRealVar* var = dynamic_cast<RooRealVar*>(bModel->GetParametersOfInterest()->first());
			double oldval = var->getVal();
			var->setVal(0);
			bModel->SetSnapshot(RooArgSet(*var));
			var->setVal(oldval);
		}
		else if(!bModel->GetSnapshot())
		{
			Info("HypoTestInvTool", "Model %s has no snapshot  - make one using model POI and 0 values ", modelBName.c_str());
			RooRealVar* var = dynamic_cast<RooRealVar*>(bModel->GetParametersOfInterest()->first());
			if(var)
			{
				double oldval = var->getVal();
				var->setVal(0);
				bModel->SetSnapshot(RooArgSet(*var));
				var->setVal(oldval);
			}
			else
				Error("HypoTestInvTool", "Model %s has no valid POI", modelBName.c_str());
		}
		// check model  has global observables when there are nuisance pdf
		// for the hybrid case the globals are not needed
		if(options.CalculatorType != 1)
		{
			bool hasNuisParam = (sbModel->GetNuisanceParameters() && sbModel->GetNuisanceParameters()->getSize() > 0);
			bool hasGlobalObs = (sbModel->GetGlobalObservables() && sbModel->GetGlobalObservables()->getSize() > 0);
			if(hasNuisParam && !hasGlobalObs)
			{
				// try to see ifmodel has nuisance parameters first
				RooAbsPdf * constrPdf = RooStats::MakeNuisancePdf(*sbModel, "nuisanceConstraintPdf_sbmodel");
				if(constrPdf)
				{
					Warning("HypoTestInvTool", "Model %s has nuisance parameters but no global observables associated", sbModel->GetName());
					Warning("HypoTestInvTool", "The effect of the nuisance parameters will not be treated correctly");
				}
			}
		}
		else if(!nuisPriorName.empty())
			nuisPdf = workspace.pdf(nuisPriorName.c_str());
		// save all initial parameters of the model including the global observables
		sbModel->GetPdf()->getParameters(*data)->snapshot(initialParameters);
	}
/*********************************************************************************************************************/
	// internal routine to run the inverter
	HypoTestInverterResult* HypoTestInvTool::RunInverter()
	{
		// run first a data fit
		const RooArgSet* poiSet = sbModel->GetParametersOfInterest();
		RooRealVar* poi = (RooRealVar*)poiSet->first();
		Info("HypoTestInvTool::RunInverter", "POI initial value: %s = %f", poi->GetName(), poi->getVal());
		// fit the data first (need to use constraint)
		bool doFit = options.InitialFit;
		if(options.InitialFit == -1 && (options.TestStatType == 0 || options.CalculatorType == 3)) doFit = false;  // case of LEP test statistic or Asymptoticcalculator with nominal Asimov
		// Choose the minimizer
		if(options.MinimizerType.empty())
			options.MinimizerType = ROOT::Math::MinimizerOptions::DefaultMinimizerType();
		else
			ROOT::Math::MinimizerOptions::SetDefaultMinimizer(options.MinimizerType.c_str());
		Info("HypoTestInvTool::RunInverter", "Using %s as minimizer for computing the test statistic", ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str());
		// Optionally perform an initial fit
		if(doFit)
		{
			InitialFit(*poi);
		}
		// print a message in case of LEP test statistics because it affects result by doing or not doing a fit
		if(options.TestStatType == 0)
		{
			if(!doFit)
				Info("HypoTestInvTool::RunInverter", "Using LEP test statistic - an initial fit is not done and the TS will use the nuisances at the model value");
			else
				Info("HypoTestInvTool::RunInverter", "Using LEP test statistic - an initial fit has been done and the TS will use the nuisances at the best fit value");
		}
		if(options.MaxPoi > 0) poi->setMax(options.MaxPoi);  // increase limit
		TestStatistic* testStat = CreateTestStat(*poi);
		if(testStat == 0)
		{
			Error("HypoTestInvTool::RunInverter", "Test statistic failed to construct");
			return 0;
		}
		if(options.Optimize) ROOT::Math::MinimizerOptions::SetDefaultStrategy(0);
		AsymptoticCalculator::SetPrintLevel(options.PrintLevel);
		// create the HypoTest calculator class
		HypoTestCalculatorGeneric * hc = CreateCalculator(*testStat);
		// Get the result
		RooMsgService::instance().getStream(1).removeTopic(RooFit::NumIntegration);
		HypoTestInverter calc(*hc);
		calc.SetConfidenceLevel(options.ConfLevel);
		calc.UseCLs(options.UseCLs);
		calc.SetVerbose(true);
		if(options.NPoints > 0)
		{
			if(options.PoiMin > options.PoiMax)
			{
				// if no min/max given scan between MLE and +4 sigma
				options.PoiMin = int(poi->getVal());
				options.PoiMax = int(poi->getVal() +  4 * poi->getError());
			}
			std::cout << "Doing a fixed scan in interval : " << options.PoiMin << " , " << options.PoiMax << "\n";
			calc.SetFixedScan(options.NPoints, options.PoiMin, options.PoiMax);
		}
		else
		{
			std::cout << "Doing an automatic scan in interval : " << poi->getMin() << " , " << poi->getMax() << "\n";
		}
		HypoTestInverterResult* r = calc.GetInterval();
		if(options.Rebuild)
		{
			r = Rebuild(r, calc);
		}
		return r;
	}
/*********************************************************************************************************************/
	void HypoTestInvTool::InitialFit(RooRealVar& poi)
	{
		double poihat = 0;
		// do the fit : By doing a fit the POI snapshot (for S+B)  is set to the fit value
		// and the nuisance parameters nominal values will be set to the fit value.
		// This is relevant when using LEP test statistics
		Info("HypoTestInvTool::InitialFit", "Doing a first fit to the observed data ");
		RooArgSet constrainParams;
		if(sbModel->GetNuisanceParameters()) constrainParams.add(*sbModel->GetNuisanceParameters());
		RooStats::RemoveConstantParameters(&constrainParams);
		RooFitResult* fitres = sbModel->GetPdf()->fitTo(*data, RooFit::InitialHesse(false), RooFit::Hesse(false), RooFit::Minimizer(options.MinimizerType.c_str(), "Migrad"), RooFit::Strategy(0), RooFit::PrintLevel(options.PrintLevel), RooFit::Constrain(constrainParams), RooFit::Save(true), RooFit::Offset(RooStats::IsNLLOffset()));
		if(fitres->status() != 0)
		{
			Warning("HypoTestInvTool::InitialFit", "Fit to the model failed - try with strategy 1 and perform first an Hesse computation");
			fitres = sbModel->GetPdf()->fitTo(*data, RooFit::InitialHesse(true), RooFit::Hesse(false), RooFit::Minimizer(options.MinimizerType.c_str(), "Migrad"), RooFit::Strategy(1), RooFit::PrintLevel(options.PrintLevel+1), RooFit::Constrain(constrainParams), RooFit::Save(true), RooFit::Offset(RooStats::IsNLLOffset()));
		}
		if(fitres->status() != 0)
			Warning("HypoTestInvTool::InitialFit", "Fit still failed - continue anyway.....");
		poihat = poi.getVal();
		Info("HypoTestInvTool::InitialFit", "Best Fit value : %s = %f ± %f", poi.GetName(), poihat, poi.getError());
		//save best fit value in the poi snapshot
		sbModel->SetSnapshot(*sbModel->GetParametersOfInterest());
		Info("HypoTestInvTool::InitialFit", "Snapshot of S+B Model %s is set to the best fit value", sbModel->GetName());
	}
/*********************************************************************************************************************/
	TestStatistic* HypoTestInvTool::CreateTestStat(RooRealVar& poi)
	{
		TestStatistic* testStat;
		if(options.TestStatType == 0)
		{
			SimpleLikelihoodRatioTestStat* slrts = new SimpleLikelihoodRatioTestStat(*sbModel->GetPdf(), *bModel->GetPdf());
			slrts->SetReuseNLL(options.Optimize);
			// null parameters must includes snapshot of poi plus the nuisance values
			RooArgSet nullParams(*sbModel->GetSnapshot());
			if(sbModel->GetNuisanceParameters()) nullParams.add(*sbModel->GetNuisanceParameters());
			if(sbModel->GetSnapshot()) slrts->SetNullParameters(nullParams);
			RooArgSet altParams(*bModel->GetSnapshot());
			if(bModel->GetNuisanceParameters()) altParams.add(*bModel->GetNuisanceParameters());
			if(bModel->GetSnapshot()) slrts->SetAltParameters(altParams);
			if(options.EnableDetOutput) slrts->EnableDetailedOutput();
			testStat = slrts;
		}
		else if(options.TestStatType == 1 || options.TestStatType == 11)
		{
			// ratio of profile likelihood - need to pass snapshot for the alt
			RatioOfProfiledLikelihoodsTestStat* ropl = new RatioOfProfiledLikelihoodsTestStat(*sbModel->GetPdf(), *bModel->GetPdf(), bModel->GetSnapshot());
			ropl->SetSubtractMLE(options.TestStatType == 11);
			ropl->SetPrintLevel(options.PrintLevel);
			ropl->SetMinimizer(options.MinimizerType.c_str());
			if(options.EnableDetOutput) ropl->EnableDetailedOutput();
			ropl->SetReuseNLL(options.Optimize);
			if(options.Optimize) ropl->SetStrategy(0);
			testStat = ropl;
		}
		else if(options.TestStatType == 2 || options.TestStatType == 3 || options.TestStatType == 4)
		{
			// profile likelihood test statistic
			ProfileLikelihoodTestStat* profll = new ProfileLikelihoodTestStat(*sbModel->GetPdf());
			profll->SetOneSided(options.TestStatType == 3);
			profll->SetSigned(options.TestStatType == 4);
			profll->SetMinimizer(options.MinimizerType.c_str());
			profll->SetPrintLevel(options.PrintLevel);
			if(options.EnableDetOutput) profll->EnableDetailedOutput();
			profll->SetReuseNLL(options.Optimize);
			if(options.Optimize) profll->SetStrategy(0);
			testStat = profll;
		}
		else if(options.TestStatType == 5)
		{
			MaxLikelihoodEstimateTestStat* maxll = new MaxLikelihoodEstimateTestStat(*sbModel->GetPdf(), poi);
			testStat = maxll;
		}
		else if(options.TestStatType == 6)
		{
			NumEventsTestStat* nevtts = new NumEventsTestStat;
			testStat = nevtts;
		}
		else Error("HypoTestInvTool::CreateTestStat", "Invalid test statistic type = %d supported values are only :\n\t\t\t 0 (LEP), 1 (Tevatron), 11 (Tevatron minus MLE), 2 (PLR), 3 (one-sided PLR), 4 (signed PLR), 5 (MLE), 6 (Num obs)", options.TestStatType);
		return testStat;
	}
/*********************************************************************************************************************/
	HypoTestCalculatorGeneric* HypoTestInvTool::CreateCalculator(TestStatistic& testStat)
	{
		HypoTestCalculatorGeneric* hc;
		if(options.CalculatorType == 0) hc = new FrequentistCalculator(*data, *bModel, *sbModel);
		else if(options.CalculatorType == 1) hc = new HybridCalculator(*data, *bModel, *sbModel);
		else if(options.CalculatorType == 2) hc = new AsymptoticCalculator(*data, *bModel, *sbModel, false);
		else if(options.CalculatorType == 3) hc = new AsymptoticCalculator(*data, *bModel, *sbModel, true);  // for using Asimov data generated with nominal values
		else
		{
			Error("HypoTestInvTool::CreateCalculator", "Invalid calculator type = %d supported values are only :\n\t\t\t 0 (Frequentist) , 1 (Hybrid) , 2 (Asymptotic), 3 (Asimov)", options.CalculatorType);
			return 0;
		}
		ToyMCSampler *toymcs = (ToyMCSampler*)hc->GetTestStatSampler();
		if(toymcs && (options.CalculatorType == 0 || options.CalculatorType == 1))
		{
			// look ifpdf is number counting or extended
			if(sbModel->GetPdf()->canBeExtended())
			{
				if(options.UseNumberCounting)   Warning("HypoTestInvTool::CreateCalculator", "PDF is extended: but number counting flag is set: ignore it ");
			}
			else
			{
				// for not extended pdf
				if(!options.UseNumberCounting)
				{
					int nEvents = data->numEntries();
					Info("HypoTestInvTool::CreateCalculator", "Pdf is not extended: number of events to generate taken  from observed data set is %d", nEvents);
					toymcs->SetNEventsPerToy(nEvents);
				}
				else
				{
					Info("HypoTestInvTool::CreateCalculator", "Using a number counting PDF");
					toymcs->SetNEventsPerToy(1);
				}
			}
			toymcs->SetTestStatistic(&testStat);
			if(data->isWeighted() && !options.GenerateBinned)
			{
				Info("HypoTestInvTool::CreateCalculator", "Data set is weighted, nentries = %d and sum of weights = %8.1f but toy generation is unbinned - it would be faster to set options.GenerateBinned to true\n", data->numEntries(), data->sumEntries());
			}
			toymcs->SetGenerateBinned(options.GenerateBinned);
			toymcs->SetUseMultiGen(options.Optimize);
			if(options.GenerateBinned &&  sbModel->GetObservables()->getSize() > 2)
			{
				Warning("HypoTestInvTool::CreateCalculator", "Generate binned is activated but the number of observable is %d. Too much memory could be needed for allocating all the bins", sbModel->GetObservables()->getSize());
			}
			// set the random seed ifneeded
			if(options.RandomSeed >= 0) RooRandom::randomGenerator()->SetSeed(options.RandomSeed);
		}
		if(options.UseProof)
			toymcs->SetProofConfig(&pc);    // enable proof
		// specify ifneed to re-use same toys
		if(options.ReuseAltToys)
			hc->UseSameAltToys();
		if(options.CalculatorType == 0)
		{
			((FrequentistCalculator*) hc)->SetToys(options.NToys, options.NToys/options.NToysRatio);
			// store also the fit information for each poi point used by calculator based on toys
			if(options.EnableDetOutput) ((FrequentistCalculator*) hc)->StoreFitInfo(true);
		}
		else if(options.CalculatorType == 1)
		{
			HybridCalculator *hhc = dynamic_cast<HybridCalculator*> (hc);
			hhc->SetToys(options.NToys, options.NToys/options.NToysRatio); // can use less options.NToys for b hypothesis
			// remove global observables from ModelConfig (this is probably not needed anymore in 5.32)
			bModel->SetGlobalObservables(RooArgSet());
			sbModel->SetGlobalObservables(RooArgSet());
			// check for nuisance prior pdf in case of nuisance parameters
			if(bModel->GetNuisanceParameters() || sbModel->GetNuisanceParameters())
			{
				// fix for using multigen (does not work in this case)
				toymcs->SetUseMultiGen(false);
				ToyMCSampler::SetAlwaysUseMultiGen(false);
				// use prior defined first in bModel (then in SbModel)
				if(nuisPdf == nullptr)
				{
					Info("HypoTestInvTool::CreateCalculator", "No nuisance pdf given for the HybridCalculator - try to deduce pdf from the model");
					if(bModel->GetPdf() && bModel->GetObservables())
						nuisPdf = RooStats::MakeNuisancePdf(*bModel, "nuisancePdf_bmodel");
					else
						nuisPdf = RooStats::MakeNuisancePdf(*sbModel, "nuisancePdf_sbmodel");
				}
				if(nuisPdf == nullptr)
				{
					if(bModel->GetPriorPdf())
					{
						nuisPdf = bModel->GetPriorPdf();
						Info("HypoTestInvTool::CreateCalculator", "No nuisance pdf given - try to use %s that is defined as a prior pdf in the B model", nuisPdf->GetName());
					}
					else
					{
						Error("HypoTestInvTool::CreateCalculator", "Cannot run Hybrid calculator because no prior on the nuisance parameter is specified or can be derived");
						return 0;
					}
				}
				assert(nuisPdf != nullptr);
				Info("HypoTestInvTool::CreateCalculator", "Using as nuisance Pdf ... ");
				nuisPdf->Print();
				const RooArgSet * nuisParams = (bModel->GetNuisanceParameters()) ? bModel->GetNuisanceParameters() : sbModel->GetNuisanceParameters();
				RooArgSet* np = nuisPdf->getObservables(*nuisParams);
				if(np->getSize() == 0)
				{
					Warning("HypoTestInvTool::CreateCalculator", "Prior nuisance does not depend on nuisance parameters. They will be smeared in their full range");
				}
				delete np;
				hhc->ForcePriorNuisanceAlt(*nuisPdf);
				hhc->ForcePriorNuisanceNull(*nuisPdf);
			}
		}
		else if(options.CalculatorType == 2 || options.CalculatorType == 3)
		{
			if(options.TestStatType == 3)
				((AsymptoticCalculator*)hc)->SetOneSided(true);
			if(options.TestStatType != 2 && options.TestStatType != 3)
				Warning("HypoTestInvTool::CreateCalculator", "Only the PL test statistic can be used with AsymptoticCalculator - use by default a two-sided PL");
		}
		return hc;
	}
/*********************************************************************************************************************/
	HypoTestInverterResult* HypoTestInvTool::Rebuild(HypoTestInverterResult* r, HypoTestInverter& calc)
	{
		Info("HypoTestInvTool::Rebuild", "Rebuild the upper limit distribution by re-generating new set of pseudo-experiment and re-compute for each of them a new upper limit");
		RooArgSet* allParams = sbModel->GetPdf()->getParameters(data);
		// define on which value of nuisance parameters to do the rebuild
		// default is best fit value for bmodel snapshot
		if(options.RebuildParamValues != 0)
		{
			// set all parameters to their initial workspace values
			*allParams = initialParameters;
		}
		if(options.RebuildParamValues == 0 || options.RebuildParamValues == 1)
		{
			RooArgSet constrainParams;
			if(sbModel->GetNuisanceParameters()) constrainParams.add(*sbModel->GetNuisanceParameters());
			RooStats::RemoveConstantParameters(&constrainParams);
			const RooArgSet* poiModel = sbModel->GetParametersOfInterest();
			bModel->LoadSnapshot();
			// do a profile using the B model snapshot
			if(options.RebuildParamValues == 0)
			{
				RooStats::SetAllConstant(*poiModel, true);
				sbModel->GetPdf()->fitTo(*data, RooFit::InitialHesse(false), RooFit::Hesse(false), RooFit::Minimizer(options.MinimizerType.c_str(), "Migrad"), RooFit::Strategy(0), RooFit::PrintLevel(options.PrintLevel), RooFit::Constrain(constrainParams), RooFit::Offset(RooStats::IsNLLOffset()));
				Info("HypoTestInvTool::Rebuild", "Rebuild using fitted parameter value for B-model snapshot");
				constrainParams.Print("v");
				RooStats::SetAllConstant(*poiModel, false);
			}
		}
		std::cout << "Initial parameters used for rebuilding: \n";
		RooStats::PrintListContent(*allParams, std::cout);
		delete allParams;
		calc.SetCloseProof(1);
		SamplingDistribution * limDist = calc.GetUpperLimitDistribution(true, options.NToyToRebuild);
		if(limDist)
		{
			std::cout << "Expected limits after rebuild distribution " << "\n";
			std::cout << "expected upper limit  (median of limit distribution) " << limDist->InverseCDF(0.5) << "\n";
			std::cout << "expected -1 sig limit (0.16% quantile of limit dist) " << limDist->InverseCDF(ROOT::Math::normal_cdf(-1)) << "\n";
			std::cout << "expected +1 sig limit (0.84% quantile of limit dist) " << limDist->InverseCDF(ROOT::Math::normal_cdf(1)) << "\n";
			std::cout << "expected -2 sig limit (.025% quantile of limit dist) " << limDist->InverseCDF(ROOT::Math::normal_cdf(-2)) << "\n";
			std::cout << "expected +2 sig limit (.975% quantile of limit dist) " << limDist->InverseCDF(ROOT::Math::normal_cdf(2)) << "\n";
			// Plot the upper limit distribution
			SamplingDistPlot limPlot((options.NToyToRebuild < 200) ? 50 : 100);
			limPlot.AddSamplingDistribution(limDist);
			limPlot.GetTH1F()->SetStats(true); // display statistics
			limPlot.SetLineColor(kBlue);
			TCanvas limcan("limPlot", "Upper Limit Distribution");
			limPlot.Draw();
			/// save result in a file
			limDist->SetName("RULDist");
			TFile fileOut("RULDist.root", "RECREATE");
			limcan.Write();
			limDist->Write();
			fileOut.Close();
			//update r to a new updated result object containing the rebuilt expected p-values distributions
			// (it will not recompute the expected limit)
			if(r) delete r;  // need to delete previous object since GetInterval will return a cloned copy
			r = calc.GetInterval();
		}
		else
			Error("HypoTestInvTool::Rebuild", "Failed to re-build distributions");
		return r;
	}
/*********************************************************************************************************************/
	// analyze result produced by the inverter, optionally save it in a file
	void HypoTestInvTool::AnalyzeResult(HypoTestInverterResult& r, std::string fileNameBase)
	{
		double lowerLimit = 0;
		double llError = 0;
		if(r.IsTwoSided())
		{
			lowerLimit = r.LowerLimit();
			llError = r.LowerLimitEstimatedError();
		}
		double upperLimit = r.UpperLimit();
		double ulError = r.UpperLimitEstimatedError();
		if(lowerLimit < upperLimit*(1.- 1.E-4) && lowerLimit != 0)
			std::cout << "The computed lower limit is: " << lowerLimit << " +/- " << llError << "\n";
		std::cout << "The computed upper limit is: " << upperLimit << " +/- " << ulError << "\n";
		// compute expected limit
		std::cout << "Expected upper limits, using the B (alternate) model : " << "\n";
		std::cout << " expected limit (median) " << r.GetExpectedUpperLimit(0) << "\n";
		std::cout << " expected limit (-1 sig) " << r.GetExpectedUpperLimit(-1) << "\n";
		std::cout << " expected limit (+1 sig) " << r.GetExpectedUpperLimit(1) << "\n";
		std::cout << " expected limit (-2 sig) " << r.GetExpectedUpperLimit(-2) << "\n";
		std::cout << " expected limit (+2 sig) " << r.GetExpectedUpperLimit(2) << "\n";
		// detailed output
		if(options.EnableDetOutput)
		{
			options.WriteResult=true;
			Info("HypoTestInvTool::AnalyzeResult", "Detailed output will be written in output result file");
		}
		// write result in a file
		if(options.WriteResult)
		{
			WriteResults(r, fileNameBase);
		}
		// plot test statistics distributions for the two hypothesis
		if(options.PlotHypoTestResult)
		{
			PlotResults(r);
		}
	}
/*********************************************************************************************************************/
	void HypoTestInvTool::WriteResults(HypoTestInverterResult& r, std::string fileNameBase)
	{
		// write to a file the results
		std::string calcType = (options.CalculatorType == 0) ? "Freq" : (options.CalculatorType == 1) ? "Hybr" : "Asym";
		std::string limitType = (options.UseCLs) ? "CLs" : "Cls+b";
		std::string scanType = (options.NPoints < 0) ? "auto" : "grid";
		if(options.ResultFileName.empty())
		{
			options.ResultFileName = calcType+"_"+limitType+"_"+scanType+"_ts"+std::to_string(options.TestStatType)+"_";
			//strip the / from the filename
			if(options.MassValue.size()>0)
			{
				options.ResultFileName += options.MassValue.c_str();
				options.ResultFileName += "_";
			}
			std::string name = fileNameBase;
			name.replace(0, name.find_last_of('/')+1, "");
			options.ResultFileName += name;
		}
		TFile fileOut(options.ResultFileName.c_str(), "RECREATE");
		r.Write();
		Info("HypoTestInvTool::WriteResults", "HypoTestInverterResult has been written in the file %s", options.ResultFileName.c_str());
		fileOut.Close();
	}
/*********************************************************************************************************************/
	void HypoTestInvTool::PlotResults(HypoTestInverterResult& r)
	{
		// plot the result (p values vs scan points)
		std::string typeName;
		if(options.CalculatorType == 0)
			typeName = "Frequentist";
		if(options.CalculatorType == 1)
			typeName = "Hybrid";
		else if(options.CalculatorType == 2 || options.CalculatorType == 3)
		{
			typeName = "Asymptotic";
			Warning("HypoTestInvTool::PlotResults", "Cannot plot this CalculatorType: %s (%d)", typeName.c_str(), options.CalculatorType);
			return;
		}
		else
			Warning("HypoTestInvTool::PlotResults", "Not sure what to do with CalculatorType %d", options.CalculatorType);
		std::string resultName = r.GetName();
		std::string plotTitle = typeName+" CL Scan for workspace "+resultName;
		HypoTestInverterPlot plot("HTI_Result_Plot", plotTitle.c_str(), &r);
		// plot in a new canvas with style
		std::string c1Name = typeName+"_Scan";
		TCanvas c1(c1Name.c_str());
		c1.SetLogy(true);
		plot.Draw("CLb 2CL");  // plot all and Clb
		c1.SaveAs(c1.GetName(),"ROOT");
		const int nEntries = r.ArraySize();
		TCanvas c2("Test_Stat_Plots");
		if(nEntries > 1)
		{
			int ny = TMath::CeilNint(TMath::Sqrt(nEntries));
			int nx = TMath::CeilNint(double(nEntries)/ny);
			c2.Divide(nx, ny);
		}
		for (int i=0; i<nEntries; i++)
		{
			if(nEntries > 1) c2.cd(i+1);
			SamplingDistPlot* pl = plot.MakeTestStatPlot(i);
			pl->SetLogYaxis(true);
			pl->Draw();
		}
		c2.SaveAs(c2.GetName(),"ROOT");
	}
} // end namespace RooStats

