// Std
#include <string>
// Adapted from https://root.cern.ch/doc/v608/StandardHypoTestInvDemo_8C.html
void StandardHypoTestInverter(
	std::string infile = "",                  // Filename to load the workspace from
	std::string wsName = "combined",          // Name of the workspace object in the file
	std::string modelSBName = "ModelConfig",  // Name of the signal+background PDF
	std::string modelBName = "",              // Name of the background PDF
	std::string dataName = "obsData",         // Name of the dataset object
	std::string nuisPriorName = ""            // Name of prior for the nuisance when using the HybridCalculator. This is often expressed as constraint term in the global model. Defaults to the prior PDF from ModelConfig.
);
