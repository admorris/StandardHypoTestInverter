// ROOT
#include "TFile.h"
// Local
#include "HypoTestInvTool.h"
// Adapted from https://root.cern.ch/doc/v608/StandardHypoTestInvDemo_8C.html
void StandardHypoTestInverter(std::string infile, std::string wsName, std::string modelSBName, std::string modelBName, std::string dataName, std::string nuisPriorName, RooStats::HypoTestInvOptions& optHTInv)
{
	// Try to open the file
	TFile *file = TFile::Open(infile.c_str());
	// if input file was specified byt not found, quit
	if(!file)
	{
		Error("StandardHypoTestInverter", "Input file %s is not found", infile.c_str());
		return;
	}
	// Set options using the HypoTestInvOptions struct. For the full set of options with their default values, see the HypoTestInvOptions.h header file
	// The really important options are
	//	UseCLs             : scan for CLs (otherwise for CLs+b)
	//	NPoints            : number of points to scan, for autoscan set npoints = -1
	//	PoiMin             : min/max value to scan in case of fixed scans
	//	PoiMax             : (if min > max, try to find automatically)
	//	NToys              : number of toys per point
	//	UseNumberCounting  : set to true when using number counting events
	// Other major options are
	//	PlotHypoTestResult : plot result of tests at each point (TS distributions) (default is true)
	//	UseProof           : use Proof   (default is true)
	//	WriteResult        : write result of scan (default is true)
	//	Rebuild            : rebuild scan for expected limits (require extra toys) (default is false)
	//	GenerateBinned     : generate binned data sets for toys (default is false) - be careful not to activate with a too large (>=3) number of observables
	//	NToyRatio          : ratio of S+B/B toys (default is 2)
	// Enable offset for all roostats
	RooStats::UseNLLOffset(optHTInv.UseNLLOffset);
	// Load the workspace from the file
	RooWorkspace* w = dynamic_cast<RooWorkspace*>(file->Get(wsName.c_str()));
	// Construct the object which will do all the work
	RooStats::HypoTestInvTool calc(*w, modelSBName, modelBName, dataName, nuisPriorName, optHTInv);
	// Run the HypoTestInverter
	RooStats::HypoTestInverterResult * r = calc.RunInverter();
	if(!r)
	{
		Error("StandardHypoTestInverter", "The HypoTestInverter did not return a result object.");
		return;
	}
	else
	{
		// If workspace is not present, look for the inverter result
		Info("StandardHypoTestInverter", "Reading an HypoTestInverterResult with name %s from file %s", wsName.c_str(), infile.c_str());
		r = dynamic_cast<RooStats::HypoTestInverterResult*>(file->Get(wsName.c_str()));
		if(!r)
		{
			Error("StandardHypoTestInverter", "File %s does not contain a workspace or an HypoTestInverterResult", infile.c_str());
			file->ls();
			return;
		}
	}
	calc.AnalyzeResult( *r, infile );
	return;
}
