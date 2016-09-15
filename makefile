graph.o:
	g++ -O2 -c maxflow/graph.cpp -o maxflow/graph.o
maxflow.o:
	g++ -O2 -c maxflow/maxflow.cpp -o maxflow/maxflow.o
EasyBMP.o:
	g++ -O2 -c EasyBMP/EasyBMP.cpp -o EasyBMP/EasyBMP.o
main: main.cpp maxflow/graph.o maxflow/maxflow.o EasyBMP/EasyBMP.o
	g++ -g -O2 main.cpp -o main maxflow/graph.o maxflow/maxflow.o EasyBMP/EasyBMP.o -IEasyBMP -Imaxflow
clean:
	rm main
	rm maxflow/*.o
	rm EasyBMP/*.o
