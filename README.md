# EE_272_HW4
EE272 Homework 4
HLS Design
Due: Feb 3, 2022, 11:59 pm
Released: Jan 27, 2022
1
High Level Synthesis with Catapult
In this homework, you will generate an RTL design of your CNN accelerator using high level
synthesis (HLS), and perform behavioural verification. Your goal is to generate an RTL de-
sign similar to the one you painstakingly wrote in homework 3, only this time by writing it
in C++ and guiding an HLS tool, using compiler directives, to produce the design that you
want. It is usually relatively easy to produce a functionally correct design with HLS, because
HLS takes care of generating all the error-prone control logic in your chip (for example, your
complex top-level convolution controller). It is, however, much harder to produce an imple-
mentation that has high performance, because you have to give the scheduler enough hints
about what to unroll and pipeline. In homework 5, you will focus on optimizing your design
to have high performance. Since you hand wrote the RTL in homework 2 and 3, you know
the architecture you are looking for and its performance (in terms of number of cycles) very
well. Always keep your homework 3 architecture in mind while using HLS, and try and see
if HLS is producing the same (or better!) design. This will require you to understand deeply
how HLS translates C++ constructs into hardware. Use the two HLS lectures for guidance.
HLS tools have a steep learning curve, so this homework will need significant
time. Please start early.
The high level synthesis tool we will use is Catapult from Mentor Graphics. It takes HLS
C++ code and specified pragmas and directives, and generates RTL Verilog code. Writing
HLS code is very similar to writing C++ code, but in a more restricted way. The code
structures and styles have to satisfy certain conditions in order to generate reasonable and
efficient designs. Pragmas and directives are used for specifying resource type (whether a
memory is implemented using registers or SRAM), loop pipelining and unrolling, etc.
2
Getting Started
The starter code can be found at this code repo (https://code.stanford.edu/ee272/
dnn-accelerator-hls-unoptimized.git). To get started, log in to the FarmShare machine
and run the Singularity image as before, and then clone the homework repository:
1Figure 1: Catapult GUI.
$ git clone
https://code.stanford.edu/ee272/dnn-accelerator-hls-unoptimized.git
Then make sure you are at the top level of this code repo before running any commands:
$ cd dnn-accelerator-hls-unoptimized
Next, set up the environment variables by sourcing the setup file:
$ source setenv.csh
3
Catapult Documentation
Catapult has a very steep learning curve. You will find the following resources helpful
while using Catapult to complete this homework.
• HLS Bluebook: Use this to get a general understanding of how to write HLS code.
It is at the following location on caddy machines: /cad/mentor/2019.11/Catapult_
Synthesis_10.4b-841621/Mgc_home/shared/pdfdocs/hls_bluebook.pdf
• Catapult User Reference: For command related questions see /cad/mentor/2019.11/
Catapult_Synthesis_10.4b-841621/Mgc_home/shared/pdfdocs/catapult_useref.
pdf on caddy machines.
2• Catapult User Manual: This is the GUI version of the User Reference above. It can
be accessed by clicking Help and Documentation in Catapult GUI as shown in
Figure 1.
• Edge Detection HLS Walkthrough:
https://www.youtube.com/channel/UCJMnUQKLcNNQ_KrD97_sU6w/playlists. This
series provides a step-by-step walk-through of what is needed to take a C++ floating-
point algorithm all the way to optimized RTL using Catapult synthesis on an edge
detection design. The sources for the design are up on GitHub at https://github.
com/hlslibs/hls_tutorials/tree/master/WalkThroughs/EdgeDetect/src, so you
can download the examples and follow along.
4
Code Organization
The code organization in dnn-accelerator-hls-unoptimized folder is shown below, along
with what each file contains.
3src
boost // Library, don't worry about this folder
Conv.cpp
InputDoubleBuffer.h
WeightDoubleBuffer.h
Fifo.h
ProcessingElement.h
SystolicArray.h
SystolicArrayCore.h //
//
//
//
// Top level HLS module
Input double buffer
Weight double buffer
Templated FIFO for skewing
Combinational MAC
//
//
//
//
// Contains skewing FIFOs, combinational MAC
and registers array, accumulation buffer
Deserializes the layer parameters
Serializes the outputs from the
accumulation buffer
Deserializer.h
Serializer.h
conv.h // Contains top level instantiation
// parameters and data structures
ConvTb.cpp
conv_gold.cpp
conv_gold_tiled.cpp
conv_tb_params.h //
//
//
//
//
scripts
//
common.tcl
//
Conv.tcl
//
InputDoubleBuffer.tcl
WeightDoubleBuffer.tcl
ProcessingElement.tcl
SystolicArrayCore.tcl
generic_libraries.tcl //
set_libraries.tcl
//
run_c_test.tcl
//
run_rtl_test.tcl
//
//
run_rtl_test_no_gui.tcl
layers
Makefile
autograder.py
setenv.csh
Testbench for Conv.cpp
Gold model you wrote earlier
Tiled gold model you wrote eariler
Layer parameters used for configuring the
testbench
Catapult scripts
Sets HLS options common to all modules
Module specific Catapult scripts
Defines the technology
Uses the above library
Runs verification in C++
Runs verification on the generated RTL
and opens the Catapult waveform viewer
45
Testbench
For this homework, we give you the testbench. This is defined in src/ConvTb.cpp. It would
be helpful for you to read the testbench to understand in what order the data is streamed
in and out of your accelerator. To run the testbench on your C++ code, do:
$ make c_test
To generate the RTL and then run the testbench on the generated RTL, do:
$ make rtl_test
You should read the Makefile to see what commands to run for running any of the sub
steps individually.
6
Implementation
In the starter code, we have created the different blocks that make up the design. These are
in the various files in the src folder. The Code Organization section summarizes what each
file contains.
• The hierarchy of the design is:
Conv (Conv.cpp)
– InputDoubleBuffer (InputDoubleBuffer.h)
∗ InputBankWriter
∗ InputBankReader
– WeightDoubleBuffer (WeightDoubleBuffer.h)
∗ WeightBankWriter
∗ WeightBankReader
– SystolicArrayWrapper (SystolicArray.h)
∗ SystolicArrayLooper
∗ SystolicArrayCore (SystolicArrayCore.h)
· ProcessingElement (ProcessingElement.h)
• You should start by reading conv.h that defines the accelerator parameters and datatypes.
The parameter names roughly correspond to what they were called in homework 2 and
3. For debugging, use smaller parameter values like a 4 × 4 array.
Note that when you get to the synthesis step, if you changed the default parameters, the
scripts will need to be slightly modified, as they are written for the default parameters
defined in conv.h. For example, if you changed the ARRAY DIMENSION in the C++
code, you will have to change the ARRAY DIMENSION value in scripts/common.tcl as
well.
5Figure 2: CNN accelerator.
• Then read Conv.cpp to see how the datatypes are used, and how the major sub-modules
are instantiated and connected. You do not have to add any code in this file, but read
it to make sure you understand what is going on.
• Your first task is to implement the input and weight double buffers. We have
provided you all the interfaces, and the files indicate the places where you have to fill
in code. Before implementing double buffers go through the HLS lecture — the code
needs to be written in a certain style for Catapult to infer a double buffer correctly.
• Next implement the processing element and the systolic array. The processing
element is combinational unlike the one in homework 3, meaning the input, weight and
output registers are instantiated outside the processing element in the systolic array
core. The systolic array core reads from the weight double buffer and writes to the
weight registers. (Note: In this homework the weights are not skewed. You need to
load the weights to weight registers properly so that they are ready for PEs to use.) It
reads inputs from the input double buffer and feeds into the skew FIFOs, which feed
the PE array. (Note: In your previous homework you had skew registers, but
this homework you will use skew FIFOs, which are already created for you
6(see Figure 2). You will need to hook these up. It is important to remember
that in this assignment when we talk about FIFOs, we are referring to skew
FIFOs.) The outputs from the PE array are skewed and written to the accumulation
buffer as before, and they are read out and fed back into the PE array from the top to
accumulate across FX, FY and IC dimensions.
• The systolic array source code is contained in SystolicArray.h and SystolicArrayCo
re.h. The former defines a looper which generates the loop indices that are fed to
the systolic array core. It determines the number of times it runs the run func-
tion of SystolicArrayCore. Notice that paramsChannel is an ac channel, so in
SystolicArrayLooper function you will have to write to it as many times as you
write to LoopIndices, despite the fact that the values are always the same.
• In this homework the two banks of the output buffer are separated (see Figure 2). You
will implement the bank used for accumulation in SystolicArrayCore. You will not
need to implement the bank that captures the output that is being sent out of the
accelerator. This has been done for you. Make sure to write the outputs to the output
double buffer in SystolicArrayCore.
• Once you have an implementation, run the C test first to check that the C code you
have written is functional. Debugging this is like debugging normal C code. If you
would like to use gdb, the binary file is located at:
./test/Conv.v1/scverify/orig cxx osci/scverify top
• Another way to debug the design is to edit the ConvTb.cpp to give the array predictable
inputs and weights and print intermediate steps of the design to verify operation. This
will be a very useful strategy for debugging the C code part of the design process. You
can change rand init to 0 and test with pre-defined inputs and weights.
• Once you have a functional design, you will now need to modify the build scripts
to make sure that the tool synthesizes your design into what you are expecting.
Read through the scripts in scripts/ and understand what needs to be done in
each file. Since you need to fill in commands, it might be helpful to open the GUI
and see what commands should go in the script. Hint: For the double buffers you
will need to set the WORD WIDTH for the reader temporary variable and din and
the writer temporary variable and dout. You will also need to set WORD WIDTH
and STAGE REPLICATION for mem and mem:cns, respectively. Below is a screen-
shot showing you how to navigate to mem. You will need to follow similar steps for
SystolicArrayCore.tcl. Specifically, you should set the correct INTERLEAVE or
BLOCK SIZE for the accumulation buffer (refer to User Reference), make sure the
registers map to registers and not memories, set a register threshold, and ignore mem-
ory dependencies that the tool assumes are occurring. Try building the different blocks
and when the script stops at the unimplemented section, open the GUI:
$ make InputDoubleBuffer
$ make gui
7Figure 3: Catapult GUI.
• You will also need to look at the schedule for each block. Click on the Schedule
tab to open up the schedule for a block. Select between the different blocks in your
design in the dropdown as shown in Figure 4. Since optimization is not done in this
homework, your throughput will likely be bad. In the next assignment your goal will
be to optimize the performance of your design. (Get a head start by understanding
the Catapult scheduler early!)
• Follow the same procedure until you have completed the build script for every single
block:
$ make WeightDoubleBuffer
$ make gui
$ make ProcessingElement
$ make gui
$ make SystolicArrayCore
$ make gui
When running SystolicArrayCore scheduling you may run into “could not schedule even
with unlimited resources” error. This is because of the false memory dependencies issue
that we covered in the lecture. To investigate which memory operations are causing
the issue, in the gui click Schedule to start scheduling. It will fail and display the
error messages in red. You can then figure out the operations that you need to set
8ignore memory precedences for. Hint: You can use wildcards to cover a group of
operations in one command.
• Once you have finished all the build scripts, verify that the entire design can be syn-
thesized without issues:
$ make clean
$ make
You should now have generated RTL! You can find the generated RTL in the file
dnn-accelerator-hls-unoptimized/build/Conv.v1/concat rtl.v.
• Finally, run an RTL simulation. You can get to the top level module by searching for
module Conv, and then look inside it hierarchically.
When NCSim launches, several signals are added to the waveform window by default.
Input and output ports for all the blocks are added by default, as well as several signals
that indicate when the blocks in your design are active. Click through the hierarchy
tab on the left as shown in Figure 5 to find the ProcessingElement module, and add the
ccs ccore en signal of a PE to the waveform window. This signal is high whenever the
PE is active. After you run the simulation, you can look at this signal to assess whether
your design achieves high PE utilization. There might be other signals that would be
helpful to look at. Please reference the Catapult User Reference to understand the
names for the generated RTL. Note: Run a 4 × 4 design with a small test layer for
faster debug iteration times.
Press the run button on the top to start the simulation as shown in Figure 5. The
red error cnt sig, which appears in the GUI after the top level ports, will tell you if
there’s a mismatch between the C++ code and the generated Verilog. Verify that all
the outputs match. The comparison result is also printed when you run the autograder.
The RTL test compares the RTL outputs with C++ outputs. Therefore, if the RTL
test shows no errors but the C++ test does, this does not mean that your design works.
• If you make changes to your code and recompile, and see errors that you can
not decipher, sometimes running make clean will help, before recompiling.
$ make clean
7
Autograder
The autograder can help run the commands required to test your design. In this assignment,
you must test both the functionality of your design in C and in Verilog.
To test in C, use the following command:
$ python3 autograder.py c_test -l layers/resnet_conv2_x_params.json
9Figure 4: Click on (A) to switch to the Schedule step and click on the dropdown in (B) to
switch between blocks.
10Figure 5: Click on the Hierarchy tab in (A) to go through the design hierarchy and find
the ccs ccore en signal for a ProcessingElement. Click the start button in (B) to run the
simulation.
If you would like to run an RTL test, use the command:
$ python3 autograder.py rtl_test -l layers/small_layer1.json -v
If you want to debug the output of the commands that the autograder is
running, use the verbose option -v or run the layer manually as indicated in the previous
subsection.
Because without optimization your resultant RTL implementation will be very slow, we
created 3 smaller layers for you to run RTL tests with. To receive full credits, you will
need to pass all ResNet-18 layers for c test, but only small layer1, small layer2,
11and small layer3 in the layers folder for rtl test.
If you do not pass in any layer json file, the autograder will use small layer1.json. You
can pass in as many layers at a time as you want, just leave a space in between the JSON
file names. Also, if you would like to test all the ResNet-18 layers or small layers at once,
use the commands:
$ python3 autograder.py c_test -l all
$ python3 autograder.py rtl_test -l small
The RTL test command will set up your RTL simulation with the specified layers and run
the test. The RTL test has to compile your HLS code to RTL and run a lengthy simulation,
so this command will take a while.
Note that if you get an error in your RTL test, a useful way to debug is to look at the
waveform. Use the make rtl test command to pull up simulation waveform viewer and
refer to the previous section on running the simulation.
If you need any help with the commands for the autograder use:
$ python3 autograder.py --help
8
Code Submission and Grading
This homework is considered complete when it passes the C tests and RTL tests mentioned
above. Please submit your code as a .tar.gz file to Gradescope. To pack things up, use:
$ tar -czvf filename.tar.gz /path/to/your/directory
You will not be graded for performance on this assignment. Feel free to start optimizing
the design using what you have learned in class to get an early start on the next assignment.
Please name your files using your and your partner’s full names:
• firstname1 lastname1-firstname2 lastname2.tar.gz
12
