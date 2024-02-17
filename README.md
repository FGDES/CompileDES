# CompileDES --- Compile Discrete-Event Systems to Executable Code 

Once a number of automata have been synthesised to supervise a physical
plant, the task remains to implement their behaviour on a likewise physical device.
For the latter task, CompileDES in that it generates executable code from
libFAUDES automata. At this stage, target languages _C_ and _IEC_ _61131_ _ST_
are supported, along with a exhaustive range of configuration options.

- [doxygen generated documentation](https://fgdes.tf.fau.de/compiledes)


The code in this repository is strictly source only.
Precomiled binaries/executable are available for Linux/MacOSx/Windows.
see https://fgdes.tf.fau.de/compiledes. If you want to build your own,
it is recommended that you get a precompiled package as a basis and
then update the sources via this repository or to whatever your needs are.

- [get precompiled package](https://fgdes.tf.fau.de/download.html)
- [build process](https://fgdes.tf.fau.de/compiledes/build.html)


# Copyright/License/Authors

The current implementation of the code generator is a re-design from scratch, taking into account the experiences gathered in the course of earlier student projects conducted by Silke Figgen (Step7 AWL), Johannes Tautz (IEC 61131 IL), Nicklas Rüffler (Step7 SCL), Frank Schieber (refined controller semantics) and Jakub Lachky (Step7 SCL and AWL), with respective supervisors Klaus Schmidt, Thomas Wittmann and Thomas Moor.

Copyright of the current implementation is with Thomas Moor. CompileDES is distributed under terms of the GPL; see LICENSE. If you need access under a different license, this can be arranged.

Copyright (c) 2010, 2016-2019, 2024 Thomas Moor.


