All: main exe

main: main.cpp
	g++ main.cpp -o main

exe: main
	./main