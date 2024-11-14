
solver.exe: main.cpp instance.h
	g++ -o solver.exe main.cpp -Wall -Wextra -Wshadow -std=c++20 -O2

.SILENT:
.ONESHELL:
run: solver.exe
	rm -rf ../instances/sol*
	rm -rf ../instances/*.json-partition*.txt
	for file in `ls ../instances/*.json` 
	do
		echo $$file
		./solver.exe $$file
		echo ""
	done

large2: solver.exe
	./solver.exe ../instances/large_2.json

clean:
	rm -rf solver.exe