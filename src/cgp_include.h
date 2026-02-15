/**
 * @file cgp_include.h @brief Includes all code generator headers
 *
 * Convenience header file that includes all headers
 * relevant to the code generator. Also used to configure
 * doxygen mainpage/groups -- to be distributed in separate
 * files in a future revision.
 *
 * (c) Thomas Moor 2010, 2016-2019, 2024
 *
 */


#ifndef FAUDES_CGP_INCLUDE_H
#define FAUDES_CGP_INCLUDE_H

// include relevant headers
#include "cgp_eventconfig.h"
#include "cgp_codegenerator.h"
#include "cgp_embeddedc.h"
#include "cgp_atmega.h"
#include "cgp_kinetis.h"
#include "cgp_iec61131st.h"
#include "cgp_gebtools.h"
#include "cgp_plcoxml.h"

#endif


// **************************************************************
// *** Doxygen Page: Introduction *******************************
// **************************************************************

/**
 
@mainpage CompileDES


CompileDES translates a set of automata to executable code that exhibits
a runtime behaviour to match the synchronous composition of the input data.
There are various options to set execution priorities and to associate input and
output signals with specific events; see @ref PExSem. Code generation is organised 
by a hierarchy of classes with the common base CodeGenerator to address the target
languages C and IEC 61131-3 Structured Text; see also @ref CGClasses.

<h2 class="groupheader">Command-Line Tool</h2>

For convenient generation of code, a command-line wrapper is
provided; see compiledes.cpp for details.

@code
~/compiledes> ./bin/compiledes -?

compiledes: usage:

  compiledes [-q][-v] -t <target> -o <outfile> <srcfile>

where
  <desfile>: code generator configuration file

  -q:  less console output
  -qq: absolutely no console output
  -v:  more console output
  -vv: even more console output

  -t <target>: target platform, see also below
  -o <outfile>: base name of output file(s)

compiledes: target platforms: atm ec geb k20 pox st

@endcode

Example configurations are provided in the <tt>.compiledes/examples</tt> directory.
The <tt>*.cgc</tt>-files configure the source automata, event execution
attributes and code-generation options. The overall configuration is then
compiled by <tt>compiledes</tt> to produce the target code. E.g., the ATmega
hello-world adaption <tt>blink_atm.c</tt> is generated from <tt>@ref blink_atm.cgc</tt>
by invoking

@code
~/compiledes> ./bin/compiledes -t atm -o blink_atm.c examples/blink/blink_atm.cfc
@endcode

This particular example is configured for an Arduino Nano board. Compiling
the resulting <tt>blink_atm.c</tt> with an AVR8 C-compiler and downloading
the binary to the board will result in the desired behaviour ... play along.



<h2 class="groupheader">Download &amp; Build</h2>

The CompileDES project is provided for 
<a href="https://fgdes.tf.fau/dedownoad.html">download from the FAUDES-archive</a>
as <tt>faudes_compiledes_x_yyz.tar.gz</tt> with <tt>x_yyz</tt> representing
the corresponding libFAUDES version. Although the package includes pre-compiled executables,
you may need to re-compile to obatin an executable for your particular platform 
(this is specifically the case for Linux based systems).

A GNU-Makefile is provided for to build CompileDES itself,
i.e.,
@code
~/compiledes> make -f Makefile_GMake
@endcode
should build the executable <tt>./build/compiledes</tt> by using the GNU-toolchain; inspect the
makefile for details. If this fails, you will also need to re-compile the provided libFAUDES; see @ref BUILD

For more detailed documentation of the build process and
on how to import the project in the C++ IDEs
Qt Creator and MS Visual Studio, see @ref BUILD.



<h2 class="groupheader">Copyright &amp; License</h2>

The current implementation of the code-generator is a re-design from scratch,
taking into account the experiences gathered in the course of earlier student projects
conducted by Silke Figgen (Step7 AWL), Johannes Tautz (IEC 61131 IL), Nicklas R&uuml;ffler (Step7 SCL),
Frank Schieber (refined controller semantics) and Jakub Lachky (Step7 SCL and AWL),
with respective supervisors Klaus Schmidt, Thomas Wittmann and Thomas Moor.
Copyright of the current implementation is with Thomas Moor. CompileDES is distributed
under terms of the GPL; see @ref LICENSE. If you need access under a different license,
this can be arranged.
<br>
<br>


Copyright (c) 2010, 2016-2019, 2024-2026 Thomas Moor.

*/

