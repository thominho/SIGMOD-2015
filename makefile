all:
	g++ -g -o main sources/Main.cpp sources/Journal.cpp sources/ListVal.cpp sources/ExtendableHashtable.cpp sources/ListVal2.cpp sources/TridHash.cpp sources/ValidationPredicate.cpp sources/ValidationHash.cpp sources/zombie.cpp sources/ListVal3.cpp sources/JobScheduler.cpp sources/Bitmap.cpp -pthread -std=c++11
