## Using custom RooFit classes

The folder `ExtraRooFit/` is designed to accomodate additional custom RooFit classes.
It comes with `RooStudentT` as an example.
In order to add one of your own, place the header (ending in `.h`) in `ExtraRooFit/include/` and the source (ending in `.cpp`) in `ExtraRooFit/src/`, then add a line in `ExtraRooFit/include/LinkDef.h`:

    #pragma link C++ class RooCustomPDF+;

where you substitute `RooCustomPDF` for the appropriate class name.
The right file extensions are important, as they need to be picked up in the Makefile.