// **************************************************************
// *** Doxygen Page: Execution Semantics ************************
// **************************************************************


/** @page PExSem Execution Semantics

By synchronous automata we refer to executing shared events only
if they are enabled in all component automata, and, when executed,
then simultaneously in all component automata. These are the plain
semantics commonly used in supervisory control theory. There is a relevant
gap from this theory point of view to an actual implementation and we need
to precisely formulate how CompileDES generated code fills this gap; relevant
sources are CodePrimitives and AttributeCodeGeneratorEvent.


<h2 class="groupheader">Which entity controls the occurrence of an event?</h2>

The occurrence of an event can be either controlled by the generated code or externally.
Externally controlled events are referred to as "input-events".

With CompileDES, input-events are triggered by edges of digital signals.
Choices are per event and per input line to trigger by positive edge or to trigger
by negative edge or both. In order to detect signals with relevant initial
value, an optional fake edge can be generated on program start; e.g.,
an input-event that is associated with a positive edge will be triggered at program
start if the respective line evaluates true. For convenience purposes,
events can also be triggered by a boolean expression given in terms of the
target language, e.g., a function call with boolean return value.

At any instance of time, the generated code shall accept any
input-event that has been detected. Here "accept" reads that once
detected, transitions are executed and states are updated accordingly.

Events that are not input-events can be executed whenever enabled.
Hence, the semantics so far need to be refined to achieve
a deterministic behaviour.

<h2 class="groupheader">If more than one event is enabled, which one shall occur?</h2>

CompileDES configurations attach an integer-priority attribute to each event.
If multiple events are enabled, the one with the highest priority is considered
to occur.

Merging priorities and the concept of input-events is done by the
following two rules. At any instance of time,

- if an input-event has been detected, the detected input-event with
the highest priority shall be selected
- if no event qualified for the first clause, and if a non-input event
is enabled, then the enabled non-input with the highest priority shall
be selected

Once an event is selected, transitions are executed and states are updated.
It is considered an error if an input-event is detected which is not enabled
by all component automata. Historically, libFAUDES uses negative priorities
for input-events. This is somewhat counter-intuitive. For CompileDES
configurations, all priorities are configured positive (with strategic
conversion for the internal representation).


<h2 class="groupheader">How to handle simultaneous events?</h2>

From the theory point of view, an arbitrary number of events can occur
within an arbitrary short duration of physical time, however, events can not
occur simultaneously; ie. the state-tracjectory is assumed to be a piecewise 
continuous function and logic time is related to physical time at an arbitrarily
high resolution. In contrast, an implementation based on scan-cycles provides
only a limited resolution of physical time.  Successive events within one scan cycle 
are sensed simultaneously. For a faithful recovery of logic time, simultaneously sensed
events must be ordered optimistically. This leads to the following relaxed scheme of event selection.

Initialise the set of pending events to be empty. Then go through
the following steps once per scan cycle

-
figure the set of events enabled by all component automata
-
if input events are detected insert them to the set of pending events
-
if a pending event is enabled, select the one with the highest priority
-
if no event is selected so far, and if an event is enabled, select
the one with the highest priority
-
if an event has been selected, execute transitions and
update the state accordingly
-
if no event has been selected, there is nothing to do until a new input event is detected

It is considered an error if there are pending input-events but neither input-events
nor non-input-events are enabled.
This error should be reported to the host program and the pending event should
be discarded. The current implementation of CompileDES sets a status flag correspondingly.

If there are pending input-events that can not be executed this is not considered
an error as long as some other event can be executed. In this case one can optionally loop
the above steps within one scan cycle until there are no more pending events. However,
this approach is only suitable if the composed discrete-event-system has no non-input-event strongly
connected components. As a safe variation of this approach CompileDes can optionally loop until
there are no more pending input events enabled. In general, it is not recommended
to sacrifice execution semantics for performance. On a decent hardware/compiler, CompileDES
will achieve sufficiently fast cycle times without a nested event-execution loop.



<h2 class="groupheader">Other special cases: timer-events and output-events</h2>

Timers are used to relate logic time with physical time.
CompileDES supports timers with a fixed initial value and the common
start/stop/reset semantics, the latter triggered by individually specified events.
The elapse of a timer in turn triggers the respective timer-event.
Although timers are implemented by generated code, regarding event execution
they are considered as input-events.

Output-events are events with actions attached. Supported actions include
setting or clearing line levels as well as evaluating target language
expressions, e.g. setting flags or calling functions. Regarding execution
semantics as outlined above, output-events are treated the same as other non-input events.

*/


// **************************************************************
// *** Doxygen Page: Code Organisation **************************
// **************************************************************


/** @page CGClasses Implementation Outline


Code generation is organised as a common base class CodeGenerator with a hierarchy of
derived classes that specialise for specific targets.


- CodeGenerator
  handle file-i/o and precompile input data
- CodePrimitives
  specifies execution semantics in terms of
  abstract datatypes and operations
.
- EmbeddedcCodeGenerator
  implement code primitives for generic embedded C targets
- ATmegaCodeGenerator
  specialise embedded C for Atmel ATmega devices
- KinetisCodeGenerator
  specialise embedded C for Freescale Kinetis K20 devices
.
- Iec61131stCodeGenerator
  implement code primitives by generic IEC 61131-3 Structured Text
- GebtoolsCodeGenerator
  specialise IEC 61131-3 code for the GEB Automation toolchain
- PlcoxmlCodeGenerator format IEC 61131-3 code according to the
  PLCopen XML specification


The base class CodeGenerator reads (and writes) the configuration file. The latter
includes source automata as well as execution attributes associated with individual events.
E.g., the configuration file specifies a mapping from input events to respective input line
addresses and polarities of edges that trigger the event. This is fine for configuration
purposes, but it does not match the needs for code generation. Here, we need
to set up edge detection code per line with associated events --- just the other way around.
Other aspects addressed by precompiling configuration data include:
allocating consecutive state indices and event indices for the target code;
have the event indices ordered by execution priority; compile the transition relations
to vectors of integers; prepare look-up tables for bit-masks.

CodePrimitives is derived from CodeGenerator to formulate how the generated code
should behave. For this purpose, it introduces abstract data types like Integer and Word
and operations on them, i.e., integer division, bit-wise conjunction of words, conditional execution
like switch/case or if/then/else. Abstract here reads "without a specific implementation".
The outcome on this stage is a cyclic function stated in terms of data types and operations
thereon to perform edge detection, event selection, event execution and output operation.
The respective templates of code snippets are meant to be independent of an
actual implementation.


To obtain actual code, the classes EmbeddedcCodeGenerator and Iec61131stCodeGenerator
are derived from CodePrimitives, respectively.  The derived classes implement the
individual primitives, e.g., they specify how an integer data is allocated and how and
abstract if/then/else construct in implemented C or IEC 61131-3 ST. This is a straight
forward exercise. At this stage, the generated code can also be organised
according to the target language: do we generate a function, or a program, shall we
distinguish local from global data etc, and what are the generic options to read line-levels
or to otherwise interact with a host program. The latter stage is then further refined
by the derived classes ATmegaCodeGenerator, KinetisCodeGenerator and GebtoolsCodeGenerator. 
The former two take special care of GPIO pin access and other hardware architecture featurs.
The latter provides tailored typedef constructs to please the GEB toolchain.

*/

// **************************************************************
// *** Doxygen Page: Build Process ******************************
// **************************************************************

