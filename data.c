#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/pkcs12.h>
#include <jansson.h>
#include "data.h"
#include <openssl/rand.h>

#define NON_VAR_LENGTH 0     //TODO change me




void record_names_cleanup (Record* recList, size_t size){
    for(int i = 0;i < size; i++){
        free(recList[i].name);
    }
}
bool get_records(json_t* root, char* name, GuestType guest, Record ** recordList, size_t* recSize){
    if(root == NULL) return false;
    if (!json_is_object(json_object_get(root, "records"))) return false;
    root = json_object_get(root, "records");

    if (guest == Employee){
        if (!json_is_object(json_object_get(root, "employee"))) return false;
        root = json_object_get(root, "employee");
    }else {
        if (!json_is_object(json_object_get(root, "guest"))) return false;
        root = json_object_get(root, "guest");
    }
    if (!json_is_array(json_object_get(root, name))) return false;
    root = json_object_get(root, name);
    *recSize = json_array_size(root);
    if(*recSize == 0) return false;
    Record* recList = *recordList;
    recList = calloc(sizeof(Record), *recSize);
    if(recList == NULL) return false;
    *recordList = recList;
    size_t index;
    json_t *recJSON;
    json_array_foreach(root, index, recJSON) {
        Record* r = &recList[index];

        //timestamp
        json_t* temp = json_object_get(recJSON,"timestamp");
        if(!json_is_integer(temp)){
            record_names_cleanup(recList, index);
            free(recList);
            return false;
        } 
        r->timestamp = (long)json_integer_value(temp);
        //guestType
        temp = json_object_get(recJSON,"guestType");
        if(!json_is_integer(temp)){
            record_names_cleanup(recList, index);
            free(recList);
            return false;
        }
        int guestType = (int)json_integer_value(temp); 
        if(guestType == 0){
            r->guestType = Employee;
        }else{
            r->guestType = Guest;
        }
        //actionType
        temp = json_object_get(recJSON,"actionType");
        if(!json_is_integer(temp)){
            record_names_cleanup(recList, index);
            free(recList);
            return false;
        }
        int actionType = (int)json_integer_value(temp); 
        if(actionType == 0){
            r->actionType = Arrived;
        }else{
            r->actionType = Left;
        }
        //name
        temp = json_object_get(recJSON,"name");
        if(!json_is_string(temp)){
            record_names_cleanup(recList, index);
            free(recList);
            return false;
        }
        int nameLen = json_string_length(temp);
        r->name = calloc(sizeof(char), nameLen+1);
        if(r->name == NULL){
            record_names_cleanup(recList, index);
            free(recList);
            return false;
        }
        memcpy(r->name, json_string_value(temp), nameLen);
        //room
        temp = json_object_get(recJSON,"room");
        if(!json_is_integer(temp)){
            record_names_cleanup(recList, index+1);
            free(recList);
            return false;
        } 
        r->room = (long)json_integer_value(temp);
    }
    return true;
}
bool get_all_type_records(json_t* root, GuestType guest, Record ** recordList, size_t* recSize){
    if(root == NULL) return false;
    if (!json_is_object(json_object_get(root, "records"))) return false;
    root = json_object_get(root, "records");

    if (guest == Employee){
        if (!json_is_object(json_object_get(root, "employee"))) return false;
        root = json_object_get(root, "employee");
    }else {
        if (!json_is_object(json_object_get(root, "guest"))) return false;
        root = json_object_get(root, "guest");
    }

    const char *key;
    json_t *value;
    void * iter = json_object_iter(root);
    if(iter == NULL){
        return false;
    }


    Record* recList = *recordList;
    recList = malloc(1);
    if(recList == NULL) return false;
    *recSize = 0;

    while(iter){

        value = json_object_iter_value(iter);
        if (!json_is_array(value)) continue;
        int arraySize = json_array_size(value);
        if(arraySize == 0) return false;

        Record* tempRecList = realloc(recList, sizeof(Record)*(*recSize+arraySize));
        if(tempRecList == NULL) {
            free(recList);
            return false;
        }
        recList = tempRecList+(*recSize);
        *recSize+=arraySize;
        size_t index;
        json_t *recJSON;
        json_array_foreach(value, index, recJSON) {
            Record* r = &recList[index]; 
            //timestamp
            json_t* temp = json_object_get(recJSON,"timestamp");
            if(!json_is_integer(temp)){
                record_names_cleanup(recList, index);
                free(recList);
                return false;
            } 
            r->timestamp = (long)json_integer_value(temp);
            //guestType
            temp = json_object_get(recJSON,"guestType");
            if(!json_is_integer(temp)){
                record_names_cleanup(recList, index);
                free(recList);
                return false;
            }
            int guestType = (int)json_integer_value(temp); 
            if(guestType == 0){
                r->guestType = Employee;
            }else{
                r->guestType = Guest;
            }
            //actionType
            temp = json_object_get(recJSON,"actionType");
            if(!json_is_integer(temp)){
                record_names_cleanup(recList, index);
                free(recList);
                return false;
            }
            int actionType = (int)json_integer_value(temp); 
            if(actionType == 0){
                r->actionType = Arrived;
            }else{
                r->actionType = Left;
            }
            //name
            temp = json_object_get(recJSON,"name");
            if(!json_is_string(temp)){
                record_names_cleanup(recList, index);
                free(recList);
                return false;
            }
            int nameLen = json_string_length(temp);
            r->name = calloc(sizeof(char), nameLen+1);
            if(r->name == NULL){
                record_names_cleanup(recList, index);
                free(recList);
                return false;
            }
            memcpy(r->name, json_string_value(temp), nameLen);
            //room
            temp = json_object_get(recJSON,"room");
            if(!json_is_integer(temp)){
                record_names_cleanup(recList, index+1);
                free(recList);
                return false;
            } 
            r->room = (long)json_integer_value(temp);
        }
        
        recList = tempRecList;
        *recordList = recList;
        iter = json_object_iter_next(root, iter);
    }
        
    return true;
}
bool get_all_records(json_t* root, Record ** recordList, size_t* recSize){
    Record *employeeList;
    size_t employeeSize;
    if(!get_all_type_records(root, Employee, &employeeList, &employeeSize)){
        employeeSize = 0;
        employeeList = malloc(1);
        if(employeeList == NULL) return false;
    }
    Record *guestList;
    size_t guestSize;
    if(!get_all_type_records(root, Guest, &guestList, &guestSize)){
        if(employeeSize == 0){
            free(employeeList);
            return false;
        }else{
            *recordList = employeeList;
            *recSize = employeeSize;
            return true;
        }
    }

    Record* recList = realloc(employeeList, sizeof(Record)*(employeeSize + guestSize));

    if(recList == NULL){
        free(employeeList);
        free(guestList);
        return false;
    }
    memcpy(recList + employeeSize, guestList, sizeof(Record)*guestSize);
    *recordList = recList;
    *recSize = employeeSize + guestSize; 
    return true;
}
int get_latest_timestamp(json_t* root){
    if(root == NULL) return false;
    if (!json_is_integer(json_object_get(root, "timestamp"))) return -1;
    return (int)json_integer_value(json_object_get(root, "timestamp"));
}
bool get_latest_record(json_t* root, char* name, GuestType guest, Record ** record){
    if(root == NULL) return false;
    if (!json_is_object(json_object_get(root, "records"))) return false;
    root = json_object_get(root, "records");

    if (guest == Employee){
        if (!json_is_object(json_object_get(root, "employee"))) return false;
        root = json_object_get(root, "employee");
    }else {
        if (!json_is_object(json_object_get(root, "guest"))) return false;
        root = json_object_get(root, "guest");
    }
    json_t * userArray = json_object_get(root, name);
    if (userArray == NULL){
        *record = NULL;
        return true;
    }
    if (!json_is_array(userArray)) return false;
    root = json_object_get(root, name);
    *record = calloc(sizeof(Record), 1);

    size_t size = json_array_size(root);
    if (size == 0) return false;
    json_t *recJSON = json_array_get(root, size - 1);
    Record* r = *record;
    //timestamp
    json_t* temp = json_object_get(recJSON,"timestamp");
    if(!json_is_integer(temp)){
        
        return false;
    } 
    r->timestamp = (long)json_integer_value(temp);
    //guestType
    temp = json_object_get(recJSON,"guestType");
    if(!json_is_integer(temp)){
        
        return false;
    }
    int guestType = (int)json_integer_value(temp); 
    if(guestType == 0){
        r->guestType = Employee;
    }else{
        r->guestType = Guest;
    }
    //actionType
    temp = json_object_get(recJSON,"actionType");
    if(!json_is_integer(temp)){
        
        return false;
    }
    int actionType = (int)json_integer_value(temp); 
    if(actionType == 0){
        r->actionType = Arrived;
    }else{
        r->actionType = Left;
    }
    //name
    temp = json_object_get(recJSON,"name");
    if(!json_is_string(temp)){
        
        return false;
    }
    int nameLen = json_string_length(temp);
    r->name = calloc(sizeof(char), nameLen+1);
    if(r->name == NULL){
        
        return false;
    }
    memcpy(r->name, json_string_value(temp), nameLen);
    //room
    temp = json_object_get(recJSON,"room");
    if(!json_is_integer(temp)){
        
        return false;
    } 
    r->room = (long)json_integer_value(temp);
    return true;
}
bool insert_rec(json_t *root, Record* r){
    if(root == NULL) return false;
    if (!json_is_integer(json_object_get(root, "timestamp"))
      && json_object_set_new(root, "timestamp", json_integer(r->timestamp)) == -1) {
      return false;
    }

    if (!json_is_object(json_object_get(root, "records")) 
        && json_object_set_new(root, "records", json_object()) == -1){
        return false;
    }
    

    root = json_object_get(root, "records");
    if (!json_is_object(json_object_get(root, "employee")) 
        && json_object_set_new(root, "employee", json_object()) == -1){
        return false;
    }
    if (!json_is_object(json_object_get(root, "guest")) 
        && json_object_set_new(root, "guest", json_object()) == -1){
        return false;
    }
    if (r->guestType == Employee){
        root = json_object_get(root, "employee");
    }else {
        root = json_object_get(root, "guest");
    }
    const char *name = r->name;
    if (!json_is_array(json_object_get(root, name) )
        && json_object_set_new(root, name, json_array()) == -1){
        return false;
    }
    root = json_object_get(root, name);
    json_t* recJSON = json_object();
    json_object_set_new(recJSON, "timestamp", json_integer(r->timestamp));
    if(r->guestType == Employee){
        json_object_set_new(recJSON, "guestType", json_integer(0));
    }else{
        json_object_set_new(recJSON, "guestType", json_integer(1));
    }
    if(r->actionType == Arrived){
        json_object_set_new(recJSON, "actionType", json_integer(0));
    }else{
        json_object_set_new(recJSON, "actionType", json_integer(1));
    }
    json_object_set_new(recJSON, "room", json_integer(r->room));
    json_object_set_new(recJSON, "name", json_string(r->name));

    json_array_append_new(root, recJSON);
    return true;}

