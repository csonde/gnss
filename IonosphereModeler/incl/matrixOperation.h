#ifndef MATRIXOPERATION_H
#define MATRIXOPERATION_H

#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include <containers.h>
#include <ionosphereGrid.h>

extern const double freqC1;
extern const double freqP2;

gsl_matrix* createAlakMatrix(IonoVariables** vars, Measurement* measRoot, QuadraticGrid* grid);

void kiegyenlites(gsl_matrix* alakMatrix, gsl_matrix* sulyMatrix,
                  gsl_vector* paramStartVektor, gsl_vector* paramJavVektor,
                  gsl_vector* meresVektor, gsl_vector* meresJavVektor,
                  gsl_vector* allandok, int iterations);

#endif //MATRIXOPERATION_H