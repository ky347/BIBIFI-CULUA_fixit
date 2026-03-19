#ifndef _BUFFR_H
#define _BUFFR_H
#include <jansson.h>
#include <stdbool.h>

typedef struct _Buffer {
  unsigned char *Buf;
  unsigned long Length;
} Buffer;

typedef enum _GuestType {
  Employee,
  Guest
} GuestType;

typedef enum _ActionType {
  Arrived,
  Left
} ActionType;

typedef struct _Record {
  //put some things here
  long timestamp;
  GuestType guestType; 
  ActionType actionType;
  int room;
  char * name; 
} Record;

typedef struct _CmdLineResult {
  int timestamp;
  char  * token;
  char * name;
  GuestType guestType; 
  int room;
  char * logpath;
  ActionType actionType;
  int batch_flag;
  char * batchpath;
  int     good;
} CmdLineResult;

typedef struct _SecInfo {
  unsigned int bufSize;  //TBD modify char to unsinged int
  char iv[16];
  char salt[16];
  char signature[32];
  char* record;

} SecInfo;

void record_names_cleanup (Record* recList, size_t size);

bool get_records(json_t* root, char* name, GuestType guest, Record ** recordList, size_t* recSize);

int get_latest_timestamp(json_t* root);
bool get_all_type_records(json_t* root, GuestType guest, Record ** recordList, size_t* recSize);

bool get_all_records(json_t* root, Record ** recordList, size_t* recSize);

bool get_latest_record(json_t* root, char* name, GuestType guest, Record ** record);
bool insert_rec(json_t *root, Record* r);

json_t *json_from_buf(char* buffer, size_t bufLen);

const char * buf_from_json(json_t *root);

SecInfo * read_from_path(char *path);
bool write_to_path(char *path, SecInfo * s);
Buffer concat_buffs(Buffer *A, Buffer *B);
Buffer print_record(Record *R);
void dump_record(Record *R);

int read_records_from_path(char *path, unsigned char *key, Record **, unsigned int *);
Buffer verify_then_decrypt(char *path, unsigned char *token, unsigned int token_len);
bool encrypt_then_sign(char *path, char *token, unsigned int token_len, unsigned char *data, unsigned long data_len);

#endif
