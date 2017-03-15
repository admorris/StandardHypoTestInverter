// Adapted from https://root.cern.ch/doc/v608/StandardHypoTestInvDemo_8C.html
#pragma once
// Std
#include <string>
// structure defining the options
namespace RooStats
{
	struct HypoTestInvOptions
	{
		// Output options
		bool EnableDetOutput;
		bool PlotHypoTestResult;
		bool WriteResult;
		int PrintLevel;
		double ConfLevel;
		std::string MassValue;
		std::string ResultFileName;
		// Scan options
		bool UseCLs;
		int InitialFit;
		int NPoints;
		double MaxPoi;
		double NToysRatio;
		double PoiMin;
		double PoiMax;
		// Toy options
		bool UseProof;
		bool ReuseAltToys;
		int NToys;
		int NWorkers;
		int RandomSeed;
		// Rebuild options
		bool Rebuild;
		int NToyToRebuild;
		int RebuildParamValues;
		// Calculation options
		bool GenerateBinned;
		bool NoSystematics;
		bool Optimize;
		bool UseNLLOffset;
		bool UseVectorStore;
		int CalculatorType;
		int NAsimovBins;
		int TestStatType;
		bool UseNumberCounting;
		std::string MinimizerType;
	};
} // end namespace RooStats

