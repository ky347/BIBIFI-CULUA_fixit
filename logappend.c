#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>
#include <stdbool.h>
#include "data.h"
#include <ctype.h>
#include <errno.h>
#include <jansson.h>



bool is_all_digits(char *s){
  if (s == NULL || *s == '\0'){
    return false;
  }
  
  //digit rule: 0-9
  for (int i=0; s[i] != '\0'; i++){
    if (!((s[i]>='0' && s[i]<='9'))){
      return false;
    }
  }  

  return true;
}

bool is_valid_name(char *name){
  if (name == NULL || *name == '\0'){
    return false;
  }
  
  //naming rule: a-z or A-Z
  for (int i=0; name[i] != '\0'; i++){
    if (!((name[i]>='a' && name[i]<='z')|| (name[i]>='A' && name[i]<='Z'))){
      return false;
    }

  }

  return true;
}

bool is_valid_token(char *token){
  if (token == NULL || *token == '\0'){
    return false;
  }
  //token rule: a-z or A-Z or 0-9
  for (int i =0; token[i]!= '\0'; i++){
    if (!((token[i]>='a' && token[i]<='z')||(token[i]>='A' && token[i]<='Z')||(token[i]>='0' && token[i]<='9'))) {
      return false;
    }
  }
  return true;
}

bool is_valid_logname(char *logname){
  if (logname == NULL || *logname == '\0'){
    return false;
  }
  
  //naming rule: a-z or A-Z
  for (int i=0; logname[i] != '\0'; i++){
    if (!(isalnum(logname[i])|| (logname[i] == '_') ||(logname[i] == '.'))){
      return false;
    }

  }
  return true;
}

bool is_valid_batch_name(char *batchpath){
  if (batchpath == NULL || *batchpath == '\0') return false;

  if (strchr(batchpath, '/') != NULL) return false;
  if (strchr(batchpath, '\\') != NULL) return false;
  return true;
}

void free_cmd_result(CmdLineResult *R){
  if (R->name){
    free(R->name);
    R->name = NULL;
  } 
  if (R->token){
    free(R->token);
    R->token = NULL;
  }
  if (R->logpath){
    free(R->logpath);
    R->logpath = NULL;
  }
  if (R->batchpath) {
    free(R->batchpath);
    R->batchpath = NULL;
  }

}
bool validation_check(CmdLineResult *cmdRes , Record *current_rec, int latest_time){

  //check timestamp
  if (latest_time >= cmdRes->timestamp){
    return false;
  }

  int cur_room;
  if (current_rec == NULL){
    cur_room = -2;
  } else{
    if (current_rec->actionType == Left){
      if (current_rec->room>=0){
        cur_room = -1; //left room x -> back to gallery
      } else if (current_rec->room == -1)
      {
        cur_room = -2; //left gallery -> back to outside
      } else{
        return false; 
      }
      
    } else {
      //arrived condition
      cur_room = current_rec->room;
    }
  }


  
  //user first login or current not in gallery. (only allow: arrive + galleryID)
  if(current_rec == NULL || cur_room == -2){
    if ((cmdRes->actionType == Arrived )&&(cmdRes->room == -1)){
      //validate
      return true;
    } 
  } 

  //user current in room(only allow: left + roomID)
  else if (cur_room >=0){
    if (cmdRes->actionType == Left && cmdRes->room == cur_room){
      //validated
      return true;
    } 
  }

  //user current in gallery(allow: left + galleryID/ arrive + roomID)
  else if(cur_room == -1){
    if (cmdRes->actionType == Left && cmdRes->room == -1){
      //validated
      return true;
    } else if( cmdRes->actionType == Arrived && cmdRes->room >= 0){
      //validated
      return true;
    } 
  }
  
  return false;
}

