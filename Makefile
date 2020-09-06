bin/target: obj/define.o obj/lexical.o obj/ll1.o obj/semantic.o obj/syntax.o obj/output.o obj/main.o
	mingw32-g++ -g -std=c++11 obj/define.o obj/lexical.o obj/ll1.o obj/semantic.o obj/syntax.o obj/output.o obj/main.o -o bin/target

obj/define.o: define.cpp
	mingw32-g++ -c -g -std=c++11 define.cpp -o obj/define.o

obj/lexical.o: lexical.cpp
	mingw32-g++ -c -g -std=c++11 lexical.cpp -o obj/lexical.o

obj/ll1.o: ll1.cpp
	mingw32-g++ -c -g -std=c++11 ll1.cpp -o obj/ll1.o

obj/semantic.o: semantic.cpp
	mingw32-g++ -c -g -std=c++11 semantic.cpp -o obj/semantic.o

obj/syntax.o: syntax.cpp
	mingw32-g++ -c -g -std=c++11 syntax.cpp -o obj/syntax.o

obj/main.o: main.cpp
	mingw32-g++ -c -g -std=c++11 main.cpp -o obj/main.o

obj/output.o: output.cpp
	mingw32-g++ -c -g -std=c++11 output.cpp -o obj/output.o
	
clean:
	rm obj/* bin/*

move:
	cp bin/* ./