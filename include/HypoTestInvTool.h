// Adapted from https://root.cern.ch/doc/v608/StandardHypoTestInvDemo_8C.html
#pragma once
// Std
#include <string>
// RooStats
#include "RooStats/HypoTestInverter.h"
#include "RooStats/HypoTestInverterResult.h"
#include "RooStats/ToyMCSampler.h"
// Local
#include "HypoTestInvOptions.h"
// internal class to run the inverter and more
namespace RooStats
{
	class HypoTestInvTool
	{
		public:
			HypoTestInvTool(RooWorkspace& workspace, std::string modelSBName, std::string modelBName, std::string dataName, std::string nuisPriorName, const HypoTestInvOptions& _options);
			~HypoTestInvTool(){};
			HypoTestInverterResult* RunInverter();
			void AnalyzeResult(HypoTestInverterResult& r, std::string fileNameBase);
		private:
			void InitialFit(RooRealVar& poi);
			TestStatistic* CreateTestStat(RooRealVar& poi);
			HypoTestCalculatorGeneric* CreateCalculator(TestStatistic& testStat);
			HypoTestInverterResult* Rebuild(HypoTestInverterResult* r, HypoTestInverter& calc);
			void PlotResults(HypoTestInverterResult& r);
			void WriteResults(HypoTestInverterResult& r, std::string fileNameBase);
			HypoTestInvOptions options;
			ModelConfig* bModel;
			ModelConfig* sbModel;
			RooAbsPdf* nuisPdf;
			RooAbsData* data;
			ProofConfig pc;
			RooArgSet initialParameters;
	};
} // end namespace RooStats

