#include "libused.h"

string inputImagePath()
{
    string path;
    cout << "Enter image's name" << endl;
    cin >> path;
    return path;
}

string readFile(string nameFile)
{
    ifstream t(nameFile);
    stringstream ss;
    ss << t.rdbuf();
    return ss.str();
}

void exportQT(string qt[6])
{
    ofstream o;
    for(int i = 0; i < 6; i ++){
        o.open("tmp/qt" + to_string(i+1) + ".txt");
        o << qt[i];
        o.close();
    }
}

void splitImage(string fileName, Mat &rawImg, Mat bgrimg[])
{
    char *temp = get_current_dir_name();

    string filePath = temp;
    filePath = filePath + "/" + fileName;

    rawImg = imread(filePath);

    split(rawImg, bgrimg);
}

void compress(Mat rawImg, Mat imageComponent[], Mat bgrImg[])
{
    for (int i = 0; i < rawImg.rows; i++)
        for (int j = 0; j < rawImg.cols; j++)
        {
            int temp;

            if (bgrImg[0].at<uchar>(i, j) != 0)
            {
                temp = 0 + rand() % ((bgrImg[0].at<uchar>(i, j) + 1) - 0);
                imageComponent[0].at<uchar>(i, j) = temp;
                imageComponent[1].at<uchar>(i, j) = bgrImg[0].at<uchar>(i, j) - temp;
            }
            else if (bgrImg[0].at<uchar>(i, j) == 0)
            {
                imageComponent[0].at<uchar>(i, j) = 0;
                imageComponent[1].at<uchar>(i, j) = 0;
            }
            if (bgrImg[1].at<uchar>(i, j) != 0)
            {
                temp = 0 + rand() % ((bgrImg[1].at<uchar>(i, j) + 1) - 0);
                imageComponent[2].at<uchar>(i, j) = temp;
                imageComponent[3].at<uchar>(i, j) = bgrImg[1].at<uchar>(i, j) - temp;
            }
            else if (bgrImg[0].at<uchar>(i, j) == 0)
            {
                imageComponent[2].at<uchar>(i, j) = 0;
                imageComponent[3].at<uchar>(i, j) = 0;
            }
            if (bgrImg[2].at<uchar>(i, j) != 0)
            {
                temp = 0 + rand() % ((bgrImg[2].at<uchar>(i, j) + 1) - 0);
                imageComponent[4].at<uchar>(i, j) = temp;
                imageComponent[5].at<uchar>(i, j) = bgrImg[2].at<uchar>(i, j) - temp;
            }
            else if (bgrImg[0].at<uchar>(i, j) == 0)
            {
                imageComponent[4].at<uchar>(i, j) = 0;
                imageComponent[5].at<uchar>(i, j) = 0;
            }
        }
}

void paillier_encryption(Mat rawMat, Mat &destinationMat, pcs_public_key *pk, hcs_random *hr, string &qt)
{
    destinationMat = rawMat.clone();
    for (int i = 0; i < rawMat.rows; i++)
        for (int j = 0; j < rawMat.cols; j++)
        {
            while (true)
            {
                bool break_flag = false;
                mpz_t encryptedValue, newValue, modNumber, quotient;
                mpz_inits(encryptedValue, newValue, modNumber, quotient, NULL);
                mpz_set_ui(encryptedValue, rawMat.at<uchar>(i, j));
                mpz_set_ui(newValue, 0);
                mpz_set_ui(modNumber, 257);
                mpz_set_ui(quotient, 0);
                pcs_encrypt(pk, hr, encryptedValue, encryptedValue);
                mpz_mod(newValue, encryptedValue, modNumber);
                if (mpz_cmp_ui(newValue, 255) < 0)
                {
                    mpz_fdiv_q_ui(quotient, encryptedValue, 257);
                    destinationMat.at<uchar>(i, j) = mpz_get_ui(newValue);
                    break_flag = true;
                    char *charQuotient = 0;
                    charQuotient = mpz_get_str(charQuotient, 10, quotient);
                    string strQuotient = charQuotient;
                    qt += strQuotient + ";";
                    break_flag = true;
                }
                mpz_clears(encryptedValue, newValue, modNumber, quotient, NULL);
                if (break_flag == true)
                    break;
            }
        }
}

void encrypt(pcs_public_key *pK, hcs_random *hr,
             Mat imageComponent[],
             string qt[],
             Mat encryptImg[])
{

    cout << "Encrypting,please wait for a while" << endl;

    future<void> t1 = async(paillier_encryption, imageComponent[0], ref(encryptImg[0]), pK, hr, ref(qt[0]));
    future<void> t2 = async(paillier_encryption, imageComponent[1], ref(encryptImg[1]), pK, hr, ref(qt[1]));
    future<void> t3 = async(paillier_encryption, imageComponent[2], ref(encryptImg[2]), pK, hr, ref(qt[2]));
    future<void> t4 = async(paillier_encryption, imageComponent[3], ref(encryptImg[3]), pK, hr, ref(qt[3]));
    future<void> t5 = async(paillier_encryption, imageComponent[4], ref(encryptImg[4]), pK, hr, ref(qt[4]));
    future<void> t6 = async(paillier_encryption, imageComponent[5], ref(encryptImg[5]), pK, hr, ref(qt[5]));

    t1.get();
    t2.get();
    t3.get();
    t4.get();
    t5.get();
    t6.get();
}

int main()
{
    Mat bgrImg[3], rawImg;
    Mat imageComponent[6], encrptImage[6];
    string fileName = inputImagePath();   //nhap file
    splitImage(fileName, rawImg, bgrImg); //tach anh thanh 3 kenh mau R G B

    string qt[6];
    for (int i = 0; i < 6; i++)
    {
        qt[i] = "";
    }

    imageComponent[0] = bgrImg[0].clone();
    imageComponent[1] = bgrImg[0].clone();
    imageComponent[2] = bgrImg[1].clone();
    imageComponent[3] = bgrImg[1].clone();
    imageComponent[4] = bgrImg[2].clone();
    imageComponent[5] = bgrImg[2].clone();

    compress(rawImg, imageComponent, bgrImg);

    pcs_public_key *pk = pcs_init_public_key();
    pcs_private_key *vk = pcs_init_private_key();
    hcs_random *hr = hcs_init_random();
    
    // IMPORT PUBKEY TO ENCRYPT IMAGE
    string x = readFile("key-pub.json");
    const char * xx = x.c_str();
    pcs_import_public_key(pk,xx);

    // ENCRYPTION RECORD TIME
    auto start = chrono::steady_clock::now();

    encrypt(pk, hr, imageComponent, qt, encrptImage);

    auto end = chrono::steady_clock::now();
    cout <<"Encryption time: ";
    cout << chrono::duration <double> (end-start).count() << "s" << endl;

    // Draw image from Matrix
    imwrite("tmp/p1.png", encrptImage[0]);
    imwrite("tmp/p2.png", encrptImage[1]);
    imwrite("tmp/p3.png", encrptImage[2]);
    imwrite("tmp/p4.png", encrptImage[3]);
    imwrite("tmp/p5.png", encrptImage[4]);
    imwrite("tmp/p6.png", encrptImage[5]);

    // Write quotion to file.
    exportQT(qt);
    return 0;
}