// Std
#include <iostream>
// BOOST
#include "boost/program_options.hpp"
// Local
#include "StandardHypoTestInverter.h"

int main(int argc, char* argv[])
{
	RooStats::HypoTestInvOptions optHTInv;
	std::string infile;
	std::string wsName;
	std::string modelSBName;
	std::string modelBName;
	std::string dataName;
	std::string nuisPriorName;
	boost::program_options::options_description desc("Allowed options",120);
	desc.add_options()
		("help", "produce help message")
		("EnableDetOutput", boost::program_options::value<bool>(&optHTInv.EnableDetOutput)->default_value(false), "enable detailed output with all fit information for each toys (output will be written in result file)")
		("PlotHypoTestResult", boost::program_options::value<bool>(&optHTInv.PlotHypoTestResult)->default_value(true), "plot test statistic result at each point")
		("WriteResult", boost::program_options::value<bool>(&optHTInv.WriteResult)->default_value(true), "write HypoTestInverterResult in a file")
		("PrintLevel", boost::program_options::value<int>(&optHTInv.PrintLevel)->default_value(0), "print level for debugging PL test statistics and calculators")
		("ConfLevel", boost::program_options::value<double>(&optHTInv.ConfLevel)->default_value(0.95), "confidence level value")
		("MassValue", boost::program_options::value<std::string>(&optHTInv.MassValue)->default_value(""), "extra string to tag output file of result")
		("ResultFileName", boost::program_options::value<std::string>(&optHTInv.ResultFileName)->default_value(""), "file with results (by default is built automatically using the workspace input file name)")
		("UseCLs", boost::program_options::value<bool>(&optHTInv.UseCLs)->default_value(true), "scan for CLs (otherwise for CLs+b)")
		("InitialFit", boost::program_options::value<int>(&optHTInv.InitialFit)->default_value(-1), "do a first  fit to the model (-1 : default, 0 skip fit, 1 do always fit)")
		("NPoints", boost::program_options::value<int>(&optHTInv.NPoints)->default_value(6), "number of points to scan , for autoscan set npoints = -1")
		("MaxPoi", boost::program_options::value<double>(&optHTInv.MaxPoi)->default_value(-1), "max value used of POI (in case of auto scan)")
		("NToysRatio", boost::program_options::value<double>(&optHTInv.NToysRatio)->default_value(2), "ratio Ntoys(S+B)/Ntoys(B)")
		("PoiMin", boost::program_options::value<double>(&optHTInv.PoiMin)->default_value(0), "min/max value to scan in case of fixed scans")
		("PoiMax", boost::program_options::value<double>(&optHTInv.PoiMax)->default_value(5), "(if min > max, try to find automatically)")
		("UseProof", boost::program_options::value<bool>(&optHTInv.UseProof)->default_value(false), "use Proof Lite when using toys (for freq or hybrid)")
		("ReuseAltToys", boost::program_options::value<bool>(&optHTInv.ReuseAltToys)->default_value(false), "reuse same toys for alternate hypothesis (if set one gets more stable bands)")
		("NToys", boost::program_options::value<int>(&optHTInv.NToys)->default_value(1000), "number of toys per point")
		("NWorkers", boost::program_options::value<int>(&optHTInv.NWorkers)->default_value(0), "number of worker for ProofLite (default use all available cores)")
		("RandomSeed", boost::program_options::value<int>(&optHTInv.RandomSeed)->default_value(-1), "random seed (if = -1: use default value, if = 0 always random ) NOTE: Proof uses automatically a random seed")
		("Rebuild", boost::program_options::value<bool>(&optHTInv.Rebuild)->default_value(false), "re-do extra toys for computing expected limits and rebuild test stat distributions (N.B this requires much more CPU (factor is equivalent to nToyToRebuild)")
		("NToyToRebuild", boost::program_options::value<int>(&optHTInv.NToyToRebuild)->default_value(100), "number of toys used to rebuild")
		("RebuildParamValues", boost::program_options::value<int>(&optHTInv.RebuildParamValues)->default_value(0), "= 0 do a profile of all the parameters on the B (alt snapshot) before performing a rebuild operation (default)\n= 1 use initial workspace parameters with B snapshot values\n= 2 use all initial workspace parameters with B\nOtherwise the rebuild will be performed using [text missing, I assume it works like 2]")
		("GenerateBinned", boost::program_options::value<bool>(&optHTInv.GenerateBinned)->default_value(false), "generate binned data sets")
		("NoSystematics", boost::program_options::value<bool>(&optHTInv.NoSystematics)->default_value(false), "force all systematics to be off (i.e. set all nuisance parameters as constant to their nominal values)")
		("Optimize", boost::program_options::value<bool>(&optHTInv.Optimize)->default_value(true), "optimize evaluation of test statistic")
		("UseNLLOffset", boost::program_options::value<bool>(&optHTInv.UseNLLOffset)->default_value(false), "use NLL offset when fitting (this increase stability of fits)")
		("UseVectorStore", boost::program_options::value<bool>(&optHTInv.UseVectorStore)->default_value(true), "convert data to use new roofit data store")
		("CalculatorType", boost::program_options::value<int>(&optHTInv.CalculatorType)->default_value(0), "= 0 Freq calculator\n= 1 Hybrid calculator\n= 2 Asymptotic calculator\n= 3 Asymptotic calculator using nominal Asimov data sets (not using fitted parameter values but nominal ones)")
		("NAsimovBins", boost::program_options::value<int>(&optHTInv.NAsimovBins)->default_value(0), "number of bins in observables used for Asimov data sets (0 is the default and it is given by workspace, typically is 100)")
		("TestStatType", boost::program_options::value<int>(&optHTInv.TestStatType)->default_value(2), "= 0 LEP\n= 1 Tevatron\n= 2 Profile Likelihood\n= 3 Profile Likelihood one sided (i.e. = 0 if mu < mu_hat)\n= 4 Profile Likelihood signed ( pll = -pll if mu < mu_hat)\n= 5 Max Likelihood Estimate as test statistic\n= 6 Number of observed event as test statistic")
		("UseNumberCounting", boost::program_options::value<bool>(&optHTInv.UseNumberCounting)->default_value(false), "set to true when using number counting events")
		("MinimizerType", boost::program_options::value<std::string>(&optHTInv.MinimizerType)->default_value(""), "minimizer type (default is what is in ROOT::Math::MinimizerOptions::DefaultMinimizerType()")
		("InputFile", boost::program_options::value<std::string>(&infile)->required(), "input file")
		("Workspace", boost::program_options::value<std::string>(&wsName)->required(), "workspace name")
		("ModelSB", boost::program_options::value<std::string>(&modelSBName)->required(), "signal+background model name")
		("ModelB", boost::program_options::value<std::string>(&modelBName)->default_value(""), "optional background model name")
		("Data", boost::program_options::value<std::string>(&dataName)->required(), "data name")
		("NuisPrior", boost::program_options::value<std::string>(&nuisPriorName)->default_value(""), "optional nuisance prior name")
	;
	boost::program_options::positional_options_description positionalOptions;
	positionalOptions.add("InputFile",1);
	positionalOptions.add("Workspace",1);
	positionalOptions.add("ModelSB",1);
	positionalOptions.add("Data",1);
	boost::program_options::variables_map vmap;
	boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).positional(positionalOptions).run(), vmap);
	boost::program_options::notify(vmap);
	if(vmap.count("help") || argc == 1)
	{
		std::cout << desc << "\n";
		return 1;
	}
	StandardHypoTestInverter(infile, wsName, modelSBName, modelBName, dataName, nuisPriorName, optHTInv);
	return 0;
}
