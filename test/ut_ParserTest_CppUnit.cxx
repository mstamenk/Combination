///
/// CppUnit tests for the parser
///
///  This is copmlex enough I feel like we need a parser around to make sure nothing gets
/// screwed up as development evolves over time.
///
///  This is black-box testing of the parser.
///

#include "Combination/Parser.h"
#include "Combination/CalibrationDataModelStreams.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Exception.h>
#include <iostream>
#include <stdexcept>
#include <sstream>

using namespace std;
using namespace BTagCombination;

// VS2012 (which ROOT is built against) doesn't have NAN).
#ifdef _MSC_VER
#if (_MSC_VER <= 1700)
unsigned long nan[2] = { 0xffffffff, 0x7fffffff };
double gNAN = *(double*)nan;
#define NAN gNAN
#endif
#endif

//
// Test harness/fixture for the parser
//

class ParserTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( ParserTest );

  //CPPUNIT_TEST( testSourceComments );

  CPPUNIT_TEST(testParseEmptyAnalysis);
  CPPUNIT_TEST_EXCEPTION( testParseSyntaxBasicErrorThrows, std::runtime_error );
  CPPUNIT_TEST( testParseEmptyAnalysisString );
  CPPUNIT_TEST( testParseSimpleAnalysis );
  CPPUNIT_TEST( testParseSimpleAnalysisNotEqual );
  CPPUNIT_TEST( testParseSimpleAnalysisFunnySpaces );
  CPPUNIT_TEST( testParseSimpleAnalysisWithOneBinOneArg );
  //CPPUNIT_TEST( testParseSimpleAnalysisWithOneBinTwoArg );
  CPPUNIT_TEST( testParseTwoAnalyses );
  //CPPUNIT_TEST( testParseSimpleAnalysisBadFlavor );
  CPPUNIT_TEST(testParseSimpleAnalysisWithSys);
  CPPUNIT_TEST(testParseSimpleAnalysisWithSysQuoted);
  CPPUNIT_TEST(testParseSimpleAnalysisWithSysColon);
  CPPUNIT_TEST(testParseSimpleAnalysisWithUSys);
  CPPUNIT_TEST(testParseSimpleAnalysisWithNSys);
  CPPUNIT_TEST(testParseSimpleAnalysisWithSpaceSys);
  CPPUNIT_TEST(testParseSimpleAnalysisWithSlashSys);
  CPPUNIT_TEST( testParseSimpleAnalysisWithFunnyBinning );

  CPPUNIT_TEST( testParseSimpleAnalysisWithOneExtendedBin );
  CPPUNIT_TEST(testParseRoundTripExtendedBin);

  CPPUNIT_TEST_EXCEPTION(writeNanCV, std::runtime_error);
  CPPUNIT_TEST_EXCEPTION(writeNanCVS, std::runtime_error);
  CPPUNIT_TEST_EXCEPTION(writeNanSYS, std::runtime_error);
  CPPUNIT_TEST_EXCEPTION(readNanCV, std::runtime_error);
  CPPUNIT_TEST_EXCEPTION(readNanCVS, std::runtime_error);
  CPPUNIT_TEST_EXCEPTION(readNanSYS, std::runtime_error);

  CPPUNIT_TEST(testParseRoundTripZeros);
  CPPUNIT_TEST(testParseRoundTripZerosNotSys);
  CPPUNIT_TEST(testParseRoundTrip);
  CPPUNIT_TEST(testParseRoundTrip2);
  CPPUNIT_TEST(testParseRoundTrip3);
  CPPUNIT_TEST(testParseRoundTrip4);
  CPPUNIT_TEST(testParseRoundTrip5);
  CPPUNIT_TEST(testParseRoundTrip6);
  CPPUNIT_TEST(testParseRoundTrip7);
  CPPUNIT_TEST(testParseRoundTrip8);
  CPPUNIT_TEST(testParseRoundTrip9);
  CPPUNIT_TEST(testParseRoundTrip10);

  CPPUNIT_TEST(testParseCorrelation);
  CPPUNIT_TEST(testParseCorrelation2);
  CPPUNIT_TEST_EXCEPTION(testParseCorrelation3, std::runtime_error);
  CPPUNIT_TEST_EXCEPTION(testParseCorrelation4, std::runtime_error);
  CPPUNIT_TEST(testParseCorrelation5);

  //CPPUNIT_TEST(testParseBinMetadata1);
  CPPUNIT_TEST(testParseBinMetadata2);
  CPPUNIT_TEST(testParseBinMetadata3);
  CPPUNIT_TEST(testParseBinMetadata4);
  CPPUNIT_TEST(testParseBinMetadata5);

  CPPUNIT_TEST(testParseMetadata1);
  CPPUNIT_TEST(testParseMetadata2);
  CPPUNIT_TEST(testParseMetadata3);
  CPPUNIT_TEST(testParseMetadata4);
  CPPUNIT_TEST(testParseMetadata5);
  CPPUNIT_TEST(testParseMetadata6);

  CPPUNIT_TEST(testParseCopy);
  CPPUNIT_TEST(testParseCopy2);
  CPPUNIT_TEST(testParseCopyRoundtrip);

  CPPUNIT_TEST(testParseDefault);
  CPPUNIT_TEST(testParseDefaultWildCard);
  CPPUNIT_TEST(testParseDefaultRoundtrip);

  CPPUNIT_TEST(testSysErrorNotEqual);
  CPPUNIT_TEST(testSysErrorEqual);

  CPPUNIT_TEST(testParseSplitAnalysis);
  CPPUNIT_TEST_EXCEPTION(testParseSplitAnalysisWithOverlap, std::runtime_error);

  CPPUNIT_TEST_SUITE_END();

  void testSourceComments()
  {
    cout << "Test testSourceComments" << endl;
    // Test that comments in the code can happen anywhere! :-)
    CPPUNIT_ASSERT_MESSAGE("Not written yet", false);
  }

  void testSysErrorNotEqual()
  {
    SystematicError s1, s2;
    s1.name = "hi";
    s2.name = "hi";
    s1.value = 0.1;
    s2.value = 0.15;
    s1.uncorrelated = true;
    s2.uncorrelated = true;

    CPPUNIT_ASSERT (!(s1 == s2));
  }

  void testSysErrorEqual()
  {
    SystematicError s1, s2;
    s1.name = "hi";
    s2.name = "hi";
    s1.value = 0.15;
    s2.value = 0.15;
    s1.uncorrelated = true;
    s2.uncorrelated = true;

    CPPUNIT_ASSERT (s1 == s2);
  }

  void testParseEmptyAnalysisString()
  {
    cout << "Test testParseEmptyAnalysisString" << endl;
    CalibrationInfo result (Parse(""));
    CPPUNIT_ASSERT(result.Analyses.size() == 0);
  }

  // Nothing good can come of this.
  void testParseSyntaxBasicErrorThrows()
  {
    cout << "Test testparseSyntaxBasicErrorThrows" << endl;
	CalibrationInfo result(Parse("AAANNNnalysis(ptrel, bottom, SV050){}"));
  }

  void testParseEmptyAnalysis()
  {
	  cout << "Test testParseEmptyAnalysis" << endl;
	  CalibrationInfo result(Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){}"));
	  CPPUNIT_ASSERT_EQUAL((size_t)0, result.Analyses.size());
  }

  void testParseSimpleAnalysis()
  {
    cout << "Test testParseSimpleAnalysis" << endl;
	CalibrationInfo result(Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01)}}"));
    stringstream str;
    str << "Result:" << endl << result << endl;
    CPPUNIT_ASSERT_MESSAGE(str.str(), result.Analyses.size() == 1);
    CPPUNIT_ASSERT(result.Analyses[0].name == "ptrel");
    CPPUNIT_ASSERT_MESSAGE(result.Analyses[0].operatingPoint, result.Analyses[0].operatingPoint == "0.50");
    //CPPUNIT_ASSERT(result[0].flavor == FBottom);
    CPPUNIT_ASSERT(result.Analyses[0].tagger == "SV0");
    CPPUNIT_ASSERT(result.Analyses[0].jetAlgorithm == "MyJets");
    CPPUNIT_ASSERT(result.Analyses[0].flavor == "bottom");
    CPPUNIT_ASSERT(result.Analyses[0].bins.size() == 1);
  }

  void testParseSimpleAnalysisNotEqual()
  {
    cout << "Test testParseSimpleAnalysisNotEqual" << endl;
	CalibrationInfo result(Parse("Analysis(ptrel, bottom, SV0, !=0, MyJets){bin(20<pt<30){central_value(0.5,0.01)}}"));
    stringstream str;
    str << "Result:" << endl << result << endl;
    CPPUNIT_ASSERT_MESSAGE(str.str(), result.Analyses.size() == 1);
    CPPUNIT_ASSERT(result.Analyses[0].name == "ptrel");
    CPPUNIT_ASSERT_MESSAGE(result.Analyses[0].operatingPoint, result.Analyses[0].operatingPoint == "!=0");
    CPPUNIT_ASSERT(result.Analyses[0].tagger == "SV0");
    CPPUNIT_ASSERT(result.Analyses[0].jetAlgorithm == "MyJets");
    CPPUNIT_ASSERT(result.Analyses[0].flavor == "bottom");
    CPPUNIT_ASSERT(result.Analyses[0].bins.size() == 1);
  }

  void testParseSimpleAnalysisFunnySpaces()
  {
    cout << "Test testParseSimpleAnalysisFunnySpaces" << endl;
	CalibrationInfo result(Parse("Analysis(ptrel ,bottom ,SV0, 0.50 ,MyJets){bin(20<pt<30){central_value(0.5,0.01)}}"));
    stringstream str;
    str << "Result:" << endl << result << endl;
    CPPUNIT_ASSERT_MESSAGE(str.str(), result.Analyses.size() == 1);
    cout << "Found name is '" << result.Analyses[0].name << "'!" << endl;
    CPPUNIT_ASSERT(result.Analyses[0].name == "ptrel");
    CPPUNIT_ASSERT_MESSAGE(result.Analyses[0].operatingPoint, result.Analyses[0].operatingPoint == "0.50");
    //CPPUNIT_ASSERT(result[0].flavor == FBottom);
    CPPUNIT_ASSERT(result.Analyses[0].tagger == "SV0");
    CPPUNIT_ASSERT(result.Analyses[0].jetAlgorithm == "MyJets");
    CPPUNIT_ASSERT(result.Analyses[0].flavor == "bottom");
    CPPUNIT_ASSERT(result.Analyses[0].bins.size() == 1);
  }


  void testParseTwoAnalyses()
  {
    cout << "Test testParseTwoAnalyses" << endl;
	CalibrationInfo result(Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01)}} Analysis(system8, bottom, SV0, 0.50, MyJets) {bin(20<pt<30){central_value(0.5,0.01)}}"));
    stringstream str;
    str << "Result size is " << result.Analyses.size() << "!" << endl;
    CPPUNIT_ASSERT_MESSAGE(str.str(), result.Analyses.size() == 2);
    CPPUNIT_ASSERT(result.Analyses[0].name == "ptrel");
    CPPUNIT_ASSERT(result.Analyses[1].name == "system8");
  }

  void testParseSimpleAnalysisBadFlavor()
  {
    cout << "Test testParseSimpleAnalysisBadFlavor" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, botttom, SV050){}"));
    CPPUNIT_ASSERT(false);
  }

  void testParseSimpleAnalysisWithOneBinOneArg()
  {
    cout << "Test testParseSimpleAnalysisWithOneBinOneArg" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01)}}"));
    
    CPPUNIT_ASSERT(result.Analyses.size() == 1);
    CalibrationAnalysis ana = result.Analyses[0];
    stringstream str;
    str << "  Found " << ana.bins.size() << endl;
    CPPUNIT_ASSERT_MESSAGE(str.str(), ana.bins.size() == 1);
    CalibrationBin bin0 = ana.bins[0];
    cout << "  Found bins to do spec in: " << bin0.binSpec.size() << endl;
    stringstream str1;
    str1 << "The number of bin boundaries is " << bin0.binSpec.size() << endl;
    CPPUNIT_ASSERT_MESSAGE(str1.str(), bin0.binSpec.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Binis marked extended", !bin0.isExtended);
    CalibrationBinBoundary bb = bin0.binSpec[0];
    CPPUNIT_ASSERT(bb.lowvalue == 20);
    CPPUNIT_ASSERT(bb.variable == "pt");
    CPPUNIT_ASSERT(bb.highvalue == 30);
  }

  void testParseSplitAnalysis()
  {
	  // Automaticlaly combine analyses that are split in two in the input files.
	  cout << "Test testParseSplitAnalysis" << endl;
	  CalibrationInfo result(Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01)}} Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(30<pt<40){central_value(0.5,0.01)}}"));

	  CPPUNIT_ASSERT(result.Analyses.size() == 1);
	  CalibrationAnalysis ana = result.Analyses[0];
	  stringstream str;
	  str << "  Found " << ana.bins.size() << endl;
	  CPPUNIT_ASSERT_MESSAGE(str.str(), ana.bins.size() == 2);
	  CPPUNIT_ASSERT_DOUBLES_EQUAL(20, ana.bins[0].binSpec[0].lowvalue, 0.1);
	  CPPUNIT_ASSERT_DOUBLES_EQUAL(30, ana.bins[1].binSpec[0].lowvalue, 0.1);
  }

  void testParseSplitAnalysisWithOverlap()
  {
	  // When we combine a split analysis if two have the same bin that is BAD.
	  cout << "Test testParseSplitAnalysis" << endl;
	  CalibrationInfo result(Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01)}} Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<40){central_value(0.5,0.01)}}"));
  }

  void testParseSimpleAnalysisWithFunnyBinning()
  {
    CalibrationInfo result (Parse("Analysis(negative tags,light, SV0, 0.50, MyJets){bin( 20<pt< 30, 0.0<abseta<1.2){central_value(0.5,0.01) meta_data(N jets tagger,    589, 24.3)}}"));
    
    CPPUNIT_ASSERT(result.Analyses.size() == 1);
    CalibrationAnalysis ana = result.Analyses[0];
    stringstream str;
    str << "  Found " << ana.bins.size() << endl;
    CPPUNIT_ASSERT_MESSAGE(str.str(), ana.bins.size() == 1);
    CalibrationBin bin0 = ana.bins[0];
    cout << "  Found bins to do spec in: " << bin0.binSpec.size() << endl;
    stringstream str1;
    str1 << "The number of bin boundaries is " << bin0.binSpec.size() << endl;
    CPPUNIT_ASSERT_MESSAGE(str1.str(), bin0.binSpec.size() == 2);
  }

  void testParseSimpleAnalysisWithOneBinTwoArg()
  {
    cout << "Test testParseSimpleAnalysisWithOneBinTwoArg" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV050){bin(20<pt<30, 1.1<eta<5.5)}"));
    
    CPPUNIT_ASSERT(result.Analyses.size() == 1);
    CalibrationAnalysis ana = result.Analyses[0];
    stringstream str;
    str << "  Found " << ana.bins.size() << endl;
    CPPUNIT_ASSERT_MESSAGE(str.str(), ana.bins.size() == 1);
    CalibrationBin bin0 = ana.bins[0];
    cout << "  Found bins to do spec in: " << bin0.binSpec.size() << endl;
    stringstream str1;
    str1 << "The number of bin boundaries is " << bin0.binSpec.size() << endl;
    CPPUNIT_ASSERT_MESSAGE(str1.str(), bin0.binSpec.size() == 2);
    CalibrationBinBoundary bb = bin0.binSpec[0];
    CPPUNIT_ASSERT(bb.lowvalue == 20);
    CPPUNIT_ASSERT(bb.variable == "pt");
    CPPUNIT_ASSERT(bb.highvalue == 30);

    bb = bin0.binSpec[1];
    CPPUNIT_ASSERT(bb.lowvalue == 1.1);
    CPPUNIT_ASSERT(bb.variable == "eta");
    CPPUNIT_ASSERT(bb.highvalue == 5.5);
  }

  void testParseSimpleAnalysisWithSys()
  {
    cout << "Test testParseSimpleAnalysisWithSys" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(dude, 0.1%)}}"));
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.Analyses.size());

    CalibrationAnalysis ana = result.Analyses[0];
    CalibrationBin bin0 = ana.bins[0];

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.systematicErrors.size());
    SystematicError e(bin0.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("dude"), e.name);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.05*0.1/100.0, e.value, 0.001);
  }

  void testParseSimpleAnalysisWithSysQuoted()
  {
    cout << "Test testParseSimpleAnalysisWithSys" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(\"dude \", 0.1%)}}"));
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.Analyses.size());

    CalibrationAnalysis ana = result.Analyses[0];
    CalibrationBin bin0 = ana.bins[0];

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.systematicErrors.size());
    SystematicError e(bin0.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("dude "), e.name);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.05*0.1/100.0, e.value, 0.001);
  }

  void testParseSimpleAnalysisWithSysColon()
  {
    cout << "Test testParseSimpleAnalysisWithSys" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(dude: fo.rk*, 0.1%)}}"));
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.Analyses.size());

    CalibrationAnalysis ana = result.Analyses[0];
    CalibrationBin bin0 = ana.bins[0];

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.systematicErrors.size());
    SystematicError e(bin0.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("dude: fo.rk*"), e.name);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.05*0.1/100.0, e.value, 0.001);
  }

  void testParseSimpleAnalysisWithNSys()
  {
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(dude, -0.1%)}}"));
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.Analyses.size());

    CalibrationAnalysis ana = result.Analyses[0];
    CalibrationBin bin0 = ana.bins[0];

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.systematicErrors.size());
    SystematicError e(bin0.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("dude"), e.name);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (-0.05*0.1/100.0, e.value, 0.001);
  }

  // Test the extended analysis bin specification
  void testParseSimpleAnalysisWithOneExtendedBin()
  {
    cout << "Test testParseSimpleAnalysisWithOneExtendedBin" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){exbin(20<pt<30){central_value(0.5,0.01)}}"));
    
    CPPUNIT_ASSERT(result.Analyses.size() == 1);
    CalibrationAnalysis ana = result.Analyses[0];
    stringstream str;
    str << "  Found " << ana.bins.size() << endl;
    CPPUNIT_ASSERT_MESSAGE(str.str(), ana.bins.size() == 1);
    CalibrationBin bin0 = ana.bins[0];
    cout << "  Found bins to do spec in: " << bin0.binSpec.size() << endl;
    stringstream str1;
    str1 << "The number of bin boundaries is " << bin0.binSpec.size() << endl;
    CPPUNIT_ASSERT_MESSAGE(str1.str(), bin0.binSpec.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Bin is not extended", bin0.isExtended);
    CalibrationBinBoundary bb = bin0.binSpec[0];
    CPPUNIT_ASSERT(bb.lowvalue == 20);
    CPPUNIT_ASSERT(bb.variable == "pt");
    CPPUNIT_ASSERT(bb.highvalue == 30);
  }

  void testParseRoundTripExtendedBin()
  {
    cout << "Test testParseRoundTripExtendedBin" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){exbin(20<pt<30){central_value(0.5,0.01) sys(dude, 0.1%)}}"));
    
    ostringstream buffer;
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.Analyses.size());
    cout << result << endl;
    buffer << result << endl;

    CalibrationInfo result2 (Parse(buffer.str()));

    CPPUNIT_ASSERT_EQUAL((size_t)1, result2.Analyses.size());
    CalibrationAnalysis ana = result2.Analyses[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, ana.bins.size());
    CalibrationBin bin0 = ana.bins[0];
    CPPUNIT_ASSERT(bin0.isExtended);
  }

  void testParseSimpleAnalysisWithSpaceSys()
  {
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(dude , -0.1%)}}"));
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.Analyses.size());

    CalibrationAnalysis ana = result.Analyses[0];
    CalibrationBin bin0 = ana.bins[0];

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.systematicErrors.size());
    SystematicError e(bin0.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("dude"), e.name);
    CPPUNIT_ASSERT_EQUAL(false, e.uncorrelated);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (-0.05*0.1/100.0, e.value, 0.001);
  }

  void testParseSimpleAnalysisWithSlashSys()
  {
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(ISR/FSR, -0.1%)}}"));
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.Analyses.size());

    CalibrationAnalysis ana = result.Analyses[0];
    CalibrationBin bin0 = ana.bins[0];

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.systematicErrors.size());
    SystematicError e(bin0.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("ISR/FSR"), e.name);
    CPPUNIT_ASSERT_EQUAL(false, e.uncorrelated);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (-0.05*0.1/100.0, e.value, 0.001);
  }

  void testParseSimpleAnalysisWithUSys()
  {
    cout << "Test testParseSimpleAnalysisWithUSys" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) usys(dude, 0.1%)}}"));
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.Analyses.size());

    CalibrationAnalysis ana = result.Analyses[0];
    CalibrationBin bin0 = ana.bins[0];

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.systematicErrors.size());
    SystematicError e(bin0.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("dude"), e.name);
    CPPUNIT_ASSERT_EQUAL(true, e.uncorrelated);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.05*0.1/100.0, e.value, 0.001);
  }

  void writeNanCV()
  {
    cout << "Test writeNanCV" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(dude, 0.1%)}}"));
    result.Analyses[0].bins[0].centralValue = NAN;
    cout << result << endl;
  }
  void writeNanCVS()
  {
    cout << "Test writeNanCVS" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(dude, 0.1%)}}"));
    result.Analyses[0].bins[0].centralValueStatisticalError = NAN;
    cout << result << endl;
  }
  void writeNanSYS()
  {
    cout << "Test writeNanSYS" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(dude, 0.1%)}}"));
    result.Analyses[0].bins[0].systematicErrors[0].value = NAN;
    cout << result << endl;
  }

  void readNanCV()
  {
    cout << "Test readNanCV" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(nan,0.01) sys(dude, 0.1%)}}"));
  }
  void readNanCVS()
  {
    cout << "Test readNanCVS" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,nan) sys(dude, 0.1%)}}"));
  }
  void readNanSYS()
  {
    cout << "Test readNanSYS" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(dude, nan%)}}"));
  }

  // Everything is zeros... this happens it turns out... as a signal when nothing is known.
  void testParseRoundTripZerosNotSys()
  {
    cout << "Test testParseRoundTripZeros" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.0,0.1) sys(dude, 0.2)}}"));
    
    ostringstream buffer;
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.Analyses.size());
    cout << result << endl;
    buffer << result << endl;

    CalibrationInfo result2 (Parse(buffer.str()));

    CPPUNIT_ASSERT_EQUAL((size_t)1, result2.Analyses.size());
    CalibrationAnalysis ana = result2.Analyses[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, ana.bins.size());
    CalibrationBin bin0 = ana.bins[0];
    CPPUNIT_ASSERT(!bin0.isExtended);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, bin0.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, bin0.centralValueStatisticalError, 0.001);

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.binSpec.size());
    CalibrationBinBoundary bb = bin0.binSpec[0];
    CPPUNIT_ASSERT(bb.lowvalue == 20);
    CPPUNIT_ASSERT(bb.variable == "pt");
    CPPUNIT_ASSERT(bb.highvalue == 30);

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.systematicErrors.size());
    SystematicError e(bin0.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("dude"), e.name);
    CPPUNIT_ASSERT_EQUAL(false, e.uncorrelated);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2, e.value, 0.001);
  }

  // Everything is zeros... this happens it turns out... as a signal when nothing is known.
  void testParseRoundTripZeros()
  {
    cout << "Test testParseRoundTripZeros" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.0,0.0) sys(dude, 0.0)}}"));
    
    ostringstream buffer;
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.Analyses.size());
    cout << result << endl;
    buffer << result << endl;

    CalibrationInfo result2 (Parse(buffer.str()));

    CPPUNIT_ASSERT_EQUAL((size_t)1, result2.Analyses.size());
    CalibrationAnalysis ana = result2.Analyses[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, ana.bins.size());
    CalibrationBin bin0 = ana.bins[0];
    CPPUNIT_ASSERT(!bin0.isExtended);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, bin0.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, bin0.centralValueStatisticalError, 0.001);

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.binSpec.size());
    CalibrationBinBoundary bb = bin0.binSpec[0];
    CPPUNIT_ASSERT(bb.lowvalue == 20);
    CPPUNIT_ASSERT(bb.variable == "pt");
    CPPUNIT_ASSERT(bb.highvalue == 30);

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.systematicErrors.size());
    SystematicError e(bin0.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("dude"), e.name);
    CPPUNIT_ASSERT_EQUAL(false, e.uncorrelated);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0, e.value, 0.001);
  }

  void testParseRoundTrip()
  {
    cout << "Test testParseRoundTrip" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(dude, 0.1%)}}"));
    
    ostringstream buffer;
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.Analyses.size());
    cout << result << endl;
    buffer << result << endl;

    CalibrationInfo result2 (Parse(buffer.str()));

    CPPUNIT_ASSERT_EQUAL((size_t)1, result2.Analyses.size());
    CalibrationAnalysis ana = result2.Analyses[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, ana.bins.size());
    CalibrationBin bin0 = ana.bins[0];
    CPPUNIT_ASSERT(!bin0.isExtended);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, bin0.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.01, bin0.centralValueStatisticalError, 0.001);

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.binSpec.size());
    CalibrationBinBoundary bb = bin0.binSpec[0];
    CPPUNIT_ASSERT(bb.lowvalue == 20);
    CPPUNIT_ASSERT(bb.variable == "pt");
    CPPUNIT_ASSERT(bb.highvalue == 30);

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.systematicErrors.size());
    SystematicError e(bin0.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("dude"), e.name);
    CPPUNIT_ASSERT_EQUAL(false, e.uncorrelated);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.05*0.1/100.0, e.value, 0.001);
  }

  void testParseRoundTrip2()
  {
    cout << "Test testParseRoundTrip2" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(dude, 0.1%)}} Analysis(ptrel2, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(dude, 0.1%)}}"));
    
    CPPUNIT_ASSERT_EQUAL((size_t)2, result.Analyses.size());

    ostringstream buffer;
    cout << result << endl;
    buffer << result << endl;

    CalibrationInfo result2 (Parse(buffer.str()));

    CPPUNIT_ASSERT_EQUAL((size_t)2, result2.Analyses.size());
  }

  void testParseRoundTrip3()
  {
    cout << "Test testParseRoundTrip" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) usys(dude, 0.1%)}}"));
    
    ostringstream buffer;
    cout << result.Analyses[0] << endl;
    buffer << result.Analyses[0] << endl;

    CalibrationInfo result2 (Parse(buffer.str()));

    CPPUNIT_ASSERT_EQUAL((size_t)1, result2.Analyses.size());
    CalibrationAnalysis ana = result2.Analyses[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, ana.bins.size());
    CalibrationBin bin0 = ana.bins[0];
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, bin0.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.01, bin0.centralValueStatisticalError, 0.001);

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.binSpec.size());
    CalibrationBinBoundary bb = bin0.binSpec[0];
    CPPUNIT_ASSERT(bb.lowvalue == 20);
    CPPUNIT_ASSERT(bb.variable == "pt");
    CPPUNIT_ASSERT(bb.highvalue == 30);

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.systematicErrors.size());
    SystematicError e(bin0.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("dude"), e.name);
    CPPUNIT_ASSERT_EQUAL(true, e.uncorrelated);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.05*0.1/100.0, e.value, 0.001);
  }

  void testParseCorrelation()
  {
    cout << "Test testParseCorrelation" << endl;
    CalibrationInfo result (Parse("Correlation (ptrel, s8, bottom, MV1, 0.9, AntiKt) { bin (0 < pt < 5) {statistical(0.5)}}"));
    CPPUNIT_ASSERT_EQUAL(size_t(1), result.Correlations.size());

    AnalysisCorrelation c (result.Correlations[0]);
    CPPUNIT_ASSERT_EQUAL(string("ptrel"), c.analysis1Name);
    CPPUNIT_ASSERT_EQUAL(string("s8"), c.analysis2Name);
    CPPUNIT_ASSERT_EQUAL(string("bottom"), c.flavor);
    CPPUNIT_ASSERT_EQUAL(string("MV1"), c.tagger);
    CPPUNIT_ASSERT_EQUAL(string("0.9"), c.operatingPoint);
    CPPUNIT_ASSERT_EQUAL(string("AntiKt"), c.jetAlgorithm);

    CPPUNIT_ASSERT_EQUAL(size_t(1), c.bins.size());
    BinCorrelation b(c.bins[0]);
    CPPUNIT_ASSERT_EQUAL(size_t(1), b.binSpec.size());
    CalibrationBinBoundary bb(b.binSpec[0]);

    CPPUNIT_ASSERT_EQUAL(string("pt"), bb.variable);
    CPPUNIT_ASSERT_EQUAL(0.0, bb.lowvalue);
    CPPUNIT_ASSERT_EQUAL(5.0, bb.highvalue);

    CPPUNIT_ASSERT_EQUAL(true, b.hasStatCorrelation);
    CPPUNIT_ASSERT_EQUAL(0.5, b.statCorrelation);
  }

  void testParseCorrelation2()
  {
    cout << "Test testParseCorrelation" << endl;
    CalibrationInfo result (Parse("Correlation (ptrel, s8, bottom, MV1, 0.9, AntiKt) { bin (0 < pt < 5) {}}"));
    CPPUNIT_ASSERT_EQUAL(size_t(1), result.Correlations.size());

    AnalysisCorrelation c (result.Correlations[0]);
    CPPUNIT_ASSERT_EQUAL(string("ptrel"), c.analysis1Name);
    CPPUNIT_ASSERT_EQUAL(string("s8"), c.analysis2Name);
    CPPUNIT_ASSERT_EQUAL(string("bottom"), c.flavor);
    CPPUNIT_ASSERT_EQUAL(string("MV1"), c.tagger);
    CPPUNIT_ASSERT_EQUAL(string("0.9"), c.operatingPoint);
    CPPUNIT_ASSERT_EQUAL(string("AntiKt"), c.jetAlgorithm);

    CPPUNIT_ASSERT_EQUAL(size_t(1), c.bins.size());
    BinCorrelation b(c.bins[0]);
    CPPUNIT_ASSERT_EQUAL(size_t(1), b.binSpec.size());
    CalibrationBinBoundary bb(b.binSpec[0]);

    CPPUNIT_ASSERT_EQUAL(string("pt"), bb.variable);
    CPPUNIT_ASSERT_EQUAL(0.0, bb.lowvalue);
    CPPUNIT_ASSERT_EQUAL(5.0, bb.highvalue);

    CPPUNIT_ASSERT_EQUAL(false, b.hasStatCorrelation);
  }


  void testParseCorrelation3()
  {
    cout << "Test testParseCorrelation" << endl;
    // This should cause a crash since the correlation is greater than 1.
    CalibrationInfo result (Parse("Correlation (ptrel, s8, bottom, MV1, 0.9, AntiKt) { bin (0 < pt < 5) {statistical(1.1)}}"));
  }

  void testParseCorrelation4()
  {
    cout << "Test testParseCorrelation" << endl;
    // This should cause a crash since the correlation is greater than 1.
    CalibrationInfo result (Parse("Correlation (ptrel, s8, bottom, MV1, 0.9, AntiKt) { bin (0 < pt < 5) {statistical(-1.1)}}"));
  }

  void testParseCorrelation5()
  {
    cout << "Test testParseCorrelation" << endl;
    CalibrationInfo result (Parse("Correlation (ptrel, s8, bottom, MV1, 0.9, AntiKt) { bin (0 < pt < 5) {statistical(-0.5)}}"));
    CPPUNIT_ASSERT_EQUAL(size_t(1), result.Correlations.size());

    AnalysisCorrelation c (result.Correlations[0]);
    CPPUNIT_ASSERT_EQUAL(string("ptrel"), c.analysis1Name);
    CPPUNIT_ASSERT_EQUAL(string("s8"), c.analysis2Name);
    CPPUNIT_ASSERT_EQUAL(string("bottom"), c.flavor);
    CPPUNIT_ASSERT_EQUAL(string("MV1"), c.tagger);
    CPPUNIT_ASSERT_EQUAL(string("0.9"), c.operatingPoint);
    CPPUNIT_ASSERT_EQUAL(string("AntiKt"), c.jetAlgorithm);

    CPPUNIT_ASSERT_EQUAL(size_t(1), c.bins.size());
    BinCorrelation b(c.bins[0]);
    CPPUNIT_ASSERT_EQUAL(size_t(1), b.binSpec.size());
    CalibrationBinBoundary bb(b.binSpec[0]);

    CPPUNIT_ASSERT_EQUAL(string("pt"), bb.variable);
    CPPUNIT_ASSERT_EQUAL(0.0, bb.lowvalue);
    CPPUNIT_ASSERT_EQUAL(5.0, bb.highvalue);

    CPPUNIT_ASSERT_EQUAL(true, b.hasStatCorrelation);
    CPPUNIT_ASSERT_EQUAL(-0.5, b.statCorrelation);
  }

  void testParseBinMetadata1()
  {
    cout << "Test testParseBinMetadata" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01)meta_data (ISR FSR, -0.1)}}"));
    
    CPPUNIT_ASSERT(result.Analyses.size() == 1);
    CalibrationAnalysis ana = result.Analyses[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, ana.bins.size());
    CalibrationBin bin = ana.bins[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, bin.metadata.size());
    pair<string, pair<double,double> > v = *(bin.metadata.begin());

    CPPUNIT_ASSERT_EQUAL(string("ISR FSR"), v.first);
    CPPUNIT_ASSERT_EQUAL(-0.1, v.second.first);
    CPPUNIT_ASSERT_EQUAL(0.0, v.second.second);
  }

  void testParseBinMetadata2()
  {
    cout << "Test testParseBinMetadata2" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01)meta_data (ISR FSR, -0.1, 0.05)}}"));
    
    CPPUNIT_ASSERT(result.Analyses.size() == 1);
    CalibrationAnalysis ana = result.Analyses[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, ana.bins.size());
    CalibrationBin bin = ana.bins[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, bin.metadata.size());
    pair<string, pair<double,double> > v = *(bin.metadata.begin());

    CPPUNIT_ASSERT_EQUAL(string("ISR FSR"), v.first);
    CPPUNIT_ASSERT_EQUAL(-0.1, v.second.first);
    CPPUNIT_ASSERT_EQUAL(0.05, v.second.second);
  }

  void testParseBinMetadata3()
  {
    cout << "Test testParseBinMetadata3" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) meta_data (ISR/FSR, -0.1,1.0)}}"));
    
    CPPUNIT_ASSERT(result.Analyses.size() == 1);
    CalibrationAnalysis ana = result.Analyses[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, ana.bins.size());
    CalibrationBin bin = ana.bins[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, bin.metadata.size());
    pair<string, pair<double,double> > v = *(bin.metadata.begin());

    CPPUNIT_ASSERT_EQUAL(string("ISR/FSR"), v.first);
    CPPUNIT_ASSERT_EQUAL(-0.1, v.second.first);


    CPPUNIT_ASSERT_EQUAL(1.0, v.second.second);
  }

  void testParseBinMetadata4()
  {
    cout << "Test testParseBinMetadata4" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01)meta_data (ISR(FSR), -0.1,1.0)}}"));
    
    CPPUNIT_ASSERT(result.Analyses.size() == 1);
    CalibrationAnalysis ana = result.Analyses[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, ana.bins.size());
    CalibrationBin bin = ana.bins[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, bin.metadata.size());
    pair<string, pair<double,double> > v = *(bin.metadata.begin());

    CPPUNIT_ASSERT_EQUAL(string("ISR(FSR)"), v.first);
    CPPUNIT_ASSERT_EQUAL(-0.1, v.second.first);
    CPPUNIT_ASSERT_EQUAL(1.0, v.second.second);
  }

  // Test out a single item in teh meta data.
  void testParseBinMetadata5()
  {
    cout << "Test testParseBinMetadata5" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01)meta_data (ISR(FSR), -0.1)}}"));
    
    CPPUNIT_ASSERT(result.Analyses.size() == 1);
    CalibrationAnalysis ana = result.Analyses[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, ana.bins.size());
    CalibrationBin bin = ana.bins[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, bin.metadata.size());
    pair<string, pair<double,double> > v = *(bin.metadata.begin());

    CPPUNIT_ASSERT_EQUAL(string("ISR(FSR)"), v.first);
    CPPUNIT_ASSERT_EQUAL(-0.1, v.second.first);
    CPPUNIT_ASSERT_EQUAL(0.0, v.second.second);
  }

  void testParseRoundTrip8()
  {
    cout << "Test testParseRoundTri8" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) meta_data(dude, 0.1, 0.2)}}"));
    
    ostringstream buffer;
    cout << result << endl;
    buffer << result << endl;

    CalibrationInfo result2 (Parse(buffer.str()));

    CalibrationAnalysis ana = result2.Analyses[0];
    CalibrationBin bin0 = ana.bins[0];

    CPPUNIT_ASSERT_EQUAL((size_t) 1, bin0.metadata.size());
    pair<string, pair<double, double> > rmeta = *(bin0.metadata.begin());
    CPPUNIT_ASSERT_EQUAL(string("dude"), rmeta.first);
    CPPUNIT_ASSERT_EQUAL(0.1, rmeta.second.first);
    CPPUNIT_ASSERT_EQUAL(0.2, rmeta.second.second);
  }

  void testParseMetadata1()
  {
    cout << "Test testParseMetadata" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){meta_data (ISR FSR, -0.1) bin(20<pt<30){central_value(0.5,0.01)}}"));
    
    CPPUNIT_ASSERT(result.Analyses.size() == 1);
    CalibrationAnalysis ana = result.Analyses[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, ana.metadata.size());
    pair<string, vector<double> > v = *(ana.metadata.begin());
    CPPUNIT_ASSERT_EQUAL((size_t)1, v.second.size());
    CPPUNIT_ASSERT_EQUAL(string("ISR FSR"), v.first);
    CPPUNIT_ASSERT_EQUAL(-0.1, v.second[0]);
  }

  void testParseMetadata2()
  {
    cout << "Test testParseMetadata" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){meta_data(ISR FSR, -0.1)bin(20<pt<30){central_value(0.5,0.01)}meta_data(\"yo,dude\", -0.1)}"));
    
    CPPUNIT_ASSERT(result.Analyses.size() == 1);
    CalibrationAnalysis ana = result.Analyses[0];
    CPPUNIT_ASSERT_EQUAL((size_t)2, ana.metadata.size());

    CPPUNIT_ASSERT(ana.metadata.find("yo,dude") != ana.metadata.end());
    CPPUNIT_ASSERT(ana.metadata.find("ISR FSR") != ana.metadata.end());
  }

  void testParseMetadata3()
  {
    cout << "Test testParseMetadata" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){meta_data (ISR FSR, -0.1, 5.1) bin(20<pt<30){central_value(0.5,0.01)}}"));
    
    CPPUNIT_ASSERT(result.Analyses.size() == 1);
    CalibrationAnalysis ana = result.Analyses[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, ana.metadata.size());
    pair<string, vector<double> > v = *(ana.metadata.begin());
    CPPUNIT_ASSERT_EQUAL((size_t)2, v.second.size());
    CPPUNIT_ASSERT_EQUAL(string("ISR FSR"), v.first);
    CPPUNIT_ASSERT_EQUAL(-0.1, v.second[0]);
    CPPUNIT_ASSERT_EQUAL(5.1, v.second[1]);
  }

  void testParseMetadata4()
  {
    cout << "Test testParseMetadata4" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){ meta_data_s(ISR FSR, no way) bin(20<pt<30){central_value(0.5,0.01)}}"));
    
    CPPUNIT_ASSERT(result.Analyses.size() == 1);
    CalibrationAnalysis ana = result.Analyses[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, ana.metadata_s.size());
    pair<string, string> v = *(ana.metadata_s.begin());
    CPPUNIT_ASSERT_EQUAL(string("ISR FSR"), v.first);
    CPPUNIT_ASSERT_EQUAL(string("no way"), v.second);
  }

  void testParseMetadata5()
  {
	  cout << "Test testParseMetadata5" << endl;
	  CalibrationInfo result(Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){ meta_data_s(Linage, D*[tbarpdf+dork=>DStar]) bin(20<pt<30){central_value(0.5,0.01)}}"));

	  CPPUNIT_ASSERT(result.Analyses.size() == 1);
	  CalibrationAnalysis ana = result.Analyses[0];
	  CPPUNIT_ASSERT_EQUAL((size_t)1, ana.metadata_s.size());
	  pair<string, string> v = *(ana.metadata_s.begin());
	  CPPUNIT_ASSERT_EQUAL(string("Linage"), v.first);
	  CPPUNIT_ASSERT_EQUAL(string("D*[tbarpdf+dork=>DStar]"), v.second);
  }

  void testParseMetadata6()
  {
	  cout << "Test testParseMetadata5" << endl;
	  CalibrationInfo result(Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){ meta_data_s(Linage, D*[DStar;tbarpdf+dork]) bin(20<pt<30){central_value(0.5,0.01)}}"));

	  CPPUNIT_ASSERT(result.Analyses.size() == 1);
	  CalibrationAnalysis ana = result.Analyses[0];
	  CPPUNIT_ASSERT_EQUAL((size_t)1, ana.metadata_s.size());
	  pair<string, string> v = *(ana.metadata_s.begin());
	  CPPUNIT_ASSERT_EQUAL(string("Linage"), v.first);
	  CPPUNIT_ASSERT_EQUAL(string("D*[DStar;tbarpdf+dork]"), v.second);
  }

  void testParseRoundTrip6()
  {
    cout << "Test testParseRoundTrip6" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01)}meta_data(junk, 0.2)}"));
    
    ostringstream buffer;
    //cout << result << endl;
    buffer << result << endl;

    CalibrationInfo result2 (Parse(buffer.str()));
 
    CalibrationAnalysis ana(result2.Analyses[0]);
    pair<string, vector<double> > v = *(ana.metadata.begin());
    CPPUNIT_ASSERT_EQUAL(string("junk"), v.first);
    CPPUNIT_ASSERT_EQUAL((size_t)1, v.second.size());
    CPPUNIT_ASSERT_EQUAL(0.2, v.second[0]);
  }

  void testParseRoundTrip7()
  {
    cout << "Test testParseRoundTrip6" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01)}meta_data(\"junk,f\", 0.2)}"));
    
    ostringstream buffer;
    //cout << result << endl;
    buffer << result << endl;

    CalibrationInfo result2 (Parse(buffer.str()));
 
    CalibrationAnalysis ana(result2.Analyses[0]);
    pair<string, vector<double> > v = *(ana.metadata.begin());
    CPPUNIT_ASSERT_EQUAL(string("junk,f"), v.first);
    CPPUNIT_ASSERT_EQUAL((size_t)1, v.second.size());
    CPPUNIT_ASSERT_EQUAL(0.2, v.second[0]);
  }

  void testParseRoundTrip9()
  {
    cout << "Test testParseRoundTrip9" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01)}meta_data(junk, 0.2, 3.3)}"));
    
    ostringstream buffer;
    //cout << result << endl;
    buffer << result << endl;

    CalibrationInfo result2 (Parse(buffer.str()));
 
    CalibrationAnalysis ana(result2.Analyses[0]);
    pair<string, vector<double> > v = *(ana.metadata.begin());
    CPPUNIT_ASSERT_EQUAL(string("junk"), v.first);
    CPPUNIT_ASSERT_EQUAL((size_t)2, v.second.size());
    CPPUNIT_ASSERT_EQUAL(0.2, v.second[0]);
    CPPUNIT_ASSERT_EQUAL(3.3, v.second[1]);
  }

  void testParseRoundTrip10()
  {
    cout << "Test testParseRoundTrip10" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){ meta_data_s (ISR FSR, no way) bin(20<pt<30){central_value(0.5,0.01)}}"));
    
    ostringstream buffer;
    buffer << result << endl;

    CalibrationInfo result2(Parse(buffer.str()));

    CalibrationAnalysis ana(result2.Analyses[0]);

    CPPUNIT_ASSERT_EQUAL((size_t)1, ana.metadata_s.size());
    pair<string, string> v = *(ana.metadata_s.begin());
    CPPUNIT_ASSERT_EQUAL(string("ISR FSR"), v.first);
    CPPUNIT_ASSERT_EQUAL(string("no way"), v.second);
  }

  void testParseRoundTrip4()
  {
    cout << "Test testParseRoundTrip4" << endl;
    CalibrationInfo result (Parse("Correlation (ptrel, s8, bottom, MV1, 0.9, AntiKt) {bin (0 < pt < 5) { statistical(0.5)}}"));
    
    ostringstream buffer;
    cout << result << endl;
    buffer << result << endl;

    CalibrationInfo result2 (Parse(buffer.str()));

    CPPUNIT_ASSERT_EQUAL((size_t)1, result2.Correlations.size());

    AnalysisCorrelation c (result2.Correlations[0]);
    CPPUNIT_ASSERT_EQUAL(string("ptrel"), c.analysis1Name);
    CPPUNIT_ASSERT_EQUAL(string("s8"), c.analysis2Name);
    CPPUNIT_ASSERT_EQUAL(string("bottom"), c.flavor);
    CPPUNIT_ASSERT_EQUAL(string("MV1"), c.tagger);
    CPPUNIT_ASSERT_EQUAL(string("0.9"), c.operatingPoint);
    CPPUNIT_ASSERT_EQUAL(string("AntiKt"), c.jetAlgorithm);
    CPPUNIT_ASSERT_EQUAL(size_t(1), c.bins.size());

    BinCorrelation b(c.bins[0]);
    CPPUNIT_ASSERT_EQUAL(size_t(1), b.binSpec.size());
    CalibrationBinBoundary bb(b.binSpec[0]);

    CPPUNIT_ASSERT_EQUAL(string("pt"), bb.variable);
    CPPUNIT_ASSERT_EQUAL(0.0, bb.lowvalue);
    CPPUNIT_ASSERT_EQUAL(5.0, bb.highvalue);

    CPPUNIT_ASSERT_EQUAL(true, b.hasStatCorrelation);
    CPPUNIT_ASSERT_EQUAL(0.5, b.statCorrelation);
  }

  void testParseRoundTrip5()
  {
    cout << "Test testParseRoundTrip5" << endl;
    CalibrationInfo result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(\"dude \", 0.1%)}}"));
    
    ostringstream buffer;
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.Analyses.size());
    cout << result << endl;
    buffer << result << endl;

    CalibrationInfo result2 (Parse(buffer.str()));

    CPPUNIT_ASSERT_EQUAL((size_t)1, result2.Analyses.size());
    CalibrationAnalysis ana = result2.Analyses[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, ana.bins.size());
    CalibrationBin bin0 = ana.bins[0];
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, bin0.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.01, bin0.centralValueStatisticalError, 0.001);

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.binSpec.size());
    CalibrationBinBoundary bb = bin0.binSpec[0];
    CPPUNIT_ASSERT(bb.lowvalue == 20);
    CPPUNIT_ASSERT(bb.variable == "pt");
    CPPUNIT_ASSERT(bb.highvalue == 30);

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.systematicErrors.size());
    SystematicError e(bin0.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("dude "), e.name);
    CPPUNIT_ASSERT_EQUAL(false, e.uncorrelated);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.05*0.1/100.0, e.value, 0.001);
  }

  void testParseDefault()
  {
    cout << "Test testParseDefault" << endl;
    CalibrationInfo result (Parse("Default (ptrel, bottom, MV1, 0.9, AntiKt)"));
    
    CPPUNIT_ASSERT_EQUAL((size_t)0, result.Analyses.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, result.Correlations.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.Defaults.size());

    DefaultAnalysis d (result.Defaults[0]);
    CPPUNIT_ASSERT_EQUAL(string("ptrel"), d.name);
    CPPUNIT_ASSERT_EQUAL(string("bottom"), d.flavor);
    CPPUNIT_ASSERT_EQUAL(string("MV1"), d.tagger);
    CPPUNIT_ASSERT_EQUAL(string("0.9"), d.operatingPoint);
    CPPUNIT_ASSERT_EQUAL(string("AntiKt"), d.jetAlgorithm);
  }

  void testParseDefaultWildCard()
  {
    cout << "Test testParseDefault" << endl;
    CalibrationInfo result (Parse("Default (ptrel, bottom, *, *,*)"));
    
    CPPUNIT_ASSERT_EQUAL((size_t)0, result.Analyses.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, result.Correlations.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.Defaults.size());

    DefaultAnalysis d (result.Defaults[0]);
    CPPUNIT_ASSERT_EQUAL(string("ptrel"), d.name);
    CPPUNIT_ASSERT_EQUAL(string("bottom"), d.flavor);
    CPPUNIT_ASSERT_EQUAL(string("*"), d.tagger);
    CPPUNIT_ASSERT_EQUAL(string("*"), d.operatingPoint);
    CPPUNIT_ASSERT_EQUAL(string("*"), d.jetAlgorithm);
  }

  void testParseDefaultRoundtrip()
  {
    cout << "Test testParseRoundtrip" << endl;
    CalibrationInfo result (Parse("Default (ptrel, bottom, MV1, 0.9, AntiKt)"));
    
    ostringstream buffer;
    cout << result << endl;
    buffer << result << endl;

    CalibrationInfo result2 (Parse(buffer.str()));

    CPPUNIT_ASSERT_EQUAL((size_t)0, result2.Analyses.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, result2.Correlations.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, result2.Defaults.size());

    DefaultAnalysis d (result2.Defaults[0]);
    CPPUNIT_ASSERT_EQUAL(string("ptrel"), d.name);
    CPPUNIT_ASSERT_EQUAL(string("bottom"), d.flavor);
    CPPUNIT_ASSERT_EQUAL(string("MV1"), d.tagger);
    CPPUNIT_ASSERT_EQUAL(string("0.9"), d.operatingPoint);
    CPPUNIT_ASSERT_EQUAL(string("AntiKt"), d.jetAlgorithm);
  }

  void testParseCopy()
  {
    cout << "Test testParseCopy" << endl;
    CalibrationInfo result (Parse("Copy(ptrel, bottom, MV1, 0.9, AntiKt) {Analysis(ptrel, bottom, MV2, 0.9, AntiKt)}"));
    
    CPPUNIT_ASSERT_EQUAL((size_t)0, result.Analyses.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, result.Correlations.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, result.Defaults.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.Aliases.size());

    AliasAnalysis d (result.Aliases[0]);
    CPPUNIT_ASSERT_EQUAL(string("ptrel"), d.name);
    CPPUNIT_ASSERT_EQUAL(string("bottom"), d.flavor);
    CPPUNIT_ASSERT_EQUAL(string("MV1"), d.tagger);
    CPPUNIT_ASSERT_EQUAL(string("0.9"), d.operatingPoint);
    CPPUNIT_ASSERT_EQUAL(string("AntiKt"), d.jetAlgorithm);

    CPPUNIT_ASSERT_EQUAL((size_t)1, d.CopyTargets.size());
    AliasAnalysisCopyTo c (d.CopyTargets[0]);
    CPPUNIT_ASSERT_EQUAL(string("ptrel"), c.name);
    CPPUNIT_ASSERT_EQUAL(string("bottom"), c.flavor);
    CPPUNIT_ASSERT_EQUAL(string("MV2"), c.tagger);
    CPPUNIT_ASSERT_EQUAL(string("0.9"), c.operatingPoint);
    CPPUNIT_ASSERT_EQUAL(string("AntiKt"), c.jetAlgorithm);
  }

  void testParseCopy2()
  {
    cout << "Test testParseCopy2" << endl;
    CalibrationInfo result (Parse("Copy(ptrel, bottom, MV1, 0.9, AntiKt) {Analysis(ptrel, bottom, MV2, 0.9, AntiKt) Analysis(ptrel, bottom, MV3, 0.9, AntiKt)}"));
    
    CPPUNIT_ASSERT_EQUAL((size_t)0, result.Analyses.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, result.Correlations.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, result.Defaults.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.Aliases.size());

    AliasAnalysis d (result.Aliases[0]);
    CPPUNIT_ASSERT_EQUAL(string("ptrel"), d.name);
    CPPUNIT_ASSERT_EQUAL(string("bottom"), d.flavor);
    CPPUNIT_ASSERT_EQUAL(string("MV1"), d.tagger);
    CPPUNIT_ASSERT_EQUAL(string("0.9"), d.operatingPoint);
    CPPUNIT_ASSERT_EQUAL(string("AntiKt"), d.jetAlgorithm);

    CPPUNIT_ASSERT_EQUAL((size_t)2, d.CopyTargets.size());
    AliasAnalysisCopyTo c (d.CopyTargets[0]);
    CPPUNIT_ASSERT_EQUAL(string("ptrel"), c.name);
    CPPUNIT_ASSERT_EQUAL(string("bottom"), c.flavor);
    CPPUNIT_ASSERT_EQUAL(string("MV2"), c.tagger);
    CPPUNIT_ASSERT_EQUAL(string("0.9"), c.operatingPoint);
    CPPUNIT_ASSERT_EQUAL(string("AntiKt"), c.jetAlgorithm);

    c = d.CopyTargets[1];
    CPPUNIT_ASSERT_EQUAL(string("ptrel"), c.name);
    CPPUNIT_ASSERT_EQUAL(string("bottom"), c.flavor);
    CPPUNIT_ASSERT_EQUAL(string("MV3"), c.tagger);
    CPPUNIT_ASSERT_EQUAL(string("0.9"), c.operatingPoint);
    CPPUNIT_ASSERT_EQUAL(string("AntiKt"), c.jetAlgorithm);
  }

  void testParseCopyRoundtrip()
  {
    cout << "Test testParseCopy" << endl;
    CalibrationInfo init (Parse("Copy(ptrel, bottom, MV1, 0.9, AntiKt) {Analysis(ptrel, bottom, MV2, 0.9, AntiKt)}"));
    
    ostringstream buffer;
    cout << init << endl;
    buffer << init << endl;

    CalibrationInfo result (Parse(buffer.str()));

    CPPUNIT_ASSERT_EQUAL((size_t)0, result.Analyses.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, result.Correlations.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, result.Defaults.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.Aliases.size());

    AliasAnalysis d (result.Aliases[0]);
    CPPUNIT_ASSERT_EQUAL(string("ptrel"), d.name);
    CPPUNIT_ASSERT_EQUAL(string("bottom"), d.flavor);
    CPPUNIT_ASSERT_EQUAL(string("MV1"), d.tagger);
    CPPUNIT_ASSERT_EQUAL(string("0.9"), d.operatingPoint);
    CPPUNIT_ASSERT_EQUAL(string("AntiKt"), d.jetAlgorithm);

    CPPUNIT_ASSERT_EQUAL((size_t)1, d.CopyTargets.size());
    AliasAnalysisCopyTo c (d.CopyTargets[0]);
    CPPUNIT_ASSERT_EQUAL(string("ptrel"), c.name);
    CPPUNIT_ASSERT_EQUAL(string("bottom"), c.flavor);
    CPPUNIT_ASSERT_EQUAL(string("MV2"), c.tagger);
    CPPUNIT_ASSERT_EQUAL(string("0.9"), c.operatingPoint);
    CPPUNIT_ASSERT_EQUAL(string("AntiKt"), c.jetAlgorithm);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ParserTest);

// The common atlas test driver
#include <TestPolicy/CppUnit_testdriver.cxx>
