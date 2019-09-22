
Hello fellow reader,

This project was far too big to have in a single file, so
much of it was divided into seperate ones. One can compile
the SML simulator and the Simple Language (SL) compiler with
the following steps:

	if you have "make" installed, run the "makefile",
	otherwise:

	to compile the simulator:
	compile sml_simulator.c and sml_opcodes.c together

	to compile the compiler:
	compile sl_compiler.c, expr_compiler.c, and sml_opcodes.c together

For a quick demonstration of my compiler, you can use test.sl. If 
you have any questions or problems, contact me at
afrederiksen@knights.ucf.edu. (;



