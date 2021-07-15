#include <matrixOperation.h>
#include <assert.h>


const double freqC1 = 1575420000;
const double freqP2 = 1227600000;

gsl_matrix* createAlakMatrix(IonoVariables** vars, Measurement* measRoot, QuadraticGrid* grid)
{
    if(*vars || !measRoot)
    {
        return 0;
    }

    *vars = createIonoVariables(measRoot, grid);

    doStationDCBPreorderIndexing((*vars)->stationDCBRoot, 0);
    doSatDCBPreorderIndexing((*vars)->satDCBRoot, 0);
    doCellParameterPreorderIndexing((*vars)->cellParameters, 0);

    int colNum = (*vars)->cellCount;// + (*vars)->stationCount;
    gsl_matrix* retMatrix = gsl_matrix_alloc ((*vars)->corrCount, colNum);

    int i, j;
    for (i = 0; i < (*vars)->corrCount; i++)
    {
        for (j = 0; j < colNum; j++)
        {
            gsl_matrix_set(retMatrix, i, j, 0);
        }
    }

    int measNum = 0;
    while (measRoot)
    {
        double commonCoeff = 40.3 * (freqP2 * freqP2 - freqC1 * freqC1) / (freqC1 * freqC1 * freqP2 * freqP2);
        //double commonCoeff = (double)1/200000;
        LineSectorList* tmplsl = measRoot->lineSectors;
        while (tmplsl)
        {
            double coeff = commonCoeff * tmplsl->length;
            int cellId = getSingleCellIDByIndexes(tmplsl->layerId, tmplsl->lateralId, tmplsl->longitudinalId, grid);
            int cellIndex = getCellParameterPreorderIndex(cellId, (*vars)->cellParameters);
            gsl_matrix_set(retMatrix, measNum, cellIndex, coeff*1000000000000);
            tmplsl = tmplsl-> next;
        }
        //int stationIndex = (*vars)->cellCount + getStationDCBPreorderIndex(measRoot->recId, (*vars)->stationDCBRoot);
        //gsl_matrix_set(retMatrix, measNum, stationIndex, 1);
        measRoot = measRoot->next;
        measNum++;
    }
    return retMatrix;
}


void kiegyenlites(gsl_matrix* alakMatrix, gsl_matrix* sulyMatrix,
                  gsl_vector* paramStartVektor, gsl_vector* paramJavVektor,
                  gsl_vector* meresVektor, gsl_vector* meresJavVektor,
                  gsl_vector* allandok, int iterations)
{
    int measCount = alakMatrix->size1;
    int paramCount = alakMatrix->size2;

    gsl_vector* a0Vektor = gsl_vector_alloc(measCount);
    gsl_vector* tisztaTag_vektor = gsl_vector_alloc(measCount);

    int i;
    for(i = 0; i < measCount; i++)
    {
        gsl_vector_set(a0Vektor, i, 0);
        gsl_vector_set(tisztaTag_vektor, i, 0);
    }

    gsl_matrix* ATrxP = gsl_matrix_alloc(paramCount, measCount);
    gsl_matrix* ATrxPxA = gsl_matrix_alloc(paramCount, paramCount);
    gsl_matrix* _ATrxPxA_inv = gsl_matrix_alloc(paramCount, paramCount);
    gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1, alakMatrix, sulyMatrix, 0, ATrxP);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1, ATrxP, alakMatrix, 0, ATrxPxA);
    gsl_matrix* ATrxPxACopy = gsl_matrix_alloc(paramCount, paramCount);
    gsl_matrix_memcpy (ATrxPxACopy, ATrxPxA);
/*
    for(i = 0; i < paramCount; i++)
    {
        int j;
        for(j = 0; j < paramCount; j++)
        {
            printf("ATrxPxA(%d,%d): %le\n", i, j, gsl_matrix_get(ATrxPxA, i, j));
        }
    }
*/
    int s;
    gsl_permutation * p = gsl_permutation_alloc(paramCount);
    gsl_linalg_LU_decomp (ATrxPxA, p, &s);
    gsl_linalg_LU_invert(ATrxPxA, p, _ATrxPxA_inv);

    gsl_matrix* eccseg = gsl_matrix_alloc(paramCount, paramCount);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1, ATrxPxACopy, _ATrxPxA_inv, 0, eccseg);
/*
    for(i = 0; i < paramCount; i++)
    {
        int j;
        for(j = 0; j < paramCount; j++)
        {
            printf("_ATrxPxA_inv(%d,%d): %le\n", i, j, gsl_matrix_get(_ATrxPxA_inv, i, j));
        }
    }
*/
    for(i = 0; i < paramCount; i++)
    {
        int j;
        for(j = 0; j < paramCount; j++)
        {
            printf("eccseg(%d,%d): %le\n", i, j, gsl_matrix_get(eccseg, i, j));
        }
    }

    gsl_matrix_free(eccseg);

    printf("Calculating parameters\n");
    for(i = 0; i < iterations; i++)
    {
        printf("    Iteration %d\n", i);

        gsl_blas_dgemv(CblasNoTrans, 1, alakMatrix, paramStartVektor, 0, a0Vektor);

        gsl_vector_memcpy(tisztaTag_vektor, meresVektor);
        gsl_vector_sub(meresVektor, a0Vektor);
        gsl_vector_add(meresVektor, allandok);
        gsl_vector_swap(tisztaTag_vektor, meresVektor);
        int j;
/*
        for (j = 0; j < measCount; j++)
        {
            printf("tisztatagvektor(%d): %le\n", j, gsl_vector_get(tisztaTag_vektor, j));
        }
*/
        gsl_vector* ATrxPxl = gsl_vector_alloc(paramCount);
        gsl_blas_dgemv(CblasNoTrans, 1, ATrxP, tisztaTag_vektor, 0, ATrxPxl);

        gsl_blas_dgemv(CblasNoTrans, 1, _ATrxPxA_inv, ATrxPxl, 0, paramJavVektor);
        gsl_vector* Axparam_vektor = gsl_vector_alloc(measCount);
        gsl_blas_dgemv(CblasNoTrans, 1, alakMatrix, paramJavVektor, 0, Axparam_vektor);
        gsl_vector_memcpy(meresJavVektor, Axparam_vektor);
        gsl_vector_sub(Axparam_vektor, tisztaTag_vektor);
        gsl_vector_swap(meresJavVektor, Axparam_vektor);

        gsl_vector_free(Axparam_vektor);
        gsl_vector_free(ATrxPxl);

        gsl_vector_add(paramStartVektor, paramJavVektor);
    }

    gsl_vector_free(a0Vektor);
    gsl_vector_free(tisztaTag_vektor);

    gsl_matrix_free(ATrxP);
    gsl_matrix_free(ATrxPxA);
    gsl_matrix_free(_ATrxPxA_inv);

    gsl_permutation_free(p);
}
