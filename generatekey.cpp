#include <gmp.h>
#include <libhcs.h>
#include <libhcs/pcs_t.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

void exportKey(string nameFile, pcs_private_key *vk,pcs_public_key *pk)
{

    ofstream pub, priv;
    priv.open(nameFile + "-private.json");
    priv << pcs_export_private_key(vk);
    priv.close();

    pub.open(nameFile + "-pub.json");
    pub << pcs_export_public_key(pk);
    pub.close();

    
}

string readFile(string nameFile)
{
    ifstream t(nameFile);
    stringstream ss;
    ss << t.rdbuf();
    return ss.str();
}

int inputKeyLength()
{
    int keyLength = 0;
    cout << "Enter key Length" << endl;
    cin >> keyLength;
    return keyLength;
}


int main()
{
    pcs_public_key *pk = pcs_init_public_key();
    pcs_private_key *vk = pcs_init_private_key();
    hcs_random *hr = hcs_init_random();
    int keyLength = inputKeyLength();

    pcs_generate_key_pair(pk, vk, hr, keyLength);
    exportKey("key",vk, pk);

    return 0;
}

// Run File : g++ generatekey.cpp -o generatekey -lhcs