json_t *json_from_buf(char* buffer, size_t bufLen){
    json_error_t error;
    return json_loadb(buffer, bufLen, 0, &error);
}   
const char * buf_from_json(json_t *root){
    char * buf = json_dumps(root, 0);
    json_decref(root);
    return buf;
}




int compute_record_size(Record *R) {
  int len = 0;
  //TODO do stuff
  
  return len;
}

//serialize (struct -> json)
Buffer print_record(Record *R) {
  Buffer  B = {0};
 
  //TODO Code this
  

  return B;
}

//produce A | B
Buffer concat_buffs(Buffer *A, Buffer *B) {
  Buffer  C = {0}; 
  //TODO Code this
  return C;
}

bool write_to_path (char *path, SecInfo * s){
    int file = open(path, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR | S_IWUSR); 
    if(file == -1){
        return false;
    }
    if(write(file, s->iv, 16) != 16){
        close(file);
        return false;
    }
    if(write(file, s->salt, 16) != 16){
        close(file);
        return false;
    }
    if(write(file, s->signature, 32) != 32){
        close(file);
        return false;
    }
    if(write(file, s->record, s->bufSize) != s->bufSize){
        close(file);
        return false;
    }
    return true;}


SecInfo * read_from_path (char * path){
    int file = open(path, O_RDONLY, 0);
    //printf("after open\n");
    if(file == -1){
        return NULL;
    }
    SecInfo* s = malloc(sizeof(SecInfo));
    if(s == NULL){
        return NULL;
    }
    if(read(file, s->iv, 16) != 16){
        free(s);
        close(file);
        return NULL;
    }
    if(read(file, s->salt, 16) != 16){
        free(s);
        close(file);
        return NULL;
    }
    if(read(file, s->signature, 32) != 32){
        free(s);
        close(file);
        return NULL;
    }
    int bufSize = 64;
    char * jsonBuffer = malloc(bufSize);
    if(jsonBuffer == NULL) {
        free(s);
        close(file);
        return NULL;
    }
    int amountRead = 0;
    //printf("before while\n");
    while (true){
        if (amountRead + 64 > bufSize) {
            bufSize *= 2;
            char *reallocBuffer = realloc(jsonBuffer, bufSize);
            if (reallocBuffer == NULL) {
                free(jsonBuffer); 
                free(s); 
                close(file); 
                return NULL;
            }
            jsonBuffer = reallocBuffer; 
        }
        int currRead = read(file, jsonBuffer + amountRead, 64);

        if (currRead <= 0) break; 

        amountRead += currRead;
    }
    //printf("after while\n");
    close(file);
    s->bufSize = amountRead;
    s->record = jsonBuffer;
    return s;}






int read_records_from_path(char *path, unsigned char *key, Record **outbuf, unsigned int *outnum) {
  *outnum = 0;
  *outbuf = NULL;

  //read in a file 
  //Buffer  B = read_from_path(path, key);

  //TODO Code this
  
  return 0;
}



//SIGN AND VERIFY FUNCTIONS:

//sign
//note: caller is responsible for freeing returned string
//returns mac in hex to make it easier to append to log
char *sign(unsigned char *data, unsigned int data_len,
           unsigned char *token, unsigned int token_len) {
  
  unsigned char key[32];
  unsigned int key_len = 32;
  //need to generate 32 length hmac key (token is not constant length)
  PKCS5_PBKDF2_HMAC((const char *)token, token_len, NULL, 0, 10000, EVP_sha256(), key_len, key);
  
  unsigned char mac[32];
  unsigned int mac_len;

  if (!HMAC(EVP_sha256(), key, key_len, data, data_len, mac, &mac_len)) {
      return NULL; //error in calculation
  }
  /*
  //allocate space for hex string (2 chars per byte + null terminator)
  char *hex_mac = malloc(mac_len * 2 + 1);
  if (!hex_mac) return NULL;

  //convert binary to hex
  for (unsigned int i = 0; i < mac_len; i++) {
      sprintf(&hex_mac[i * 2], "%02x", mac[i]);
  }
  hex_mac[mac_len * 2] = '\0';

  return hex_mac;
  */

  //return binary hmac
  char * binary_mac = malloc(32);
  if (! binary_mac) return NULL;

  memcpy(binary_mac, mac, 32);
  return binary_mac;
}

//verify
//returns 0 if macs are not equal, 1 if macs are equal
int verify(unsigned char *data, unsigned int data_len,
           unsigned char *token, unsigned int token_len,
           unsigned char *mac) {

  //generate new hmac
  char *calculated_mac = sign(data, data_len, token, token_len);
  if (calculated_mac == NULL){
    return 0; //gen hmac failed
  }
  
  //compare calculated_mac & mac
  /*
  if (calculated_mac == NULL && strcmp(calculated_mac, (const char *)mac) != 0) {
    free(calculated_mac);
    return 0; //macs are not equal
  }
  */
  if (memcmp(calculated_mac, (const char *)mac, 32) != 0) {
    free(calculated_mac);
    return 0; //macs are not equal
  }

  free(calculated_mac);
  return 1; //macs are equal
}

SecInfo *enc(const char *token, Buffer *json_data) {
    //printf("start enc\n");
    SecInfo *si = malloc(sizeof(SecInfo));
    if (!si) return NULL;

    unsigned char key[32];
    unsigned char *plaintext = (unsigned char *)json_data->Buf;
    unsigned long plaintext_len = json_data->Length;


    //gen IV + Key(salt+ token)
    RAND_bytes((unsigned char *)si->salt, 16);
    RAND_bytes((unsigned char *)si->iv, 16);
    if (!PKCS5_PBKDF2_HMAC(token,strlen(token),(unsigned char *)si->salt, 16, 10000, EVP_sha256(), 32, key )){
        fprintf(stderr, "enc error: PBKDF2\n"); 
        free(si);
        return NULL;
    }


    //encrypt
    unsigned char *ciphertext = malloc(plaintext_len);
    int len = 0;
    int ciphertext_len = 0;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx || !ciphertext ){
        fprintf(stderr, "enc error: ctx_new\n"); 
        free(ciphertext);
        free(si);
        return NULL;
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key, (unsigned char *)si->iv ) != 1){
        fprintf(stderr, "enc error: EVP_ini\n"); 
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext);
        free(si);
        return NULL;
    }


    if (EVP_EncryptUpdate(ctx,ciphertext,&len, plaintext, plaintext_len ) != 1){
        fprintf(stderr, "enc error: EVP_update\n"); 
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext);
        free(si);
        return NULL;
    }

    ciphertext_len = len;
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1){
        fprintf(stderr, "enc error: EVP_final\n"); 
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext);
        free(si);
        return NULL;
    }
    ciphertext_len += len;

    // store back
    si->bufSize = (unsigned int)json_data->Length;
    si->record = (char *)ciphertext;
    EVP_CIPHER_CTX_free(ctx);

    return si;
}