/**


@page BUILD Build Process


The CompileDES project was originally designed to serve as a template
for libFAUDES applications and to document the steps required for the build 
process. While libFAUDES itself largely consists of platform
independent C++ code, the build process is quite involved and naturally
depends on the choice/availability of compilers, linkers and the
operating system.

We provide a complete run-through for the CompileDES project.
Relevant files are

@verbatim
./src/*.h                            sources of CompileDES, C-headers
./src/*.cpp                          sources of CompileDES, C-code
./src/doxygen*                       doxygen configuration for HTML-documentation

./doc/*                              HTML-documentation produced by doxygen

./examples/*                         test data for CompileDES

./libFAUDES_source/*                 minimally configured libFAUDES 
./libFAUDES_lx/*                     ... pre-compiled for Linux
./libFAUDES_osx/*                    ... pre-compiled for macOs
./libFAUDES_msys/*                   ... pre-ompiled for MSYS
./libFAUDES_win/*                    ... pre-ompiled for Windows

./bin/*                              pre-compiled binaries for Linux/OsX/Windows

./Makefile_GnuMake                   plain Makefile to build with the GNU-toolchain
./CompileDES_QMakeProjekt.pro        project file to build with Qt IDE Creator
./CompileDES_Msvc2015Project.*       project file to build with MS Visual C 2015 IDE
./CompileDES_Msvc2022Project.*       project file to build with MS Visual C 2012 IDE
@endverbatim


For those familiar with either choice of plain GNU-Makefiles, the
Qt Creator IDE or the MS Visual IDE, just go ahead, inspect the respective
project file and build CompileDES. Note that, for simplicity, the pre-compiled
libFAUDES distributed with this package uses static linking in deployment mode.
If you require a different style of linking or if you experience any errors during linkage
please read the detailed instructions below. 


<h2 class="groupheader">Get and Configure libFAUDES</h2>

The CompileDES project statically links to a pre-compiled copy of libFAUDES, provided a number of
flavours for convenience. This section documents how the copies have been obtained from 
the libFAUDS source distribution. Follow these instructions in order to compile a copy 
of libFAUDES tailored to your needs.

To prepare, download and extract libFAUDES sources <tt>libfaudes-x_yz.tgz</tt>
to reside within the project directory

@verbatim
./CompileDES/libFAUDES_source/*
@endverbatim

Instructions to build libFAUDES are provided in the online
documentation; see
<a href="https://fgdes.tf.fau.de/faudes/faudes_build.html">
<i>libFAUDES-&gt;Developer-&gt;Build-System</i></a>.
Specifically, the plain sources need to be configured in order to select
plug-ins and to adjust the run-time behaviour to meet the need of the
application at hand.

The copy of the libFAUDES sources provided with CompileDES
have been generated by the script <tt>copyfaudes.sh</tt>
(functional for Linux/macOS/MSYS environments). This configuration includes specific plug-ins 
as required for the CompileDES project. If you are happy with this configuration,
you can use the sources <tt>../CompileDES/libFAUDES_source</tt> provided with the
CompileDES package as a slim alternative to the complete libFAUDES source distribution.



<h3>Compilation of libFAUDES</h3>

Once libFAUDES sources are configured they can be compiled using the
provided GNU-Makefile. This requires specifically the GNU-Make tool on your system,
regardless whether the application development later on uses the GNU-tool chain.
You may want to inspect the libFAUDES Makefile with a text editor. There are two relevant
switches for compilation, namely FAUDES_PLATFORM and FAUDES_LINKING.
FAUDES_PLATFORM adapts the build process to the toolchain and to the
operating system, prominent choices being <tt>gcc_linux</tt>, <tt>gcc_osx</tt>,
<tt>gcc_msys</tt>  and <tt>cl_win</tt>. FAUDES_LINKING selects static vs dynamic
linking and debug vs release C-libraries. For application development <tt>static</tt>
is the choice to go.

Example command-line to build with Linux/macOS/MSYS using the nominal
system toolchain

@verbatim
~/CompileDES> make -C libFAUDES_source FAUDES_LINKING="static" clean
~/CompileDES> make -C libFAUDES_source FAUDES_LINKING="static"
@endverbatim

or, on MS Windows with MS compiler/linker and MinGW mingw32-make

@verbatim
C:\CompileDES> set PATH=%PATH%;C:\MinGW\bin
C:\CompileDES> call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64"
C:\CompileDES> mingw32-make -C libFAUDES_source FAUDES_LINKING="static" clean
C:\CompileDES> mingw32-make -C libFAUDES_source FAUDES_LINKING="static"
@endverbatim

The CompileDES distribution includes the compiled libFAUDES
in the flavours <tt>gcc_linux</tt>, <tt>gcc_osx</tt>, <tt>gcc_msys</tt> and
<tt>cl_win</tt>. They heve been genereted with the scripts <tt>buildfaudes.sh </tt>,
and <tt>buildfaudes.bat</tt> respectively. If your toolchain is a close enough relative
of either flavour, the pre-compiled libFAUDES is fine to continue with the
CompileDES project. In general, it is recommended to recompile.





<h2 class="groupheader">Organise the Application Build Process</h2>

There are several choices for the organisation of a C++ application
project. For the CompileDES, the provided sources are located in

@verbatim
./CompileDES/src/*.h
./CompileDES/src/*.cpp
@endverbatim

We provide some directions on how to build ComplieDES
with a particular focus on how to link it with libFAUDES.



<h3> Makefile driven build</h3>

A Makefile is a specialised form of a script to organise a build
process and to document dependencies. The make tool, as used for the
compilation of libFAUDES discussed above, will conclude from the
Makefile how compilers and linkers need to be invoked in oder to
build the application.

The CompileDES project comes with a Makefile in the GNU-make
dialect.  It has been verified to be functional for macOS/Linux/MSYS.
In particular, it uses the same nominal system compiler/linker
that was used when compiling libFAUDES by the above instructions. The
provided Makefile links CompileDES statically with libFAUDES
and thereby matches the local copy of libFAUDES prepared for this
purpose.

To build CompileDES, run

@verbatim
./CompileDES> make -f Makefile_GnuMake
@endverbatim

and check for the executable <tt>compiledes</tt> in <tt>./build</tt>. Adaption to
other make dialects (e.g. nmake from MS Visual Studio) is straight
forward. If you feel that managing makefiles is somewhat
cumbersome and spoils the platform independence of your application,
read ahead for a modern variation of textual build process
specifications.



<h3>QMake-projects</h3>

The platform dependent aspects of a Makefile can be equalised by
various automatic Makefile generators using an abstract specification
of how the application should be built. One such tool is qmake,
provided by the Qt project. It is available for all major platforms,
including one-click installers for Linux, macOS and MS Windows.

The CompileDES project comes with the qmake project file
<tt>CompileDES_QMakeProject.pro</tt> which can be processed by qmake to generate a
Makefile that matches the environment specific dialect and that considers
other platform dependent details. Compared with a native Makefile, a
qmake project file can be organised to be much less complex, basically
specifying the project type and the list of sources.  The only libFAUDES
specific lines are the extra include path and the directions to
statically link with libFAUDES.

@verbatim
./CompileDES> qmake CompileDES_QMakeProject.pro
@endverbatim

will generate <tt>Makefile_by_QMake</tt> which in turn builds the
application.

As with the plain Makefile discussed above, qmake will use "the native
C compiler" for macOS and Linux. This should match the compiler in
the system path and therefore is the same one as picked up by the
libFAUDES build system. In consequence,

@verbatim
./CompileDES> make -f Makefile_by_QMake
@endverbatim

should faithfully build CompileDES. Check for the executable
<tt>compiledes</tt> in <tt>./build</tt>.

For MS Windows, Qt is available for different toolchains, including MS
Visual C in variour editions, as well as MinGW gcc in 32bit
and 64bit variants. These compilers are compatible only to a very
limited degree.  As outlined above, it is crucial to use a copy of
libFAUDES that has been compiled with the same toolchain as the one used for
application development. This affects in particular compiler switches used when
building in debug mode. The general advice here is to observe the relevant
tools, compiler switches and linker options that qmake chooses to build the
application.  If the final linking step fails, do not be confused by
experts-only error-reports.  Simply go back and compile libFAUDES in
the same fashion and with the same options. A prominent example here
is the linkage style to the std-c-libraries by the options <tt>/MDd</tt>,
<tt>/MD</tt>, <tt>/MT</tt> or <tt>/MTd</tt> for the MS Visual C compiler --- any mismatch
will break linking with libFAUDES (which by default
uses <tt>/MD</tt> or, with FAUDES_LINKING set on debug, <tt>/MDd</tt>, which we
believe is a sensible choice as of MS Visual C 2015).

Example command line to build CompileDES for MS Windows

@verbatim
C:\CompileDES> qmake CompileDES_QMakeProject.pro
C:\CompileDES> nmake /F Makefile_by_QMake
@endverbatim


<h3>Qt Creator Integrated Development Environment (IDE)</h3>

The Qt project also provides an integrated development environment
(IDE) called Creator. To use the IDE, open the provided qmake
project file and click on the build button. If the build fails due
to "can not find libfaudes.a" uncheck "shadow build" or manually
copy the provided <tt>libfaudes.a</tt> to the relevant build directory.

To run (and/or debug)  <tt>compiledes</tt> from within Creator, use
<i>Creator-&gt;Projects-&gt;Run-Settings</i>
to provide command line arguments and to specify the working directory.
The CompileDES hello-world example is invoked by

@verbatim
./CompileDES> ./build/compiledes -t atm examples/blink/blink_atm.cgc".
@endverbatim

Creator can also be used to generate projects from scratch, avoiding
the textual project description altogether. For the CompileDES project
one may alternatively get started by creating a project "CompileDES" by the new-project wizard

<i>Creator-&gt;New-Project-&gt;non-Qt-project->Plain-C++-project</i>

This creates a folder <tt>CompileDES</tt> and a template source <tt>main.cpp</tt>.

To direct Creator to the location of libFAUDES, use

<i>Creator-&gt;CompileDES-&gt;Context-Menu-&gt;Add-Library</i>

and choose "add external library" to specify <tt>libfaudes.a</tt> (Linux/macOS)
or <tt>faudes.a</tt> (MS Windows) and set <tt>./libFAUDES_xxxx/include</tt>
as the include path; if you prefer all relevant files to be within
the <tt>CompileDES</tt> folder, you should move them there beforehand.

Finally, add the CompileDES sources by

<i>Creator-&gt;CompileDES-&gt;Context-Menu-&gt;Add-Existing-Files</i>

and remove the automatically generated template <tt>main.cpp</tt>; again, you may opt for
copying your sources to be located within the <tt>CompileDES</tt> folder.



<h3>MS Visual Studio C++ IDE</h3>


The steps required here are essentially the same as for Creator,
except that the details are a bit more involved. Let MS Visual
C generate a new "Console C++" project, use "Project Properties" to
direct MS Visual C to libFAUDES and dump in your sources.

In order to be able to build in MS Visual C debug-mode, libFAUDES must
have been compiled with the <tt>/MDd</tt> option; see discussion above.  If
you want to be able to debug into libFAUDES, you also need <tt>/ZI</tt>.  Be
aware, <tt>/ZI</tt> will need a substantial amount of disk space for
debugging information.

The code-generator project comes with the MS Visual project file
<tt>CompileDES_MsVcProject.pro</tt> which has been configured to
- x64 / release configuration
- explicitly direct MS Visual to <tt>faudes.lib</tt> in <tt>libfaudes_win</tt>
- explicitly direct MS Visual to the include path <tt>libfaudes_win/include</tt>
- don't use precompiled headers
- explicitly direct MS Visual C to link with <tt>winmm.lib</tt> and <tt>wsock32.lib</tt>
- use the provided build directory for intermediate and target files



<h2 class="groupheader">Applicable License</h2>

Regarding CompileDES, which is distributed under terms of the GPL, derived work
can only be distributed as GPL open source. If you require access to CompileDES
under different terms, please let us know, this can be arranged.
If you want to distribute your own libFAUDES-based project as closed source 
and/or commercially, configure libFAUDES to include only the LGPL plug-ins and make sure to
use dynamic linking (!). There are some further administrative requirements which
can be met easily --- check the provided license file.

*/

