all : sim slc

sim : sml_simulator.o sml_opcodes.o
	gcc sml_simulator.o sml_opcodes.o -o sim

slc : sl_compiler.o expr_compiler.o sml_opcodes.o
	gcc sl_compiler.o expr_compiler.o sml_opcodes.o -o slc

sml_simulator.o : sml_simulator.c
	gcc -c sml_simulator.c

sml_opcodes.o : sml_opcodes.c sml_opcodes.h
	gcc -c sml_opcodes.c

sl_compiler.o: sl_compiler.c sl_compiler.h expr_compiler.h
	gcc -c sl_compiler.c

expr_compiler.o: expr_compiler.c expr_compiler.h sl_compiler.h
	gcc -c expr_compiler.c
