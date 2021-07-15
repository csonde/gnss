ionoshepre_modeler_make:
		mkdir -p ./bin
		gcc -g -I ../GCP/incl -I ../GridModel/incl -I ./incl -L ~/lib ./src/*.c -lrinexparser -lgridmodel -o ./bin/ionosphere_modeler -lm -lgsl -lgslcblas
		cp ./scripts/* ./bin
		cp ../testData/almanac.yuma.week0711.589824.txt ./bin