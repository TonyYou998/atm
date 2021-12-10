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
    cout<<filePath;
    rawImg=imread(filePath);
    Mat bgrImg[3];
    split(rawImg,bgrImg);
    p1=bgrImg[0];
    p2=bgrImg[1];
    p3=bgrImg[2];
}

void paillier_encryption(Mat rawMat,Mat &destinationMat, pcs_public_key* pk, pcs_private_key*vk, hcs_random* hr, string &qt)
{
    destinationMat = rawMat.clone();
    for (int i=0;i<rawMat.rows;i++)
        for (int j=0;j<rawMat.cols;j++)
        {
            while (true)
            {
                bool break_flag = false;     
                mpz_t encryptedValue,newValue,modNumber,quotient;
                mpz_inits(encryptedValue,newValue,modNumber,quotient,NULL);
                mpz_set_ui(encryptedValue,rawMat.at<uchar>(i,j));
                mpz_set_ui(newValue,0);
                mpz_set_ui(modNumber,257);
                mpz_set_ui(quotient,0);
                pcs_encrypt(pk,hr,encryptedValue,encryptedValue);
                mpz_mod(newValue,encryptedValue,modNumber);
                mpz_t decryptedValue;
                mpz_init(decryptedValue);
                if (mpz_cmp_ui(newValue,255)<0) 
                {
                    pcs_decrypt(vk,decryptedValue,encryptedValue);
                    mpz_fdiv_q_ui(quotient,encryptedValue,257);                    
                    destinationMat.at<uchar>(i,j) = mpz_get_ui(newValue);
                    break_flag = true;
                    char * charQuotient = 0;
                    charQuotient= mpz_get_str(charQuotient,10,quotient);
                    string strQuotient = charQuotient;
                    qt += strQuotient + ";"; 
                    break_flag = true;
                } 
                mpz_clears(encryptedValue,newValue,modNumber,quotient, NULL);
                if (break_flag==true) break;
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
            pcs_ee_add(pk,result,number1,number2);
            pcs_decrypt(vk,result,result);
            unsigned long int recoveredValue =  mpz_get_ui (result);
            destinationMat.at<uchar>(i,j) = recoveredValue;
            index++;
        }
}


void serialize( Mat rawImg,Mat &imageComponent0,Mat &imageComponent1,Mat &imageComponent2,Mat &imageComponent3,Mat &imageComponent4,Mat &imageComponent5,Mat bgrImg1,Mat bgrImg2,Mat bgrImg3){
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
void encrypt(int keyLength,Mat imageComponent0,Mat imageComponent1,Mat imageComponent2,Mat imageComponent3,Mat imageComponent4,Mat imageComponent5,
    Mat &encryptImg0,Mat &encryptImg1,Mat &encryptImg2,Mat &encryptImg3,Mat &encryptImg4,Mat &encryptImg5
){
    string qt[6]; //???
    
    pcs_public_key *pK=pcs_init_public_key();
    pcs_private_key *prK=pcs_init_private_key();
    hcs_random *hr=hcs_init_random();
    pcs_generate_key_pair(pK,prK,hr,keyLength);
    for (int i=0;i<6;i++){
        qt[i]="";
    }

    cout<<"Encrypting,please wait for a while"<<endl;
    
    future<void> t1 = async(paillier_encryption,imageComponent0,ref(encryptImg0),pK,prK,hr,ref(qt[0]));        
    future<void> t2 = async(paillier_encryption,imageComponent1,ref(encryptImg1),pK,prK,hr,ref(qt[1]));
    future<void> t3 = async(paillier_encryption,imageComponent2,ref(encryptImg2),pK,prK,hr,ref(qt[2]));
    future<void> t4 = async(paillier_encryption,imageComponent3,ref(encryptImg3),pK,prK,hr,ref(qt[3])); 
    future<void> t5 = async(paillier_encryption,imageComponent4,ref(encryptImg4),pK,prK,hr,ref(qt[4]));
    future<void> t6 = async(paillier_encryption,imageComponent5,ref(encryptImg5),pK,prK,hr,ref(qt[5]));  
    cout<<"qt"<<qt[0]<<endl;
    t1.get();
    t2.get();
    t3.get();
    t4.get();
    t5.get();
    t6.get();   

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
    serialize(rawImg,imageComponent[0],imageComponent[1],imageComponent[2],imageComponent[3],imageComponent[4],imageComponent[5],bgrImg1,bgrImg2,bgrImg3);//serialize pic
    auto start = chrono::steady_clock::now();
    encrypt(keyLength,imageComponent[0],imageComponent[1],imageComponent[2],imageComponent[3],imageComponent[4],imageComponent[5],encrptImage[0],encrptImage[1],encrptImage[2],encrptImage[3],encrptImage[4],encrptImage[5]);//enrypt image
    auto end = chrono::steady_clock::now();
    cout <<"Encryption time: ";
    cout << chrono::duration <double> (end-start).count() << "s" << endl;

    imwrite("p1.png",encrptImage[0]);
    imwrite("p2.png",encrptImage[1]);
    imwrite("p3.png",encrptImage[2]);
    imwrite("p4.png",encrptImage[3]);
    imwrite("p5.png",encrptImage[4]);
    imwrite("p6.png",encrptImage[5]);


    








  

  



     


    

    
    
    
}