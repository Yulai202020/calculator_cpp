all:
	g++ -I cparse -std=c++11 -c builtin-features.cpp -o my-features.o
	g++ -I cparse -std=c++11 my-features.o build/core-shunting-yard.o main.cpp -o main -lreadline