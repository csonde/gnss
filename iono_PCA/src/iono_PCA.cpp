#include <eigen3/Eigen/Eigenvalues> 

#include "iono_PCA.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <math.h>

Iono_PCA::Iono_PCA()
{
}


Iono_PCA::~Iono_PCA()
{
}


void Iono_PCA::LoadDataFromDir(const char* dir)
{
    DIR* FD = opendir(dir);
    struct dirent* in_file;
    FILE    *entry_file;

    printf("loading from files\n");

    while ((in_file = readdir(FD))) 
    {
        if (!strcmp (in_file->d_name, "."))
            continue;
        if (!strcmp (in_file->d_name, ".."))    
            continue;

        char filepath[128] = {0};
        strcat(filepath, dir);
        strcat(filepath, "/");
        strcat(filepath, in_file->d_name);
        entry_file = fopen(filepath, "r");
        if (entry_file == NULL)
        {
            fprintf(stderr, "Error : Failed to open electron density profile %s - %s\n", in_file->d_name, strerror(errno));
            exit(-1);
        }

        printf("reading %s\n", in_file->d_name);

        rawDensityData.conservativeResize(20, rawDensityData.outerSize() + 1);
        heightData.conservativeResize(20);

        unsigned int colNum = rawDensityData.outerSize();

        char line[128];
        memset(line, 0, 128);
        unsigned int lineNumber = 1;
        while(fgets(line, 128, entry_file))
        {
            if (lineNumber >= 13 && lineNumber <= 53 && !((lineNumber-13)%10))
            {
                double t1, t2, t3, t4;
                sscanf(line, "%lf%lf%lf%lf%lf", &t1, &t2, &t3, &t4, &rawDensityData((lineNumber-13)/10, colNum-1));
                heightData[(lineNumber-13)/10] = (int)t2;
            }
            else if (lineNumber > 53 && lineNumber <= 83 && !((lineNumber-53)%2))
            {
                double t1, t2, t3, t4;
                sscanf(line, "%lf%lf%lf%lf%lf", &t1, &t2, &t3, &t4, &rawDensityData((lineNumber-53)/2+4, colNum-1));
                heightData[(lineNumber-53)/2+4] = (int)t2;
            }
            lineNumber++;
        }

        heightData = heightData * 1000;

        fclose(entry_file);
    }
    closedir(FD);
}





/*
        loc1    loc2    loc3    ...
    h1  val11   val12   val13
    h2  val21   val22   val23
    h3  val31   val32   val33
    .
    .
    .

            |
            |
            V

        expVal
    h1  expVal1
    h2  expVal2
    h3  expVal3
    .
    .
    .
            |
            |
            V
        h1      h2      h3      ...
    h1  cov11   cov12   cov13
    h2  cov21   cov22   cov23
    h3  cov31   cov32   cov33
    .
    .
    .

        v1    v2    v3      ...
    c1  c11   c12   c13
    c2  c21   c22   c23
    c3  c31   c32   c33
    .
    .
    .
    evs ev1   ev2   ev3

*/

PCA_Components Iono_PCA::calculatePCAs()
{
    PCA_Components retVal;
    long rowNum = rawDensityData.innerSize();
    long colNum = rawDensityData.outerSize();

    if (rowNum == 0 || colNum == 0)
    {
        return retVal;
    }

    MatrixXd expectedValues;
    expectedValues.resize(rowNum, 1);
    for (int i = 0; i < rowNum; i++)
    {
        double sum = 0;
        for (int j = 0; j < colNum; j++)
        {
            sum += rawDensityData(i, j);
        }
        expectedValues(i) = sum / (double)colNum;
    }

    MatrixXd szorasnezzet;
    szorasnezzet.resize(rowNum, 1);
    for (int i = 0; i < rowNum; i++)
    {
        double sum = 0;
        for (int j = 0; j < colNum; j++)
        {
            sum += (rawDensityData(i, j) - expectedValues(i)) * (rawDensityData(i, j) - expectedValues(i));
        }
        szorasnezzet(i) = sum / (double)(colNum - 1);
    }

    MatrixXd covarianceMatrix;
    covarianceMatrix.resize(rowNum, rowNum);
    for (int i = 0; i < rowNum; i++)
    {
        for (int j = 0; j < rowNum; j++)
        {
            double sum = 0;
            for (int k = 0; k < colNum; k++)
            {
                sum += (rawDensityData(i, k) - expectedValues(i)) * (rawDensityData(j, k) - expectedValues(j));
            }
            covarianceMatrix(i, j) = sum / (double)colNum;
        }
    }
    std::cout << "covariance matrix" << std::endl << covarianceMatrix << std::endl;

    MatrixXd correlationMatrix;
    correlationMatrix.resize(rowNum, rowNum);
    for (int i = 0; i < rowNum; i++)
    {
        for (int j = 0; j < rowNum; j++)
        {
            double sum = 0;
            for (int k = 0; k < colNum; k++)
            {
                sum += (rawDensityData(i, k) - expectedValues(i)) * (rawDensityData(j, k) - expectedValues(j));
            }
            correlationMatrix(i, j) = sum / ((double)(colNum - 1) * sqrt(szorasnezzet(i) * szorasnezzet(j)));
        }
    }
    std::cout << "correlation matrix" << std::endl << correlationMatrix << std::endl;

    EigenSolver<MatrixXd> eigenSolver(covarianceMatrix);
    if (eigenSolver.info() != Success)
    {
        return retVal;
    }

    retVal.principalComponents.resize(rowNum, rowNum);
    retVal.principalComponents = eigenSolver.eigenvectors();
    retVal.partialVariances.resize(rowNum, 1);
    retVal.partialVariances = eigenSolver.eigenvalues();

    return retVal;
}
