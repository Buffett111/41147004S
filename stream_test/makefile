all:
	g++ sender.cpp -o sender `pkg-config --cflags --libs opencv4` -lboost_system -lpthread
	g++ receiver.cpp -o receiver `pkg-config --cflags --libs opencv4` -lboost_system -lpthread
	
clean:
	rm -f sender receiver