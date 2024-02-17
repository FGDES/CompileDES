/** @file compiledes.cpp @brief Commandline DES compiler

This is a commad-line wrapper for the class CodeGenerator;
see also @ref index for usage details.


@include compiledes.cpp

**/


#include "libfaudes.h"
#include "cgp_codegenerator.h"


using namespace faudes;



// print usage info and exit
void usage_exit(const std::string& message="") {
  if(message!="") {
    std::cout << "compiledes: " << message << std::endl;
    std::cout << "" << std::endl;
    exit(-1);
  }
  std::cout << "compiledes: version " << CodeGenerator::VersionString() << std::endl;
  std::cout << "" << std::endl;
  std::cout << "compiledes: usage: " << std::endl;
  std::cout << "compiledes [-q][-v] -t <target> -o <outfile> <srcfile>"<< std::endl;
  std::cout << "where " << std::endl;
  std::cout << "  <scrfile>: code generator source file, typically \"*.cgc\"" << std::endl;
  std::cout << "" << std::endl;
  std::cout << "  -q:  less console output " << std::endl;
  std::cout << "  -qq: absolutely no console output " << std::endl;
  std::cout << "  -v:  more console output" << std::endl;
  std::cout << "  -vv: even more console output" << std::endl;
  std::cout << "" << std::endl;
  std::cout << "  -t <target>: target platform, see also below " << std::endl;
  std::cout << "  -o <outfile>: (base)name of output file(s), defaults to console output" << std::endl;
  std::cout << "" << std::endl;
  std::cout << "compiledes: target platforms: ";
  std::vector< std::string > registry = CodeGenerator::Registry();
  for(size_t i=0; i<registry.size(); ++i)
      std::cout << registry[i] << " ";
  std::cout << "" << std::endl;
  std::cout << "" << std::endl;
  exit(-1);
}

// parse commandline, read executor and run executor
int main(int argc, char* argv[]) {

  // helper
  std::string mMark="% compiledes: ========================================= ";

  // default behaviour
  int mConsoleOut=0;
  std::string mSrcFile="";
  std::string mOutFile="std::cout";
  std::string mTarget="";

  // primitive commad line parsing
  for(int i=1; i<argc; i++) {
    std::string option(argv[i]);
    // option: quiet
    if((option=="-q") || (option=="--quiet")) {
      mConsoleOut=-1;
      continue;
    }
    // option: quiet
    if(option=="-qq"){
      mConsoleOut=-2;
      continue;
    }
    // option: verbose
    if((option=="-v") || (option=="--verbose")) {
      mConsoleOut=1;
      continue;
    }
    // option: verbose
    if(option=="-vv") {
      mConsoleOut=2;
      continue;
    }
    // option: out file
    if((option=="-o") || (option=="--out")) {
      i++; if(i>=argc) usage_exit();
      mOutFile=argv[i];
      continue;
    }
    // option: target
    if((option=="-t") || (option=="--target")) {
      i++; if(i>=argc) usage_exit();
      mTarget=argv[i];
      continue;
    }
    // option: help
    if((option=="-?") || (option=="--help")) {
      usage_exit();
      continue;
    }
    // option: unknown
    if(option.c_str()[0]=='-') {
      usage_exit("unknown option "+ option);
      continue;
    }
    // filename
    if(mSrcFile!="")
      usage_exit("more than one source file specified" );
    mSrcFile=option;
  }

  // insist in filename
  if(mSrcFile=="")
    usage_exit("you must specify a source file" );

  // prepare code generator object
  CodeGenerator* mpCodeGenerator = CodeGenerator::New(mTarget);

  // test target
  if(!mpCodeGenerator) 
    usage_exit("unknown target platform");

  // set verbosity
  mpCodeGenerator->Verbose(mConsoleOut);

  // read configuration
  try{
    mpCodeGenerator->Read(mSrcFile);
  }
  catch(const Exception& fe) {
    std::cout << std::flush;
    std::cerr << fe.Message() << std::endl;
    std::cerr << "CompileDES: caught libFAUDES exception --- missing/missformed configuration file" << std::endl;
    return 1;
  }

  // set output file
  try{
    mpCodeGenerator->OutputMode(mOutFile);
  }
  catch(const Exception& fe) {
    std::cout << std::flush;
    std::cerr << fe.Message() << std::endl;
    std::cerr << "CompileDES: caught libFAUDES exception --- failed to setup output file/console or output filters" << std::endl;
    return 1;
  }

  // compile
  try{
    mpCodeGenerator->Compile();
  }
  catch(const Exception& fe) {
    std::cout << std::flush;
    std::cerr << fe.Message() << std::endl;
    std::cerr << "CompileDES: caught libFAUDES exception --- failed to compile configuration data to internal representation" << std::endl;
    return 1;
  }

  // generate code
  try{
    mpCodeGenerator->Generate();
  }
  catch(const Exception& fe) {
    std::cout << std::flush;
    std::cerr << fe.Message() << std::endl;
    std::cerr << "CompileDES: caught libFAUDES exception --- failed to generate target code for specified options" << std::endl;
    return 1;
  }

  // done
  return 0;
}
