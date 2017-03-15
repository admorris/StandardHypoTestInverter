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

Command line option | Description
--- | ---
--ModelB                           | optional background model name
## Using custom RooFit classes

The folder called `ExtraRooFit/` is designed to accomodate additional custom RooFit classes.
It comes with `RooStudentT` as an example.
In order to add one of your own, place the header (ending in `.h`) in `ExtraRooFit/include/` and the source (ending in `.cpp`) in `ExtraRooFit/src/`, then add a line in `ExtraRooFit/include/LinkDef.h`:

    #pragma link C++ class RooCustomPDF+;

where you substitute `RooCustomPDF` for the appropriate class name.
The file right extensions are important, as they need to be picked up in the Makefile.
