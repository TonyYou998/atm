#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <gmp.h>    
#include <libhcs.h> 
#include <fstream>  
#include <thread>   
#include <future>   
#include <unistd.h> 
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <sstream>

using namespace std;
using namespace cv;

string inputImagePath(){
    string path;
    cout<<"Enter image's name"<<endl;
    cin>>path;
    return path;

}
int inputKeyLength(){
    int keyLength=0;
    cout<<"Enter key Length"<<endl;
    cin>>keyLength;
    return keyLength;

}
void splitImage(string fileName,Mat &rawImg,Mat &p1,Mat &p2, Mat &p3){
    char * temp=get_current_dir_name();

    string filePath=temp;
    filePath=filePath+"/"+fileName;
    
    rawImg=imread(filePath);
    Mat bgrImg[3];
    split(rawImg,bgrImg);
    p1=bgrImg[0];
    p2=bgrImg[1];
    p3=bgrImg[2];
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

void paillier_decryption(Mat img1, Mat img2, Mat &destinationMat,vector<string> quotes1, vector<string> quotes2, pcs_public_key* pk, pcs_private_key* vk)
{
    int index = 0;
    for (int i=0;i<destinationMat.rows;i++)
        for (int j=0;j<destinationMat.cols;j++)
        {
            mpz_t number1, number2, result;
            const char * c1 = quotes1[index].c_str();
            const char * c2 = quotes2[index].c_str();
            mpz_init(number1);
            mpz_init(number2);
            mpz_init(result);
            mpz_set_str(number1,c1,10);
            mpz_set_str(number2,c2,10);
            mpz_mul_ui(number1,number1,257);
            mpz_mul_ui(number2,number2,257);
            mpz_add_ui(number1,number1,img1.at<uchar>(i,j));
            mpz_add_ui(number2,number2,img2.at<uchar>(i,j));
            pcs_decrypt(vk,number1,number1);
            pcs_decrypt(vk,number2,number2);
            mpz_add(result, number1, number2);
            unsigned long int recoveredValue =  mpz_get_ui (result);
            destinationMat.at<uchar>(i,j) = recoveredValue;
            index++;
        }
}


void compress( Mat rawImg,Mat &imageComponent0,Mat &imageComponent1,Mat &imageComponent2,Mat &imageComponent3,Mat &imageComponent4,Mat &imageComponent5,Mat bgrImg1,Mat bgrImg2,Mat bgrImg3){
        for (int i=0;i<rawImg.rows;i++)
        for (int j=0;j<rawImg.cols;j++)
        {
            int temp;
            
            if (bgrImg1.at<uchar>(i,j)!=0)
            {
                temp = 0 + rand() % (( bgrImg1.at<uchar>(i,j) + 1 ) - 0);
                imageComponent0.at<uchar>(i,j) = temp;
                imageComponent1.at<uchar>(i,j) = bgrImg1.at<uchar>(i,j) - temp;
            } else if (bgrImg1.at<uchar>(i,j) ==0)
            {
                imageComponent0.at<uchar>(i,j) = 0;
                imageComponent1.at<uchar>(i,j) = 0;
            }
            if (bgrImg2.at<uchar>(i,j)!=0)
            {
                temp = 0 + rand() % (( bgrImg2.at<uchar>(i,j) + 1 ) - 0);
                imageComponent2.at<uchar>(i,j) = temp;
                imageComponent3.at<uchar>(i,j) = bgrImg2.at<uchar>(i,j) - temp;
            }
            else if (bgrImg1.at<uchar>(i,j) ==0)
            {
                imageComponent2.at<uchar>(i,j) = 0;
                imageComponent3.at<uchar>(i,j) = 0;
            }
            if (bgrImg3.at<uchar>(i,j)!=0)
            {
                temp = 0 + rand() % (( bgrImg3.at<uchar>(i,j) + 1 ) - 0);
                imageComponent4.at<uchar>(i,j) = temp;
                imageComponent5.at<uchar>(i,j) = bgrImg3.at<uchar>(i,j) - temp;
            } else if (bgrImg1.at<uchar>(i,j) ==0)
            {
                imageComponent4.at<uchar>(i,j) = 0;
                imageComponent5.at<uchar>(i,j) = 0;
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

void decrypt(pcs_public_key *pk, pcs_private_key *vk, Mat pic [], Mat decryptImage [], vector<string> quotesPart[])
{
    future<void> t7 = async(paillier_decryption, pic[0], pic[1], ref(decryptImage[0]), quotesPart[0], quotesPart[1], pk, vk);
    future<void> t8 = async(paillier_decryption, pic[2], pic[3], ref(decryptImage[1]), quotesPart[2], quotesPart[3], pk, vk);
    future<void> t9 = async(paillier_decryption, pic[4], pic[5], ref(decryptImage[2]), quotesPart[4], quotesPart[5], pk, vk);

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
int main(){
    Mat bgrImg1,bgrImg2,bgrImg3,rawImg;
    Mat imageComponent[6],encrptImage[6];
    string fileName=inputImagePath();//nhap file
    int keyLength=inputKeyLength();//nhap key length
    splitImage(fileName,rawImg,bgrImg1,bgrImg2,bgrImg3);//tach anh thanh 3 kenh mau R G B
   
    imageComponent[0]=bgrImg1.clone();
    imageComponent[1]=bgrImg1.clone();
    imageComponent[2]=bgrImg2.clone();
    imageComponent[3]=bgrImg2.clone();
    imageComponent[4]=bgrImg3.clone();
    imageComponent[5]=bgrImg3.clone();
    compress(rawImg,imageComponent[0],imageComponent[1],imageComponent[2],imageComponent[3],imageComponent[4],imageComponent[5],bgrImg1,bgrImg2,bgrImg3);//serialize pic
    
    auto start = chrono::steady_clock::now();
    // generate key
    pcs_public_key *pK=pcs_init_public_key();
    pcs_private_key *prK=pcs_init_private_key();
    hcs_random *hr=hcs_init_random();
    pcs_generate_key_pair(pK,prK,hr,keyLength);
    string qt[6];
    for (int i=0;i<6;i++)
    {
        qt[i] = "";
    }


    encrypt(pK, hr, imageComponent, qt, encrptImage);

    auto end = chrono::steady_clock::now();
    cout <<"Encryption time: ";
    cout << chrono::duration <double> (end-start).count() << "s" << endl;

    vector<string> quotesPart[6];

    boost::split(quotesPart[0], qt[0], boost::is_any_of(";"));
    boost::split(quotesPart[1], qt[1], boost::is_any_of(";"));
    boost::split(quotesPart[2], qt[2], boost::is_any_of(";"));
    boost::split(quotesPart[3], qt[3], boost::is_any_of(";"));
    boost::split(quotesPart[4], qt[4], boost::is_any_of(";"));
    boost::split(quotesPart[5], qt[5], boost::is_any_of(";"));

    imwrite("E1.png",encrptImage[0]);
    imwrite("E2.png",encrptImage[1]);
    imwrite("E3.png",encrptImage[2]);
    imwrite("E4.png",encrptImage[3]);
    imwrite("E5.png",encrptImage[4]);
    imwrite("E6.png",encrptImage[5]);

    Mat decryptImage[3];

    decryptImage[0]=encrptImage[0].clone();
    decryptImage[1]=encrptImage[0].clone();
    decryptImage[2]=encrptImage[0].clone();

    ofstream o;
    o.open("encrptImage");
    o << encrptImage[0];
    o.close();

    cout<<"Start Decryption..."<<endl;
    auto start_decrypt = chrono::steady_clock::now();

    decrypt(pK,prK,encrptImage, decryptImage ,quotesPart);

    Mat result;
    vector<Mat> vectorOfRecovered;
    
    mergeImage(vectorOfRecovered, decryptImage, result);


    imwrite("DecryptImage.jpg",result);
    auto end_decrypt = chrono::steady_clock::now(); 
   

    cout<<"Decryption time is: ";
    cout<<chrono::duration <double> (end_decrypt-start_decrypt).count() << "s" << endl;

    pcs_free_private_key(prK);
    pcs_free_public_key(pK);


    return 0;



    



    








  

  



     


    

    
    
    
}