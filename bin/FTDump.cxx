//
// A diagnostics program that will dump
// out and check the input files
//

#include "Combination/Parser.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/BinBoundaryUtils.h"

#include <vector>
#include <set>
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace BTagCombination;

// Helper routines forward defined.
void Usage(void);
void DumpEverything (const vector<CalibrationAnalysis> &calibs);
void CheckEverything (const CalibrationInfo &info);
void PrintNames (const CalibrationInfo &info);
void PrintQNames (const CalibrationInfo &info);

// Main program - run & control everything.
int main (int argc, char **argv)
{
  if (argc <= 1) {
    Usage();
    return 1;
  }

  try {
    // Parse the input arguments
    CalibrationInfo info;
    vector<string> otherFlags;
    ParseOPInputArgs ((const char**)&(argv[1]), argc-1, info, otherFlags);

    bool doCheck = false;
    bool doDump = true;
    bool doNames = false;
    bool doQNames = false;

    for (unsigned int i = 0; i < otherFlags.size(); i++) {
      if (otherFlags[i] == "check") {
	doCheck = true;
	doDump = false;
      } else if (otherFlags[i] == "names") {
	doDump = false;
	doNames = true;
	doQNames = false;
      } else if (otherFlags[i] == "qnames") {
	doDump = false;
	doQNames = true;
	doNames = false;
      } else {
	cerr << "Unknown command line option --" << otherFlags[i] << endl;
	Usage();
	return 1;
      }
    }

    const vector<CalibrationAnalysis> &calibs(info.Analyses);

    // Dump out a list of comma seperated values
    if (doDump)
      DumpEverything (calibs);

    // Check to see if there are overlapping bins
    if (doCheck)
      CheckEverything(info);

    if (doNames)
      PrintNames(info);

    if (doQNames)
      PrintQNames (info);

    // Check to see if the bin specifications are consistent.
    return 0;

  } catch (exception &e) {
    cerr << "Error: " << e.what() << endl;
    return 1;
  }

  return 0;
}

//
// Hold onto a single ana/bin - makes sorting and otherwise
// dealing with this a bit simpler in the code.
//
class holder
{
public:
  holder (const CalibrationAnalysis &ana, const CalibrationBin &bin)
    : _ana(ana), _bin(bin) {}
  
  inline string name() const {return OPFullName(_ana);}
  inline string binName() const {return OPBinName(_bin);}
  inline vector<string> sysErrorNames() const
  {
    vector<string> r;
    for (unsigned int i = 0; i < _bin.systematicErrors.size(); i++)
      r.push_back(_bin.systematicErrors[i].name);
    return r;
  }
  inline bool hasSysError(const string &name) const
  {
    for (unsigned int i = 0; i < _bin.systematicErrors.size(); i++)
      if (_bin.systematicErrors[i].name == name)
	return true;
    return false;
  }
  inline double sysError(const string &name) const
  {
    for (unsigned int i = 0; i < _bin.systematicErrors.size(); i++)
      if (_bin.systematicErrors[i].name == name)
	return _bin.systematicErrors[i].value;
    throw runtime_error (string("sys error '") + name + "' not known!");
  }
private:
  CalibrationAnalysis _ana;
  CalibrationBin _bin;
};

//
// Generate a comma seperated list of csv values.
//
void DumpEverything (const vector<CalibrationAnalysis> &calibs)
{
    vector<holder> held;
    for (unsigned int i = 0; i < calibs.size(); i++)
      for (unsigned int b = 0; b < calibs[i].bins.size(); b++)
	held.push_back(holder(calibs[i], calibs[i].bins[b]));

    // Get a complete list of all systematic errors!
    set<string> allsyserrors;
    for (unsigned int i = 0; i < held.size(); i++) {
      vector<string> binerr (held[i].sysErrorNames());
      allsyserrors.insert(binerr.begin(), binerr.end());
    }

    // Now that we are parsed, dump a comma seperated output to stdout...
    // hopefully this can be c/p into a excel file for nicer formatting.

    // Line1: analysis name headers
    for (unsigned int i = 0; i < held.size(); i++) {
      cout << "," << held[i].name() << " " << held[i].binName();
    }
    cout << endl;

    // Do a line for everything now...
    for (set<string>::const_iterator i = allsyserrors.begin(); i != allsyserrors.end(); i++) {
      cout << *i;
      for (unsigned int h = 0; h < held.size(); h++) {
	cout << ",";
	if (held[h].hasSysError(*i))
	  cout << held[h].sysError(*i);
      }
      cout << endl;
    }
}

//
// Make sure there are no overlapping bins, etc. for each
// analysis.
//
void CheckEverything (const vector<CalibrationAnalysis> &calibs)
{
  //
  // Split up everything by the analysis we are going to be done
  //

  typedef map<string, vector<CalibrationAnalysis> > t_CalibList;
  t_CalibList byBin (BinAnalysesByJetTagFlavOp(calibs));
  for (t_CalibList::const_iterator itr = byBin.begin(); itr != byBin.end(); itr++) {
    const vector<CalibrationAnalysis> anas(itr->second);
    
    //
    // Calculating boundaries will make sure each analysis
    // has a fully consitent set of bin boundaries
    //

    vector<bin_boundaries> bb;
    for (unsigned int i = 0; i < anas.size(); i++) {
      bb.push_back(calcBoundaries(anas[i]));
    }
  
    //
    // Next we check different analysis have consistent bins.
    //
  
    checkForConsitentBoundaries(bb);

    //
    // See if the various calibratoins are consistent for other reasons...
    //

    checkForConsistentAnalyses(anas);
  }
}

void CheckEverything (const CalibrationInfo &info)
{
  CheckEverything(info.Analyses);
  checkForValidCorrelations(info);
}

void PrintNames (const vector<CalibrationAnalysis> &calibs, bool ignoreFormat = true)
{
  for (unsigned int i = 0; i < calibs.size(); i++) {
    for (unsigned int b = 0; b < calibs[i].bins.size(); b++) {
      if (ignoreFormat) {
	cout << OPIgnoreFormat(calibs[i], calibs[i].bins[b]) << endl;
      } else {
	cout << OPComputerFormat(calibs[i], calibs[i].bins[b]) << endl;
      }
    }
  }
}

void PrintNames (const vector<AnalysisCorrelation> &cors)
{
  for (size_t i = 0; i < cors.size(); i++) {
    for (size_t b = 0; b < cors[i].bins.size(); b++) {
      cout << OPIgnoreFormat(cors[i], cors[i].bins[b]) << endl;
    }
  }
}

void PrintNames (const CalibrationInfo &info)
{
  PrintNames(info.Analyses);
  PrintNames(info.Correlations);
}

void PrintQNames (const CalibrationInfo &info)
{
  PrintNames(info.Analyses, false);
}

void Usage(void)
{
  cout << "FTDump <file-list-and-options>" << endl;
  cout << "  --check - check if the binning of the input is self consistent" << endl;
  cout << "  --names - print out the names used for the --ignore command of everything" << endl;
  cout << "  --qnames - print out the names used in a fully qualified, and easily computer parsable format" << endl;
}