CmdLineResult parse_cmdline(int argc, char *argv[], int is_batch) {
  /*
  for (int i = 0; i < argc; i++) {
    printf(" dbg print: argv[%d] = %s (hex: %02x)\n", i, argv[i], (unsigned char)argv[i][0]);
  }
    */
  optind = 0; //TBD mac=1, linux=0
  opterr = 0; 
  optopt = 0;
  CmdLineResult R = { 0 };
  int opt,r = -1;
  int has_batch = 0;
  int has_time = 0;
  int has_token = 0;
  int has_action = 0;
  int has_name = 0;
  int has_room = 0;
  R.good = -1;
  R.room = -1; //without -R, default action: entering gallery

  //argument data
  char    *logpath = NULL;

  //pick up the switches
  while ((opt = getopt(argc, argv, "T:K:E:G:ALR:B:")) != -1) {
    int     br = 0;
    size_t  len;
    

    switch(opt) {
      case 'B':
        // 'B' in batch file or double B
        if (is_batch || has_batch) {
          goto cleanup;
        }
        
        if (!is_valid_batch_name(optarg))  goto cleanup;

        R.batchpath = strdup(optarg);
        has_batch = 1;
        R.batch_flag = 1;

	      break;  

      case 'T':
	      //timestamp 
        if(has_time)  goto cleanup;
        if (!is_all_digits(optarg))  goto cleanup;

        long t_val = strtol(optarg, NULL, 10);
        if (t_val<1 || t_val > 1073741823)  goto cleanup;

        R.timestamp = (int) t_val;
        has_time = 1;
        break;

      case 'K':
	      //secret token
        if (has_token)  goto cleanup;
        if(!is_valid_token(optarg))  goto cleanup;
        R.token = strdup(optarg);
        has_token = 1;

        break;

      case 'A':
        //arrival
        if (has_action)  goto cleanup;
        R.actionType = Arrived;
        has_action = 1;
        break; 
      
      case 'L':
        //departure
        if (has_action)  goto cleanup;
        R.actionType = Left;
        has_action = 1;
        break;

      case 'E':
        //employee name
        if (has_name)  goto cleanup;
        if (!is_valid_name(optarg))  goto cleanup;
        R.name=strdup(optarg);
        R.guestType = Employee;
        has_name = 1;
        break;

      case 'G':
        //guest name
        if (has_name)  goto cleanup;
        if (!is_valid_name(optarg))  goto cleanup;
        R.name=strdup(optarg);
        R.guestType = Guest;
        has_name = 1;
	      break;

      case 'R':
        //room ID
        if(has_room)  goto cleanup;
        if (!is_all_digits(optarg))  goto cleanup;

        long r_val = strtol(optarg, NULL, 10);
        if (r_val<0 || r_val > 1073741823)  goto cleanup;

        R.room = (int) r_val;
        has_room = 1;

        break;
      case '?':
        //printf("ignore0: [-%c] (ASCII: %d)\n", optopt, optopt); 
        goto cleanup;
        break;

      default:
        //unknown option, leave
        //printf("ignore1\n");
        goto cleanup;
        break;
    }

    if(br) {
      break;
    }
  }
  
  
  //pick up the positional argument for log path
  if(optind < argc) {
    logpath = argv[optind]; 

    //check logfile name validation
    if(!is_valid_logname(logpath)){
      goto cleanup;
    }
    
    R.logpath = strdup(argv[optind]);
    
    //if more than 1 logfile name
    optind ++;
    if (optind <argc){
      //printf("ignore2\n");
      goto cleanup;
    }
     
  } else if (!has_batch){
    //no logpath arg
     goto cleanup;
  }

  if(has_batch) {
    if(has_time || has_token || has_action || has_name || has_room|| logpath){
      goto cleanup;
    }
    R.good = 0;

  } else {
    //single command
    if(!(has_time && has_token && has_action && has_name )){
       goto cleanup;
    }

    R.good = 0;
  }

  cleanup:
    if (R.good == -1){
      free_cmd_result(&R);
    }
    return R;
}

int do_batch(char *filepath) {
  //printf("start do_batch test!\n");
  //read batch file from filepath
  char  *data = NULL;
 
  FILE *fp  = fopen(filepath, "r");

  //batch file not exist
  if (!fp){
    return -1;
  }

  size_t len = 0;

  //parse each cmd
  while(getline(&data, &len, fp) != -1){
    size_t data_len = strlen(data);
    if(data_len > 0 && data[data_len - 1] =='\n'){
      data[data_len - 1] = '\0';
    }
    if(strlen(data) == 0) continue;

    //printf("command: [%s]\n", data);

    //parsing each line to arg array
    wordexp_t p;
    if (wordexp(data, &p, WRDE_NOCMD) != 0){
      //printf("illegal format\n");  
      continue;
    }

    //shift arguments, b/c getopt drop first arg
    int temp_argc = p.we_wordc + 1;
    char **temp_argv = malloc(sizeof(char*)*(temp_argc +1));
    temp_argv[0] = "logappend";
    for (int i=0; i< p.we_wordc; i++){
      temp_argv[i+1]= p.we_wordv[i];
    }
    temp_argv[temp_argc] = NULL;

    CmdLineResult cmdRes  = parse_cmdline(temp_argc, temp_argv, 1);
    if (cmdRes.good != 0){
      printf("invalid option\n");
      free_cmd_result(&cmdRes);
      free(temp_argv);
      wordfree(&p);
      continue; 
    }

    //start parsing logfile 
    Buffer B = verify_then_decrypt(cmdRes.logpath, (unsigned char *)cmdRes.token, strlen(cmdRes.token));
    json_t *root = NULL;
    Record * current_rec = NULL;
    if(B.Length == (unsigned long)-1){
        printf("invalid token\n");
        free_cmd_result(&cmdRes);
        continue;  
    } else if (B.Length == 0) {
      //printf("logfile does not exist.\n");
      root = json_object();
    } else {
      //printf("logfile exists.\n");
      root = json_from_buf((char *)B.Buf, B.Length);
      int latest_time = get_latest_timestamp(root);
      if (latest_time == -1){
        //printf("batch:  get latest timestamp failed\n");
        free_cmd_result(&cmdRes);
        continue;     
      }
      if(!get_latest_record(root, cmdRes.name, cmdRes.guestType, &current_rec )){
        //printf("batch: get latest record failed\n");
        free_cmd_result(&cmdRes);
        continue; 
      } 
      if (!validation_check(&cmdRes, current_rec, latest_time)){
        printf("invalid check\n");
        free_cmd_result(&cmdRes);
        continue;    
      }
    }  

    //write the result back out to the file
    Record new_rec = {0};
    new_rec.timestamp = cmdRes.timestamp;
    new_rec.guestType = cmdRes.guestType;
    new_rec.room = cmdRes.room;
    new_rec.name = cmdRes.name;
    new_rec.actionType = cmdRes.actionType; 


    if(!insert_rec(root, &new_rec)){
      printf("batch: record write back failed\n");
      free_cmd_result(&cmdRes);
      continue;  
    }

    //printf("buf_from_json\n");
    const char * data = buf_from_json(root);
    if(!data){
      //printf("batch: json write back failed\n");
      free_cmd_result(&cmdRes);
      continue; 
    }

    //enc + write back
    //printf("write back\n");
    if(encrypt_then_sign(cmdRes.logpath, cmdRes.token, strlen(cmdRes.token), (unsigned char *)data, strlen(data)) != 1){
      //printf("batch: enc write back failed\n");
      free_cmd_result(&cmdRes);
      continue; 
    }


    free_cmd_result(&cmdRes);
    free(temp_argv);
    wordfree(&p);
    
  }

  free(data);
  fclose(fp);
  return 0;
}




