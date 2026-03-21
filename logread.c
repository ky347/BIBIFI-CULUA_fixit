#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <jansson.h>

#include "data.h"

int verbose = 0;

//for debugging
void print_records(Record *records, unsigned int num_records) {
  unsigned int i;

  //printf("Called print_records\n");
  // for(i = 0; i < num_records; i++) {
  //   dump_record(&records[i]);
  //   printf("----------------\n");
  // }

  return;
}

struct Person {
  char *name;
  int current_room;
  GuestType guestType;
  SLIST_ENTRY(Person)   link;
};

SLIST_HEAD(slisthead, Person);

void execute_records(Record *Rarr, unsigned int num_records, struct slisthead *head) {
  //TODO Code this
  //printf("Called execute_records\n");
  //iterates through the entire array of records and makes a singly linked list of Persons
  for (int i = 0; i < num_records; i++) {
    if (!Rarr[i].name) {
      printf("Invaid name, stopping execution\n");
      return;
    }
    struct Person *temp;
    struct Person *found_person = NULL;
    SLIST_FOREACH(temp, head, link) {
      if (strcmp(Rarr[i].name, temp->name) == 0 && Rarr[i].guestType == temp->guestType) {
        //we've found a matching name that already exists in the linked list
        found_person = temp;
        break;
      }
    }

    if (found_person == NULL && Rarr[i].actionType == Arrived) {
      //this is a person that has not yet been added to the linked list
      struct Person *new_person = malloc(sizeof(struct Person));
      new_person->name = Rarr[i].name;
      new_person->current_room = Rarr[i].room;
      new_person->guestType = Rarr[i].guestType;
      //SLIST_INSERT_HEAD(head, new_person, link);

      //insert the person alphabetically into the list
      if (SLIST_EMPTY(head) || strcmp(new_person->name, SLIST_FIRST(head)->name) < 0) {
          SLIST_INSERT_HEAD(head, new_person, link);
      }
      else {
          struct Person *current;
          SLIST_FOREACH(current, head, link) {
              struct Person *next = SLIST_NEXT(current, link);
              if (next == NULL || strcmp(new_person->name, next->name) < 0) {
                  SLIST_INSERT_AFTER(current, new_person, link);
                  break;
              }
          }
      }
    }
    else if (found_person != NULL &&
      Rarr[i].actionType == Arrived) {
        //this is a person that has already been added to linked list and is entering a new room
        found_person->current_room = Rarr[i].room;
      }
    else if (found_person != NULL &&
      Rarr[i].actionType == Left) {
        //this is a person that has already been added to linked list and is leaving a room
        if (Rarr[i].room == -1) {
          //person is leaving the gallery, remove them from linked list
          SLIST_REMOVE(head, found_person, Person, link);
          free(found_person->name);
          free(found_person);
        }
        else {
          //person is leaving current room but staying in the gallery
          found_person->current_room = -1;
        }
      }
  }

  return;
}

int print_time(Record *Rarr, unsigned int num_records, char *name, GuestType gt) {
  unsigned long total_time = 0;
  unsigned long last_arrival = 0;
  unsigned long current_time = 0;
  int inside = 0;
  int found = 0;

  //iterate through Record array
  for (unsigned int i = 0; i < num_records; i++) {
    //check if the record belongs to the person we are looking for
    if (strcmp(Rarr[i].name, name) == 0 && Rarr[i].room == -1 && Rarr[i].guestType == gt) {
      found = 1;
      if (Rarr[i].actionType == Arrived) {
        last_arrival = (unsigned long)Rarr[i].timestamp;
        inside = 1;
      }
      else if (Rarr[i].actionType == Left && inside) {
        total_time += ((unsigned long)Rarr[i].timestamp - last_arrival);
        inside = 0;
      }
    }
    current_time = (unsigned long)Rarr[i].timestamp;
  }

  if (inside) {
    total_time += current_time - last_arrival;
  }

  if (found) {
    printf("%lu", total_time);
  }

  return 0;
}

int print_rooms(Record *Rarr, unsigned int num_records, char *name, GuestType gt) {
  int           first = 1;
  int exists = 0;
  //printf("Called print_rooms\n");
  for (int i = 0; i < num_records; i++) {
    if(strcmp(Rarr[i].name, name) == 0) {
      exists = 1;
    }
    if (strcmp(Rarr[i].name, name) == 0 && 
      Rarr[i].actionType == Arrived && 
      Rarr[i].room != -1 &&
      Rarr[i].guestType == gt) {
      
      //print a comma before every element except the first one
      if (!first) {
        printf(",");
      }
      
      printf("%d", Rarr[i].room);
      first = 0;
    }
  }
  if (exists == 0){
    printf("invalid\n");
    return 255;
  }
  return 0;
}

