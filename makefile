consumer: consumer-signed-data1.cpp
	g++ -o consumer1 consumer-signed-data1.cpp -std=c++14 -lndn-cxx -lboost_system
getPrefix: producer-getPrefix.cpp
	g++ -o producer-getPrefix producer-getPrefix.cpp -std=c++14 -lndn-cxx -lboost_system
ca: CA.cpp
	g++ -o ca CA.cpp -std=c++14 -lndn-cxx -lboost_system
temp : temp.cpp
	g++ -o temp temp.cpp -std=c++14 -lndn-cxx -lboost_system
consumer-getCert : consumer-getCert.cpp
	g++ -o consumer-getCert consumer-getCert.cpp -std=c++14 -lndn-cxx -lboost_system
clean:
	rm send recv
