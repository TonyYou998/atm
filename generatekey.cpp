#include <libhcs.h>
#include "libhcs/pcs.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

void writeFile(string nameFile, pcs_private_key *prK,pcs_public_key *pK)
{

    ofstream pub, priv;
    priv.open(nameFile + "-private.json");
    priv << pcs_export_private_key(prK);
    priv.close();

    pub.open(nameFile + "-pub.json");
    pub << pcs_export_public_key(pK);
    pub.close();
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
    
    writeFile("key",prK, pK);
    
    return 0;
}

// Run File : g++ generatekey.cpp -o generatekey -lhcs