FemtoForth
==========

This is an experimental language featuring Meow-Meow, a unique 8-bit token interpreter that uses a sliding-window algorithm to decode the tokens to full addresses.  The technology is described in great detail at http://www.fpgarelated.com/showarticle/44.php.  The ultimate goal of the project is to create an FPGA-based CPU executing this bytecode directly.

FemtoForth (it is not quite Forth, but is Forth-like in spirit) runs on ARM7 (Android console) and x86 (32-bit) linux.  It is easily portable to many other processors and can be used as an embedded development system, a Domain-Specific language, or just a way to mess around with this weird tech.

The system boots as a C application which allocates memory and provides basic language services (which are ideally replaced by native services after boot).  The rest of the system consists a binary image, and headers containing name and source (yes, source).

Edit Makefile and uncomment the intended target line (x86-linux or ARM android).  In the src directory, make the project.  make run (or just run the executable).

For Android (you may need a rooted device), using an adb shell, create a directory called /data/tmp.  The makefile will push the required files when you execute 'make install'.  You can run FemtoForth directly on the device in a terminal (an external keyboard helps),  r across ADB from your host.

This is very much work in progress, and is barely implemented, so don't expect too much...

---TEMPORARY---
Once up and running, type 'load basics.ff'.  Things to try:
```
1 2 + .
cd 'system'core
pwd
ls
sys
list splash
// ptr splash see // does not work
PROC test { 1 2 + ; }
|
****Note: the vertical bar as the only thing on a line terminates the definition
test
list test
```
See lang.c, cmd.c and kernel.asm too get a sense of what commands are availabse...

See the doc folder (I will be updating it often).





TODO:
- fix table dump to print full path, no comment
- DONE convert heads to a linear format, for easy saving.
- DONE ' ' fails    
    

WATCH OUT:
- cleanse table
- watch out for table holes
- corner condition: table full, next compile will fail...




+ hexd 
{ $F and '0' +
$39 over <= if 7 + thanx ; }
