#include "iono_PCA.h"
#include <iostream>
#include <fstream>

using namespace std;

int main()
{
    Iono_PCA iono_PCA;
    iono_PCA.LoadDataFromDir("../../testData/eldens_profile");
    cout << "raw data" << endl << iono_PCA.rawDensityData << endl;
    PCA_Components pca_Components(iono_PCA.calculatePCAs());
    filebuf fb;
    fb.open ("../../testData/principal_components.txt", ios::out);
    ostream os(&fb);
    os << pca_Components.principalComponents.transpose() << endl;
    fb.close();
    fb.open ("../../testData/heightData.txt", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << iono_PCA.heightData << endl;
    fb.close();
    fb.open ("../../testData/partial_variances.txt", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << pca_Components.partialVariances << endl;
    fb.close();
    return 0;
}