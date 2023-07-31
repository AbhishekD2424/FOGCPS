# Run the 'make' command from your terminal to execute fog.cpp file and create a output file named fog
#Run 'make run' to execute the code fog, before executing make sure that the path of a.param file is set properly
#Run 'make clean' to delete all the object files

all: fog run

fog:
	@echo "Compiling fog.cpp..."
	g++ -o fog fog.cpp -L. -lgmp -lpbc

run:
	./fog <~/Documents/FOGCPS/13/a.param

clean:
	@echo "Removing all object files..."	
	rm fog


