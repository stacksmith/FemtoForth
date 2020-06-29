FemtoForth
==========

This is an experimental Forth-like VM using a novel 8-bit token interpreter that uses a sliding-window algorithm to decode  tokens to full addresses.  The technology is described in great detail at http://www.fpgarelated.com/showarticle/44.php.  Succinctly speaking, **while classic 8-bit tokens are limited to 256 potential meanings, FemtoForth has no such limits** - for all practical purposes. The ultimate goal of the project is to create an FPGA-based CPU executing this bytecode directly.

This proof-of-concept implementation shows that it is in fact possible to construct a working language using a sliding window interpreter.  This goal has been met, although I got bogged down with the language part and will now take a break (and work with ~~Smalltalk~~ ~~Lisp~~ or something).

FemtoForth (it is not quite Forth, but is Forth-like in spirit) runs on ARM7 (Android console) and x86 (32-bit) linux.  It is easily portable to many other processors (by writing a new set of kernel routines) and can be used as an embedded development system, a Domain-Specific language, or just a way to mess around with this weird tech.

The system boots as a C application which allocates memory and provides basic language services (which are ideally replaced by native services after boot).  The rest of the system consists a binary image, and headers containing name and source (yes, source).

Status
======
Language boots up on android and x86.  After loading, very little functionality is available (from kernel assembly routines and C command processor).  Type in:
```
load basics.ff
```
to make some extra words available.

Try a stupid benchmark (just a billion-times loop) called timit2.  It runs in 4 seconds on my x86 box.

The environment is much like a linux shell, use pwd, cd xxx or cd .. to move around.  ls will give you a listing of the directory you are in (any word can be a directory).  
```
list timit2
```
 to see the source.
```
load deco.ff
ref timit2 16 deco_n
```
to decompile the source.

sys show some system data.
q dumps it out.

You can always do forth-like stuff like
```
1 2 + .
( "hello World" ctell )
```
Any word can become an 'operator' by appending an open paren and compiling the right-hand-side expression before a close paren.
So these are identical:
```
1 2 +
1 +( 2 )
```
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

end
```
Note: end terminates source entry (the source gets copied into the system and then compiled).  { } actually start and stop the compilation process.


See lang.c, cmd.c and kernel.asm too get a sense of what commands are availabse...

See the doc folder (I will be updating it often).



COMPILING:
-----------
Edit Makefile and uncomment the intended target line (x86-linux or ARM android).  In the src directory, make the project.  make run (or just run the executable).

For Android (you may need a rooted device), using an adb shell, create a directory called /data/tmp.  The makefile will push the required files when you execute 'make install'.  You can run FemtoForth directly on the device in a terminal (an external keyboard helps),  r across ADB from your host.

This is very much work in progress, and is barely implemented, so don't expect too much...






TODO:

WATCH OUT:


