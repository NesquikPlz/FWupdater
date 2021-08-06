#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "openssl/aes.h"
#include "openssl/sha.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

#define TAR_FILE_CNT 3
char temp_tar_name_list[TAR_FILE_CNT][255];
char decryptedFileList[2][255] = {"boot.tar", "opt.tar"};
char hashOutput[3][65];

char hashOriginal[3][65] = {
    "7fcefaecb95c1b133abfc6b4bf454b36acc1da16eb8e544ff4a8cbe676bb6952", //hash of boot.tar
    "cf5df0bcfd05da356af23791836e75b3f88e92553b4dc4741d9b34dd30363092", //opt.tar
    "741765182ccd5b283cc3920d374faa7fc6fb800ce88728fbe55d045dd5f74ccd"  //documentation.tar
};

char key[] = "hihellonicetosee";

char inputKey[129];

int file_extract(char* path)
{
    FILE* fp;
    FILE* ftemp;
    int fsize = 0, fnameSize = 0;
    int i = 0, j = 0;
    int block_cnt = 0;
    char buf[255];
    char temp_tar_name[255];
    
    fp = fopen(path, "rb");
    printf("%s\n", path);
    for (i = 0; i < TAR_FILE_CNT; i++)
    {
        //printf("start\n");
        fread(&fnameSize, 4, 1, fp);
        memset(temp_tar_name, 0, 255);
        fread(temp_tar_name, fnameSize, 1, fp);
        printf("[*]tar name : %s \n" , temp_tar_name);
        fread(&fsize, 4, 1, fp);
        block_cnt = fsize / 255;
        ftemp = fopen(temp_tar_name, "wb");
        for (j = 0; j < block_cnt; j++)
        {
            fread(buf, 255, 1, fp);
            fwrite(buf, 255, 1, ftemp);
            memset(buf, 0, strlen(buf));
        }
        fread(buf, fsize % 255, 1, fp);
        fwrite(buf, fsize % 255, 1, ftemp);
        memset(buf, 0, strlen(buf));
        fclose(ftemp);
        strcpy(temp_tar_name_list[i], temp_tar_name);
    }
    fclose(fp);
    return 0;
}

void sha256_hash_string (unsigned char hash[SHA256_DIGEST_LENGTH], char hashOutput[65]) {
    int i = 0;
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(hashOutput + (i * 2), "%02x", hash[i]);
    }
    hashOutput[64] = 0;
}

int makeFileHash() {
    for (int i=0; i<3; i++) {
        char fileList[3][255] = {"./boot.tar", "./opt.tar", "./documentation.tar"};
        FILE *file = fopen(fileList[i], "rb");
        if(!file) return -1;

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);

        const int bufSize = 32768;
        unsigned char *buffer = (unsigned char *)malloc(bufSize);
        int bytesRead = 0;
        if(!buffer) return -2;

        while((bytesRead = fread(buffer, 1, bufSize, file)))
        {
            SHA256_Update(&sha256, buffer, bytesRead);
        }
        SHA256_Final(hash, &sha256);
        sha256_hash_string(hash, hashOutput[i]);
        fclose(file);
        free(buffer);
        
    }
    return 0;
}

int checkFileHash () {
    for (int i=0; i<3; i++) {
        if (!strcmp(hashOriginal[i], hashOutput[i])) return -1;
    }
    return 0;
}

int decrypt() {
    FILE* stream;
    char line[1024];
    char tar_fn[255] = { 0x00, };
    char cmd[1024] = { 0x00, };
    int i = 0;
    char filename[255] = {0x00, };

    char encryptedFileList[2][255] = {"boot.enc", "opt.enc"};

    for (i = 0; i < 2; i++)
    {
        
        strcpy(tar_fn, encryptedFileList[i]);
        strcpy(filename, decryptedFileList[i]);

        // if (access(tar_fn, 0) == 0) {
        //     rmdir("./boot");
        //     rmdir("./documentation");
        //     rmdir("./opt");
        // }

        sprintf(cmd, "openssl enc -d -aes-128-ctr -in %s -out %s -k %s ", tar_fn, filename, inputKey);

        stream = popen(cmd, "r");
        if (stream == NULL) {
            remove(tar_fn);
            return -1;
        }
        pclose(stream);
        remove(tar_fn);
    }

    return 1;
}

int tar_extract()
{
    FILE* stream;
    char line[1024];
    char tar_fn[255] = { 0x00, };
    char cmd[1024] = { 0x00, };
    int i = 0;

    strcpy(temp_tar_name_list[0], decryptedFileList[0]);
    strcpy(temp_tar_name_list[1], decryptedFileList[1]);
    
    for (i = 0; i < TAR_FILE_CNT; i++)
    {

        strcpy(tar_fn, temp_tar_name_list[i]);

        if (access(tar_fn, 0) == 0) {
            rmdir("./boot");
            rmdir("./documentation");
            rmdir("./opt");
            //remove("./FW");
        }

        // sprintf(cmd, "pv %s | tar xf - ", tar_fn);
        //sprintf(cmd, "tar -xf %s -C /var/update_test/", tar_fn);
        sprintf(cmd, "tar -xf %s -C /var/update_test/", tar_fn);

        stream = popen(cmd, "r");
        if (stream == NULL) {
            remove(tar_fn);
            return -1;
        }
        pclose(stream);
        remove(tar_fn);
    }

    return 1;
}


int main(int argc, char* argv[])
{
    char firmware_path[255];
    int ret = 0;

    if (argc != 3) {
        printf("usage: %s [firmware path] [key value]\n", argv[0]);
        exit(0);
    }
    if (strlen(argv[2])>17) {
        printf("**WRONG KEY LENGTH**\n");
        exit(0);
    }
    sprintf(inputKey, "%s", argv[2]);
    if (strcmp(inputKey, key)) {
        printf("**WRONG KEY VALUE**\n");
        exit(0);
    }

    sprintf(firmware_path,"%s", argv[1]);
    // Firmware extract
    printf("[Firmware Extract] \n");
    if ((ret = file_extract(firmware_path)) < 0) {
        printf("[*]File system extraction failed.(%d)", ret);
        exit(0);
    }
    printf("[*]OK\n");

    if ((ret = decrypt()) < 0) {
        printf("[*]Decrypt failed.(%d)", ret);
        exit(0);
    }
    printf("[*]File decrypted\n");

    if(ret = makeFileHash()<0) {
        printf("[*]hash failed.(%d)\n", ret);
        exit(0);
    }

    if(!checkFileHash()) {
        printf("[*]DATA MODIFIED.\n");
        printf("[*]WHAT DID YOU DO\n");
        exit(0);
    }

    printf("[*]Hash Check Completed\n");

    // Firmware Write
    printf("[Install]\n");
    tar_extract();

    printf("[Firmware Update] Success\n");
}