void print_summary(Record *Rarr, unsigned int num_records) {
  struct slisthead head = SLIST_HEAD_INITIALIZER(head);
  struct Person *temp;

  execute_records(Rarr, num_records, &head);

  //printf("Called print_summary\n");
  //comma separated list of employees currently in the gallery
  //comma separated list of guests currently in the gallery
  //print out all the rooms that currently have people in them
  //sam is an employee, gerda is a guest
  //SAM
  //GERDA
  //5: GERDA
  //30: SAM

  //print comma separated list of employees currently in the gallery
  int first_employee = 1;
  int first_guest = 1;
  SLIST_FOREACH(temp, &head, link) {
    if (temp->guestType == Employee) {
      //print a comma before every element except the first one
      if (!first_employee) {
        printf(",");
      }
      
      printf("%s", temp->name);
      first_employee = 0;
    }
  }

  printf("\n");

  //print comma separated list of guests currently in the gallery
  SLIST_FOREACH(temp, &head, link) {
    if (temp->guestType == Guest) {
      //print a comma before every element except the first one
      if (!first_guest) {
        printf(",");
      }
      
      printf("%s", temp->name);
      first_guest = 0;
    }
  }

  printf("\n");

  //print out room occupancy
  //max possible number of occupied rooms is length of records array
  int *occupied_rooms = malloc(num_records * sizeof(int));
  int num_rooms = 0;

  SLIST_FOREACH(temp, &head, link) {
    if (temp->current_room != -1) {
      //if current room is an actual room and not just the gallery
      int already_added = 0;
      for (int i = 0; i < num_rooms; i++) {
        if (occupied_rooms[i] == temp->current_room) {
          already_added = 1;
          break;
        }
      }
      if (!already_added) {
        //if current room is not already added, add it
        occupied_rooms[num_rooms++] = temp->current_room;
      }
    }
  }

  //sort the room ids numerically (bubble sort)
  for (int i = 0; i < num_rooms - 1; i++) {
    for (int j = 0; j < num_rooms - i - 1; j++) {
      if (occupied_rooms[j] > occupied_rooms[j+1]) {
        int swap = occupied_rooms[j];
        occupied_rooms[j] = occupied_rooms[j+1];
        occupied_rooms[j+1] = swap;
      }
    }
  }

  //print based on sorted room ids
  for (int i = 0; i < num_rooms; i++) {
    printf("%d: ", occupied_rooms[i]);

    int first_person = 1;
    SLIST_FOREACH(temp, &head, link) {
      if (temp->current_room == occupied_rooms[i]) {
        if (!first_person) {
          printf(",");
        }
        printf("%s", temp->name);
        first_person = 0;
      }
    }
    printf("\n");
  }

  free(occupied_rooms);

  return;
}

void cleanup(Record *records, unsigned int num_records) {
  // if (records == NULL) {
  //   return;
  // }

  // for (unsigned int i = 0; i < num_records; i++) {
  //   if (records[i].name != NULL) {
  //     free(records[i].name);
  //   }
  // }

  // free(records);
}

