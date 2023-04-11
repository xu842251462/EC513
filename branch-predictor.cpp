
#include <iostream>

#include "pin.H"
#include "instlib.H"


using namespace std;

#include "TWOLEVEL.H"

static TWOLEVEL bimodal;

using namespace INSTLIB;

static ofstream *outfile;

#define KNOB_FAMILY "pintool:branch-predictor"


KNOB<string>KnobStatFileName(KNOB_MODE_WRITEONCE,  "pintool",
                             "statfile", "bimodal.out", "Name of the branch predictor stats file.");

INT32 Usage()
{
    cerr << "This tool models a bimodal branch-predictor defined in bimodal.H" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

// To guard against running a multi-threaded program
static void threadCreated(THREADID threadIndex, CONTEXT *, INT32 , VOID *)
{
    if (threadIndex > 0)
    {
        cerr << "More than one thread detected. This tool currently works only for single-threaded programs/pinballs." << endl;
        exit(0);
    }
}


int main(int argc, char *argv[])
{
    if (PIN_Init(argc, argv)) return Usage(); // Return if error while parsing arguments
    PIN_InitSymbols();

    outfile = new ofstream(KnobStatFileName.Value().c_str());
    bimodal.Activate(outfile);

    // To guard against running a multi-threaded program
    PIN_AddThreadStartFunction(threadCreated, reinterpret_cast<void *>(0));

    PIN_StartProgram();
}



