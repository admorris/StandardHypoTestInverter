# StandardHypoTestInverter
A more sensible implementation of a RooStats demo:

https://root.cern.ch/doc/v608/StandardHypoTestInvDemo_8C.html

## Requirements

- Boost program options
- ROOT v6
- RooFit
- RooStats
- PROOF

## Building

Type `make`. If it fails, make sure you have all the requirements installed.
If you don't have Boost installed, you'll have to re-write `src/main.cpp` yourself and find some other way to set the options.
If you still need help, [contact me](https://github.com/abmorris).

In principle you can type `make bin/whatever` to rename the binary, if you please. By defailt it produces one called `bin/main`

## Input

You must provide a `RooWorkSpace` inside a `.root`. It must contain:
- a `ModelConfig` for signal+background
- a defined observable and parameter of interest
- a `RooDataSet` containing the data
- (optional) a `ModelConfig` for background
- (optional) a prior PDF with nuisance parameters

## Usage

The syntax is `bin/main InputFile Workspace ModelSB Data [Options]`

Command line option                  | Description
-------------------------------------|-------------
`--ModelB`                           | optional background model name
`--NuisPrior`                        | optional nuisance prior name
`--EnableDetOutput`<br>(default 0)      | enable detailed output with all fit information for each toys (output will be written in result file)
`--PlotHypoTestResult`<br>(default 1)   | plot test statistic result at each point
`--WriteResult`<br>(default 1)          | write HypoTestInverterResult in a file
`--PrintLevel`<br>(default 0)           | print level for debugging PL test statistics and calculators
`--ConfLevel`<br>(default 0.95)         | confidence level value
`--MassValue`                        | extra string to tag output file of result
`--ResultFileName`                   | file with results (by default is built automatically using the workspace input file name)
`--UseCLs`<br>(default 1)               | scan for CLs (otherwise for CLs+b)
`--InitialFit`<br>(default -1)          | do a first fit to the model (-1 : default, 0 skip fit, 1 do always fit)
`--NPoints`<br>(default 6)              | number of points to scan , for autoscan set npoints = -1
`--MaxPoi`<br>(default -1)              | max value used of POI (in case of auto scan)
`--NToysRatio`<br>(default 2)           | ratio Ntoys(S+B)/Ntoys(B)
`--PoiMin`<br>(default 0)               | min/max value to scan in case of fixed scans
`--PoiMax`<br>(default 5)               | (if min > max, try to find automatically)
`--UseProof`<br>(default 1)             | use Proof Lite when using toys (for freq or hybrid)
`--ReuseAltToys`<br>(default 0)         | reuse same toys for alternate hypothesis (if set one gets more stable bands)
`--NToys`<br>(default 1000)             | number of toys per point
`--NWorkers`<br>(default 0)             | number of worker for ProofLite<br>(default use all available cores)
`--RandomSeed`<br>(default -1)          | random seed (if = -1: use default value, if = 0 always random ) NOTE: Proof uses automatically a random seed
`--Rebuild`<br>(default 0)              | re-do extra toys for computing expected limits and rebuild test stat distributions (N.B this requires much more CPU (factor is equivalent to nToyToRebuild)
`--NToyToRebuild`<br>(default 100)      | number of toys used to rebuild
`--RebuildParamValues`<br>(default 0)   | = 0 do a profile of all the parameters on the B (alt snapshot) before performing a rebuild operation<br>(default)<br> = 1 use initial workspace parameters with B snapshot values<br> = 2 use all initial workspace parameters with B<br> Otherwise the rebuild will be performed using [text missing, I assume it works like 2]
`--GenerateBinned`<br>(default 0)       | generate binned data sets
`--NoSystematics`<br>(default 0)        | force all systematics to be off (i.e. set all nuisance parameters as constant to their nominal values)
`--Optimize`<br>(default 1)             | optimize evaluation of test statistic
`--UseNLLOffset`<br>(default 0)         | use NLL offset when fitting (this increase stability of fits)
`--UseVectorStore`<br>(default 1)       | convert data to use new roofit data store
`--CalculatorType`<br>(default 0)       | = 0 Freq calculator<br> = 1 Hybrid calculator<br> = 2 Asymptotic calculator<br> = 3 Asymptotic calculator using nominal Asimov data sets (not using fitted parameter values but nominal ones)
`--NAsimovBins`<br>(default 0)          | number of bins in observables used for Asimov data sets (0 is the default and it is given by workspace, typically is 100)
`--TestStatType`<br>(default 2)         | = 0 LEP<br> = 1 Tevatron<br> = 2 Profile Likelihood<br> = 3 Profile Likelihood one sided (i.e. = 0 if mu < mu_hat)<br> = 4 Profile Likelihood signed ( pll = -pll if mu < mu_hat)<br> = 5 Max Likelihood Estimate as test statistic<br> = 6 Number of observed event as test statistic
`--UseNumberCounting`<br>(default 0)    | set to true when using number counting events
`--MinimizerType`                    | minimizer type<br>(default is what is in ROOT::Math::MinimizerOptions::DefaultMinimizerType()


## Using custom RooFit classes

The folder called `ExtraRooFit/` is designed to accomodate additional custom RooFit classes.
It comes with `RooStudentT` as an example.
In order to add one of your own, place the header (ending in `.h`) in `ExtraRooFit/include/` and the source (ending in `.cpp`) in `ExtraRooFit/src/`, then add a line in `ExtraRooFit/include/LinkDef.h`:

    #pragma link C++ class RooCustomPDF+;

where you substitute `RooCustomPDF` for the appropriate class name.
The file right extensions are important, as they need to be picked up in the Makefile.
