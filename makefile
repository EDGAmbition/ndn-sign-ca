consumer: ./src/consumer-signed-data1.cpp
	g++ -o ./build/consumer1 ./src/consumer-signed-data1.cpp -std=c++14 -lndn-cxx -lboost_system
	
getPrefix: ./src/producer-getPrefix.cpp
	g++ -o ./build/producer-getPrefix ./src/producer-getPrefix.cpp -std=c++14 -lndn-cxx -lboost_system
	
ca: ./src/CA.cpp
	g++ -o ./build/ca ./src/CA.cpp -std=c++14 -lndn-cxx -lboost_system
	
temp : ./src/temp.cpp
	g++ -o ./build/temp ./src/temp.cpp -std=c++14 -lndn-cxx -lboost_system
	
consumer-getCert : ./src/consumer-getCert.cpp
	g++ -o ./build/consumer-getCert ./src/consumer-getCert.cpp -std=c++14 -lndn-cxx -lboost_system
	
consumer-signed-data : ./src/consumer-signed-data.cpp
	g++ -o ./build/consumer-signed-data ./src/consumer-signed-data.cpp -std=c++14 -lndn-cxx -lboost_system
producer-signed-data : ./src/producer-signed-data.cpp
	g++ -o ./build/producer-signed-data ./src/producer-signed-data.cpp -std=c++14 -lndn-cxx -lboost_system

clean:
	rm send recv
