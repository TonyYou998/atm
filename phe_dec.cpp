#include "libused.h"

string readFile(string nameFile)
{
    ifstream t(nameFile);
    stringstream ss;
    ss << t.rdbuf();
    return ss.str();
}

void importQT(string qt[]){
    for(int i = 0; i < 6; i ++){
        qt[i] = readFile("tmp/qt" + to_string(i+1) + ".txt");
    }
}

void paillier_decryption(Mat img1, Mat img2, Mat &destinationMat, vector<string> quotes1, vector<string> quotes2, pcs_private_key *vk)
{
    int index = 0;
    for (int i = 0; i < destinationMat.rows; i++)
        for (int j = 0; j < destinationMat.cols; j++)
        {
            mpz_t number1, number2, result;
            const char *c1 = quotes1[index].c_str();
            const char *c2 = quotes2[index].c_str();
            mpz_init(number1);
            mpz_init(number2);
            mpz_init(result);
            mpz_set_str(number1, c1, 10);
            mpz_set_str(number2, c2, 10);
            mpz_mul_ui(number1, number1, 257);
            mpz_mul_ui(number2, number2, 257);
            mpz_add_ui(number1, number1, img1.at<uchar>(i, j));
            mpz_add_ui(number2, number2, img2.at<uchar>(i, j));
            pcs_decrypt(vk,number1,number1);
            pcs_decrypt(vk,number2,number2);
            mpz_add(result, number1, number2);
            unsigned long int recoveredValue = mpz_get_ui(result);
            destinationMat.at<uchar>(i, j) = recoveredValue;
            index++;
        }
}


void decrypt(pcs_private_key *vk, Mat pic [], Mat decryptImage [], vector<string> quotesPart[])
{
    future<void> t7 = async(paillier_decryption, pic[0], pic[1], ref(decryptImage[0]), quotesPart[0], quotesPart[1], vk);
    future<void> t8 = async(paillier_decryption, pic[2], pic[3], ref(decryptImage[1]), quotesPart[2], quotesPart[3], vk);
    future<void> t9 = async(paillier_decryption, pic[4], pic[5], ref(decryptImage[2]), quotesPart[4], quotesPart[5], vk);

    t7.get();
    t8.get();
    t9.get();
}
void mergeImage(vector<Mat> vectorOfRecovered, Mat decryptImage [], Mat &mergeImage)
{
    vectorOfRecovered.push_back(decryptImage[0]);
    vectorOfRecovered.push_back(decryptImage[1]);
    vectorOfRecovered.push_back(decryptImage[2]);

    merge(vectorOfRecovered, mergeImage);
}

void ReadImg(Mat pic [])
{
    char *temp = get_current_dir_name();

    string filePath = temp;
    filePath = filePath + "/tmp/";

    for(int i = 0; i < 6 ; i++){
        pic[i] = imread(filePath + "p" + to_string(i+1) + ".png");
    }
}

int main(){
    vector<string> quotesPart[6];
    Mat pic[6];string qt[6];
    pcs_private_key *vk = pcs_init_private_key();

    // IMPORT PUBKEY,PRIVATEKEY TO ENCRYPT IMAGE
    string x = readFile("key-private.json");
    const char * xx = x.c_str();
    pcs_import_private_key(vk,xx);

    // Import quotion data from encryption image
    
    cout << "Import Quotion data" << endl;
    importQT(qt);
    
    cout << "Start spliting part ..." << endl;
    boost::split(quotesPart[0], qt[0], boost::is_any_of(";"));
    boost::split(quotesPart[1], qt[1], boost::is_any_of(";"));
    boost::split(quotesPart[2], qt[2], boost::is_any_of(";"));
    boost::split(quotesPart[3], qt[3], boost::is_any_of(";"));
    boost::split(quotesPart[4], qt[4], boost::is_any_of(";"));
    boost::split(quotesPart[5], qt[5], boost::is_any_of(";"));

    Mat decryptImage[3];
    ReadImg(pic);

    decryptImage[0] = pic[0].clone();
    decryptImage[1] = pic[0].clone();
    decryptImage[2] = pic[0].clone();


    cout << "Start Decryption..." << endl;
    decrypt(vk, pic, decryptImage, quotesPart);

    imwrite("1.png", decryptImage[0]);
    imwrite("2.png", decryptImage[1]);
    imwrite("3.png", decryptImage[2]);

    Mat result;
    vector<Mat> vectorOfRecovered;

    // Record time
    auto start_decrypt = chrono::steady_clock::now();
    cout << "Merge 3 RGB image to recovered image" << endl;

    mergeImage(vectorOfRecovered, decryptImage, result);

    ofstream o;
    o.open("matrix_error");
    o << result;
    o.close();

    imwrite("DecryptImage.png", result);
    auto end_decrypt = chrono::steady_clock::now();

    cout << "Decryption time is: ";
    cout << chrono::duration<double>(end_decrypt - start_decrypt).count() << "s" << endl;

    pcs_free_private_key(vk);
    return 0;
}