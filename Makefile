all: clean
	g++ -o gps_attach main.cpp ShapeReader.cpp RTreeLoader.cpp TrackRAInterface.cpp JobRunner.cpp ShapeWriter.cpp -std=c++11 -pthread -g

test:
	g++ -o ./test/test_1 ./test/test_1.cpp ShapeReader.cpp -std=c++11 -lboost_system -lboost_unit_test_framework 
	./test/test_1 --show_progress

clean:
	rm -rf gps_attach
	find ./test/ -executable -name "*test*" -type f | xargs rm -rf

.PHONY: all test clean