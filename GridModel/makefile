geometryPrimitives_make:
		mkdir -p ./obj
		gcc  -g -o ./obj/geometryPrimives.o -Wall -fPIC -c ./src/geometryPrimitives.c -I ./incl -lm
		gcc  -g -o ./obj/ionosphereGrid.o -Wall -fPIC -c ./src/ionosphereGrid.c -I ./incl -lm
		mkdir -p ./bin
		gcc  -g -shared -o ./bin/libgridmodel.so.1.0 ./obj/*.o
		mkdir -p ~/lib
		ln -sf `pwd`/bin/libgridmodel.so.1.0 ~/lib/libgridmodel.so.1
		ln -sf `pwd`/bin/libgridmodel.so.1.0 ~/lib/libgridmodel.so