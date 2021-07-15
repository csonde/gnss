rinexparser_make:
		mkdir -p ./obj
		gcc  -g -o ./obj/rinexCommon.o -Wall -fPIC -c ./src/rinexCommon.c -I ./incl
		gcc  -g -o ./obj/observationParser.o -Wall -fPIC -c ./src/observationParser.c -I ./incl
		gcc  -g -o ./obj/gpsNavigationParser.o -Wall -fPIC -c ./src/gpsNavigationParser.c -I ./incl
		gcc  -g -o ./obj/glonassNavigationParser.o -Wall -fPIC -c ./src/glonassNavigationParser.c -I ./incl
		gcc  -g -o ./obj/meteorologicalParser.o -Wall -fPIC -c ./src/meteorologicalParser.c -I ./incl
		mkdir -p ./bin
		gcc  -g -shared -o ./bin/librinexparser.so.1.0 ./obj/*.o
		mkdir -p ~/lib
		ln -sf `pwd`/bin/librinexparser.so.1.0 ~/lib/librinexparser.so.1
		ln -sf `pwd`/bin/librinexparser.so.1.0 ~/lib/librinexparser.so