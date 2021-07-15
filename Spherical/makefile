spherical_make:
	mkdir -p ./bin
	g++ -g -o ./bin/spherical ./src/*.cpp -I ./incl -I ../GCP/incl -L ~/lib -lm -lrinexparser -lgmpxx -lgmp
	cp ../testData/almanac.yuma.week0711.589824.txt ./bin
