// Adapted from https://root.cern.ch/doc/v608/StandardHypoTestInvDemo_8C.html
#pragma once
// Std
#include <string>
// structure defining the options
namespace RooStats
{
	struct HypoTestInvOptions
	{
		bool PlotHypoTestResult = true; // plot test statistic result at each point
		bool WriteResult = true;        // write HypoTestInverterResult in a file
		std::string ResultFileName;     // file with results (by default is built automatically using the workspace input file name)
		bool Optimize = true;           // optimize evaluation of test statistic
		bool UseVectorStore = true;     // convert data to use new roofit data store
		bool GenerateBinned = false;    // generate binned data sets
		bool NoSystematics = false;     // force all systematics to be off (i.e. set all nuisance parameters as constant to their nominal values)
		double NToysRatio = 2;          // ratio Ntoys S+b/ntoysB
		double MaxPoi = -1;             // max value used of POI (in case of auto scan)
		bool UseProof = false;          // use Proof Lite when using toys (for freq or hybrid)
		int NWorkers = 0;               // number of worker for ProofLite (default use all available cores)
		bool EnableDetOutput = false;   // enable detailed output with all fit information for each toys (output will be written in result file)
		bool Rebuild = false;           // re-do extra toys for computing expected limits and rebuild test stat distributions (N.B this requires much more CPU (factor is equivalent to nToyToRebuild)
		int NToyToRebuild = 100;        // number of toys used to rebuild
		int RebuildParamValues=0;       // = 0 do a profile of all the parameters on the B (alt snapshot) before performing a rebuild operation (default)
		                                // = 1 use initial workspace parameters with B snapshot values
		                                // = 2 use all initial workspace parameters with B
		                                // Otherwise the rebuild will be performed using [text missing, I assume it works like 2]
		int InitialFit = -1;            // do a first  fit to the model (-1 : default, 0 skip fit, 1 do always fit)
		int RandomSeed = -1;            // random seed (if = -1: use default value, if = 0 always random ) NOTE: Proof uses automatically a random seed
		int NAsimovBins = 0;            // number of bins in observables used for Asimov data sets (0 is the default and it is given by workspace, typically is 100)
		bool ReuseAltToys = false;      // reuse same toys for alternate hypothesis (if set one gets more stable bands)
		double ConfLevel = 0.95;        // confidence level value
		std::string MinimizerType = ""; // minimizer type (default is what is in ROOT::Math::MinimizerOptions::DefaultMinimizerType()
		std::string MassValue = "";     // extra string to tag output file of result
		int PrintLevel = 0;             // print level for debugging PL test statistics and calculators
		bool UseNLLOffset = false;      // use NLL offset when fitting (this increase stability of fits)
		int CalculatorType = 0;         // = 0 Freq calculator
		                                // = 1 Hybrid calculator
		                                // = 2 Asymptotic calculator
		                                // = 3 Asymptotic calculator using nominal Asimov data sets (not using fitted parameter values but nominal ones)
		int TestStatType = 2;           // = 0 LEP
		                                // = 1 Tevatron
		                                // = 2 Profile Likelihood
		                                // = 3 Profile Likelihood one sided (i.e. = 0 if mu < mu_hat)
		                                // = 4 Profile Likelihood signed ( pll = -pll if mu < mu_hat)
		                                // = 5 Max Likelihood Estimate as test statistic
		                                // = 6 Number of observed event as test statistic
		bool UseCLs = true;             // scan for CLs (otherwise for CLs+b)
		int NPoints = 6;                // number of points to scan , for autoscan set npoints = -1
		double PoiMin = 0;              // min/max value to scan in case of fixed scans
		double PoiMax = 5;              // (if min > max, try to find automatically)
		int NToys = 1000;               // number of toys per point
		bool UseNumberCounting = false; // set to true when using number counting events
	};
} // end namespace RooStats