Buffer dec(const char *token, SecInfo *si) {
    //printf("start dec\n");
    unsigned char key[32];
    Buffer result = {NULL, 0};
    

    //gen key
    if (!PKCS5_PBKDF2_HMAC((const char *)token,strlen(token), (unsigned char*)si->salt, 16, 10000, EVP_sha256(), 32, key )){
        fprintf(stderr, "error: PBKDF2\n"); 
        return result;
    }

    //decrypt c1
    unsigned char *plaintext = malloc(si->bufSize);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx){
        free(plaintext);
        fprintf(stderr, "dec error: ctx_new\n"); 
        return result;
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key, (unsigned char*)si->iv) != 1){
        EVP_CIPHER_CTX_free(ctx);
        fprintf(stderr, "enc error: EVP_ini\n"); 
        free(plaintext);
        return result;
    }

    int len = 0;
    int plaintext_len = 0;

    if (EVP_DecryptUpdate(ctx, plaintext, &len, (unsigned char*)si->record, si->bufSize) != 1) { 
        EVP_CIPHER_CTX_free(ctx);
        fprintf(stderr, "dec error: EVP_update\n"); 
        free(plaintext);
        return result;
    }
    plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        fprintf(stderr, "dec error: EVP_final\n");
        free(plaintext);
        return result;        
    }
    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    //printf("Decrypted content: %.*s\n", 10, plaintext);
    result.Buf = plaintext;
    result.Length = plaintext_len;

    return result; 

}

bool encrypt_then_sign(char *path, char *token, unsigned int token_len, unsigned char *data, unsigned long data_len) {
    //printf("start encrypt_then_sign\n");

    //encrypt the plaintext log file
    Buffer plaintext; 
    plaintext.Buf = data;
    plaintext.Length = data_len;
    SecInfo *encrypted_si = enc(token, &plaintext);
    if (!encrypted_si){return false;}


    //sign the Encrypted Data + IV + Salt
    size_t data_to_sign_len = 16 + 16 + encrypted_si->bufSize;
    char *sign_buffer = malloc(data_to_sign_len);
    if(!sign_buffer){
        free(encrypted_si->record);
        free(encrypted_si);
        return false;
    }

    memcpy(sign_buffer, encrypted_si->iv, 16);
    memcpy(sign_buffer + 16, encrypted_si->salt, 16);
    memcpy(sign_buffer + 32, encrypted_si->record, encrypted_si->bufSize);

    //sign
    char *mac = sign((unsigned char *)sign_buffer, data_to_sign_len, (unsigned char *)token, token_len);

    //copy mac into the signature field
    memcpy(encrypted_si->signature, mac, 32);


    //write the encrypted and signed log back to path
    bool success = write_to_path(path, encrypted_si);

    //cleanup
    free(sign_buffer);
    free(encrypted_si->record);
    free(encrypted_si);

    return success;
}

Buffer verify_then_decrypt(char *path, unsigned char *token, unsigned int token_len) {
    //printf("start verify_then_decrypt: %s\n", path);
    //printf("Path: %s, Token: %s\n", path, token);
    Buffer result = {NULL, 0};

    //load data from log file
    SecInfo *si = read_from_path(path);
    if(si == NULL){
        return result;
    }
    //printf("read_from_path. bufSize: %d\n", si->bufSize);

    //extract parameter 
    unsigned int data_len = 16 + 16 + si->bufSize;
    unsigned char *data = malloc(data_len);

    memcpy(data, si->iv, 16);
    memcpy(data + 16, si->salt, 16);
    memcpy(data + 32, si->record, si->bufSize);

    //verify
    int verify_result = verify(data, data_len, token, token_len, (unsigned char *)si->signature);
    //printf("verify_result: %d \n", verify_result);

    //if verify returns 0, the macs are not equal
    if (verify_result == 0) {
        free(data);
        free(si->record);
        free(si);
        result.Length= (unsigned long)-1;
        //printf("Integrity violation %lu\n",result.Length);
        return result;
    }
    
    //decrypt
    result = dec((const char *)token, si);
    //printf("dec finished. Plaintext Length: %d\n", result.Length);

    //cleanup
    free(data);
    free(si->record);
    free(si);

    return result;
}