// **************************************************************
// *** Doxygen Imported Pages  **********************************
// **************************************************************

/**
@page LICENSE LICENSE
@verbinclude LICENSE


@example blink_atm.cgc
Usage:
@verbatim
./compiledes> ./build/compiledes -t atm -o blink_atm.c ./examples/blink/blink_atm.cgc
@endverbatim
Configuration:


@example blink_atm.c
Usage:
@verbatim
./compiledes> ./build/compiledes -t atm -o blink_atm.c ./examples/blink/blink_atm.cgc
@endverbatim
Generated code:


@example blink_k20.cgc
Usage:
@verbatim
./compiledes> ./build/compiledes -t k20 -o out.c ./examples/blink/blink_k20.cgc
@endverbatim
Configuration:


@example blink_geb.cgc
Usage:
@verbatim
./compiledes> ./build/compiledes -t geb -o blink_geb.st ./examples/blink/blink_geb.cgc
@endverbatim
Configuration:


@example blink_geb.st
Usage:
@verbatim
./compiledes> ./build/compiledes -t geb -o blink_geb.st ./examples/blink/blink_geb.cgc
@endverbatim
Generated code:


@example blink_pox_c.cgc
Usage:
@verbatim
./compiledes> ./build/compiledes -t pox -o out.xml ./examples/blink/blink_pox_c.cgc
@endverbatim
Configuration:


@example blink_pox_c.xml
Usage:
@verbatim
./compiledes> ./build/compiledes -t pox -o blink_pox_c.xml ./examples/blink/blink_pox_c.cgc
@endverbatim
Configuration:


@example blink_pox_g.cgc
Usage:
@verbatim
./compiledes> ./build/compiledes -t pox -o bling_pox_g.xml ./examples/blink/blink_pox_g.cgc
@endverbatim
Configuration:


@example blink_pox_g.xml
Usage:
@verbatim
./compiledes> ./build/compiledes -t pox -o bling_pox_g.xml ./examples/blink/blink_pox_g.cgc
@endverbatim
Configuration:


*/

