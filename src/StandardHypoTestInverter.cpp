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