int main(int argc, char *argv[]) {
  int r;
  CmdLineResult cmdRes = {0};
  //printf("start test!\n");
  /*
  printf("User Command: ");
    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
  */
  cmdRes = parse_cmdline(argc, argv, 0);
  
  if(cmdRes.good != 0) {
    printf("invalid\n");
    free_cmd_result(&cmdRes);
    return 255;
  }
  
  //handle batch 
  if (cmdRes.batch_flag){
    int batch_res = do_batch(cmdRes.batchpath);
    if (batch_res !=0){
      printf("invalid\n");
      free_cmd_result(&cmdRes); 
      return 255;
    }
    free_cmd_result(&cmdRes); 
    return 0;
  }


  // int fd = open(cmdRes.logpath, O_RDONLY);
  // if( fd == -1){
  //   if (errno == ENOENT){
  //     print("log file doesn't exist. creating log.\n");
  //     // create file
  //     fd = open(cmdRes.logpath, O_CREAT | O_WRONLY, 0644);

  //     const char *json_format = "{\"records\":{\"employee\":{},\"guest\":{}}}";
  //     write(fd, json_format,strlen(json_format));

  //     if(fd == -1){
  //       printf("invalid: failed to create log file.\n");
  //       free_cmd_result(&cmdRes);
  //       return 255;
  //     }
  //   } else {
  //     printf("invalid\n");
  //     free_cmd_result(&cmdRes);
  //     return 255;
  //   }
  // }
  // close(fd);
  
  //printf("finsh parsing cmd\n");

  //decrypt and get 
  Buffer B = verify_then_decrypt(cmdRes.logpath, (unsigned char *)cmdRes.token, strlen(cmdRes.token));
  json_t *root = NULL;
  Record * current_rec = NULL;
  if(B.Length == (unsigned long)-1){
    printf("invalid\n");
    free_cmd_result(&cmdRes);
    return 255;
  } else if(B.Length == 0){
    //Buffer length == 0
    //printf("logfile does not exist.\n");
    root = json_object();
  } else {
    //printf("logfile exists.\n");
    root = json_from_buf((char *)B.Buf, B.Length);
    int latest_time = get_latest_timestamp(root);
    if (latest_time == -1){
      printf("invalid\n");
      free_cmd_result(&cmdRes);
      return 255;    
    }
    if(!get_latest_record(root, cmdRes.name, cmdRes.guestType, &current_rec )){
      printf("invalid\n");
      free_cmd_result(&cmdRes);
      return 255;
    } 
    if (!validation_check(&cmdRes, current_rec, latest_time)){
      printf("invalid\n");
      free_cmd_result(&cmdRes);
      return 255;   
    }
  } 

  //write the result back out to the file
  //printf("new rec\n");
  Record new_rec = {0};
  new_rec.timestamp = cmdRes.timestamp;
  new_rec.guestType = cmdRes.guestType;
  new_rec.room = cmdRes.room;
  new_rec.name = cmdRes.name;
  new_rec.actionType = cmdRes.actionType; 

  if(!insert_rec(root, &new_rec)){
    printf("record write back failed\n");
    free_cmd_result(&cmdRes);
    return 255;  
  }

  //printf("buf_from_json\n");
  const char * data = buf_from_json(root);
  if(!data){
    printf("json write back failed\n");
    free_cmd_result(&cmdRes);
    return 255; 
  }

  //enc + write back
  if(encrypt_then_sign(cmdRes.logpath, cmdRes.token, strlen(cmdRes.token), (unsigned char *)data, strlen(data)) != 1){
    printf("enc write back failed\n");
    free_cmd_result(&cmdRes);
    return 255;  
  }
free_cmd_result(&cmdRes);
return 0; 
}

/*
========== Parse Result ("R")==========
Status (good): 0
Timestamp:  10
Token:      secret
Name:       John
GuestType:  Employee
Action:     Arrived
room ID:    0
Log Path:   log1
Batch Flag: No
==================================
*/

 