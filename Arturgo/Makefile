
solver.exe: main.cpp instance.h
	g++ -o solver.exe main.cpp -Wall -Wextra -Wshadow -std=c++20 -O2

.SILENT:
.ONESHELL:
run: solver.exe
	rm -rf ../instances/sol*
	for file in `ls ../instances/*` 
	do
		echo $$file
		./solver.exe $$file
		echo ""
	done

clean:
	rm -rf solver.exe