int main(int argc, char *argv[]) {
  int opt;
  char *logpath = NULL;
  char *token = NULL;
  char *name = NULL;
  char mode = 0;
  GuestType gt = Guest;

  
  //TODO Code this

  //collect data from commandline
  while ((opt = getopt(argc, argv, "K:SRE:G:TI")) != -1) {
    switch(opt) {
      case 'K':
        //token should not be previously assigned
        if (token != NULL) {
          printf("invalid\n");
          return 255;
        }
        token = optarg;
        break;

      case 'S':
        //only one of -S, -R, -T, -I may be specified at once, otherwise invalid
        if (mode != 0) { 
            printf("invalid\n");
            return 255; 
        }
        mode = opt;
        break;

      case 'R':
        if (mode != 0) { 
            printf("invalid\n");
            return 255; 
        }
        mode = opt;
        break;

      case 'T':
        if (mode != 0) { 
            printf("invalid\n");
            return 255; 
        }
        mode = opt;
        break;

      case 'I':
        if (mode != 0) { 
          printf("invalid\n");
          return 255; 
        }
        mode = opt;
        break;

      case 'E':
        gt = Employee;
        if (mode == 'S') {
          printf("invalid\n");
          return 255;
        }
        if (name != NULL) { 
          printf("invalid\n");
          return 255; 
        }
        name = optarg;
        break;

      case 'G':
        if (name != NULL) { 
          printf("invalid\n");
          return 255; 
        }
        name = optarg;
        break;

      default:
        printf("invalid\n");
        return 255;
    }
  }

  //get the name of the logfile
  if(optind < argc) {
    logpath = argv[optind];
  }
  //check for additional, illegal arguments
  if(optind + 1 < argc) {
    printf("invalid\n");
    return 255;
  }

  //validate data we collected from commandline
  if (!token || !logpath) {
    printf("invalid\n");
    return 255;
  }

  //read data from logfile
  Buffer data = {NULL, 0};
  data = verify_then_decrypt(logpath, token, strlen(token));
  if(data.Length == -1){
    printf("integrity violation\n");
    return 255;
  }
  // printf("data.Length: %d\n", data.Length);
  // printf("data.Buf: %s\n", data.Buf);
  json_t *json = NULL;
  json = json_from_buf(data.Buf, data.Length);
  if(json == NULL){
    printf("invalid\n");
    return 255;
  }

  Record *records = NULL;
  size_t num_records;
  if(!get_all_records(json, &records, &num_records)){
    printf("Could not Parse JSON\n");
    return 255;
   }
  // for (int i = 0; i < num_records; i++){
  //   printf("index: %d\n", i);
  //   Record* r = &records[i];
  //   printf("timestamp: %ld\n", r->timestamp);
  //   printf("guestType: %u\n", r->guestType);
  //   printf("actionType: %u\n", r->actionType);
  //   printf("room: %d\n", r->room);
  //   printf("name: %s\n", r->name); 
  //   printf("end\n");
  // }
  //printf("Number of records is %zu\n", num_records);

  //START OF TEST DATA
  //size_t num_records = 8;
  //Record *records = malloc(sizeof(Record) * num_records);

  // 1. Alice (Employee) enters the Gallery
  //records[0] = (Record){.timestamp = 100, .name = strdup("Alice"), .actionType = Arrived, .room = -1, .guestType = Employee};

  // 2. Bob (Guest) enters the Gallery
  //records[1] = (Record){.timestamp = 110, .name = strdup("Bob"), .actionType = Arrived, .room = -1, .guestType = Guest};

  // 3. Alice (Employee) enters Room 5
  //records[2] = (Record){.timestamp = 120, .name = strdup("Alice"), .actionType = Arrived, .room = 5, .guestType = Employee};

  // 4. Charlie (Employee) enters the Gallery
  //records[3] = (Record){.timestamp = 130, .name = strdup("Charlie"), .actionType = Arrived, .room = -1, .guestType = Employee};

  // 5. Bob (Guest) enters Room 10
  //records[4] = (Record){.timestamp = 140, .name = strdup("Bob"), .actionType = Arrived, .room = 10, .guestType = Guest};

  // 6. Alice (Employee) leaves Room 5 (back to Gallery)
  //records[5] = (Record){.timestamp = 150, .name = strdup("Alice"), .actionType = Left, .room = 5, .guestType = Employee};

  //Mark enters the gallery
  //records[6] = (Record){.timestamp = 160, .name = strdup("Mark"), .actionType = Arrived, .room = -1, .guestType = Guest};

  //Mark enters room 5
  //records[7] = (Record){.timestamp = 170, .name = strdup("Mark"), .actionType = Arrived, .room = 5, .guestType = Guest};
  //END OF TEST DATA

  //call appropriate function based on commandline data
  switch(mode) {
    case 'S':
      print_summary(records, num_records);
      break;

    case 'R':
      if (name == NULL) {
        printf("invalid\n");
        cleanup(records, num_records);
        return 255;
      }
      if(print_rooms(records, num_records, name, gt) == 255){
        return 255;
      }
      break;

    case 'T':
      if (name == NULL) {
        printf("invalid\n");
        cleanup(records, num_records);
        return 255;
      }
      print_time(records, num_records, name, gt);
      break;

    case 'I':
      printf("unimplemented\n");
      break;

    default:
      printf("invalid\n");
      cleanup(records, num_records);
      return 255;
  }

  cleanup(records, num_records);
  return 0;
}
