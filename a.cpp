#include <libhcs.h>
// #include <libhcs/src/com/parson.h>
#include "libhcs/pcs.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
void writeFile(string nameFile, pcs_private_key *prK,pcs_public_key *pK)
{

    ofstream myfile;
    myfile.open(nameFile);
    myfile << pcs_export_private_key(prK);
    
    myfile.close();
}
string readFile(string nameFile)
{
    ifstream t(nameFile);
    stringstream ss;
    ss << t.rdbuf();
    string s = ss.str();

    return s;
}

int main()
{

    pcs_public_key *pK = pcs_init_public_key();
    pcs_private_key *prK = pcs_init_private_key();
    hcs_random *hr = hcs_init_random();
    pcs_generate_key_pair(pK, prK, hr, 128);
    
    writeFile("prk.json",prK);
    writeFile("pk.json",pK);
    
    return 0;
}
