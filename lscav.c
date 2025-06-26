// Macros

#define _GNU_SOURCE // Enables GNU extensions in glibc [DELETE WHEN POSIX]
#define _POSIX_C_SOURCE 200809L // Enables POSIX 2008 Standard Features
#define _XOPEN_SOURCE 700       // Enables X/Open Version 7 Standard Features

// Global Variables

int MAX_USERS = 0; // Discovery - User Limit

// Header Files 1

#include <getopt.h>      // Command-Line Option Parsing
#include <grp.h>         // Group Account Information
#include <pwd.h>         // User Account Information
#include <stdio.h>       // Standard I/O Functions
#include <stdlib.h>      // General Utility Functions
#include <string.h>      // String Functions
#include <sys/types.h>   // Defines System Data Types
#include <sys/utsname.h> // System Information
#include <unistd.h>      // POSIX System Calls

// Function Prototypes

void M001_MENU(int argc, char **argv); // Displays Menu & Handles Arguments
void M002_SYSTEM(void);                // Collects System Information
void M003_USERS(int REG_USR);          // Collects Users Information
void M004_GROUPS(void);                // Collects Group Information
void MAX_USERS_FUNC();

int main(int argc, char **argv) {

  // Main Process Functions

  M001_MENU(argc, argv);

  return 0;
}

void M001_MENU(int argc, char **argv) {
  if (argc < 2) {
    printf("\n[*] Usage Information\n");
    printf("\n./lscav \n\n");
    printf("[-u List All Users] \n");
    printf("[-g List All Groups]\n");
    printf("[-r List Regular Users Only] \n");
    printf("[-s List System Information] \n");
    printf("\n");

    exit(EXIT_FAILURE);
  } // Print Usage Message if no Argumentss

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-' &&
        strspn(&argv[i][1], "surg") == strlen(&argv[i][1])) {
      continue;
    }

    printf("\n[*] Warning Information\n\nWARNING: Bad input detected! %s\n\n",
           argv[i]);
    exit(EXIT_FAILURE);
  }

  opterr = 0; // Disable Error Message for GETOPT

  int flags[] = {0, 0, 0, 0, 0, 0}; // option, sflag, uflag, rflag, gflag, dflag

  while ((flags[0] = getopt(argc, argv, "-:surg")) != -1) {
    {
      switch (flags[0])

      {
      case 's':
        if (!flags[1]++) {
          printf("\n");
          M002_SYSTEM();
        }
        break;

      case 'u':
        if (!flags[2]++) {
          printf("\n");
          M003_USERS(0);
        }
        break;

      case 'r':
        if (!flags[3]++) {
          printf("\n");
          M003_USERS(1);
        }
        break;

      case 'g':
        if (!flags[4]++) {
          printf("\n");
          M004_GROUPS();
        }
        break;

      default:
        break;
      }
    }
  }
}

void M002_SYSTEM(void) {
  printf("\n[*] System Information\n\n");

  struct utsname uts;

  // Get system information using uname()
  if (uname(&uts) < 0) {
    perror("Error getting system information");
    return; // Return early if uname fails
  }

  // Print system information in a structured format
  printf("[-] OS          : %s\n", uts.sysname);
  printf("[-] Hostname    : %s\n", uts.nodename);
  printf("[-] Release     : %s\n", uts.release);
  printf("[-] Version     : %s\n", uts.version);
  printf("[-] Architecture: %s\n", uts.machine);

  printf("\n");
}

void M003_USERS(int REG_USR) {

  MAX_USERS_FUNC();

  printf("\n[*] Full User List\n\n");

  struct passwd *p_loop;

  uid_t uid = (REG_USR) ? 1000 : 0; // 1000 for Regular Users Only

  // Loop through users (either all or regular ones)
  while ((p_loop = (REG_USR ? getpwuid(uid) : getpwent())) != NULL) {
    // Print user details
    printf("%-20s: %s\n", "[-] Username", p_loop->pw_name);
    printf("%-20s: %d\n", "[-] UID", (int)p_loop->pw_uid);
    printf("%-20s: %d\n", "[-] GID", (int)p_loop->pw_gid);

    int ngroups = 0;

    // Get List of Groups the User Belongs To
    getgrouplist(p_loop->pw_name, p_loop->pw_gid, NULL, &ngroups);

    gid_t groups[ngroups];

    // Fill the Array with those Groups
    getgrouplist(p_loop->pw_name, p_loop->pw_gid, groups, &ngroups);

    printf("%-20s: ", "[-] Groups");

    // Print Group Information

    for (int i = 0; i < ngroups; i++) {
      struct group *g_single = getgrgid(groups[i]);
      printf("%s ", g_single->gr_name);
    }

    printf("\n");

    printf("%-20s: %s\n", "[-] Default Shell", p_loop->pw_shell);
    printf("%-20s: %s\n", "[-] Home Directory", p_loop->pw_dir);
    printf("%-20s: %s\n", "[-] Information", p_loop->pw_gecos);

    printf("\n\n");

    // Move to the next user (increment UID if we're showing regular users)
    if (REG_USR) {
      uid++;
    }
  }

  // End the user enumeration when done
  endpwent();
}

void M004_GROUPS(void) {

  MAX_USERS_FUNC();

  typedef struct {
    char username[256];
    gid_t gid;
  } User;

  User users[MAX_USERS];
  int user_count = 0;

  // Read user entries
  setpwent();
  struct passwd *pw;

  while ((pw = getpwent()) != NULL && user_count < MAX_USERS) {
    strncpy(users[user_count].username, pw->pw_name,
            sizeof(users[user_count].username) - 1);
    users[user_count].username[sizeof(users[user_count].username) - 1] = '\0';
    users[user_count].gid = pw->pw_gid;

    user_count++;
  }

  endpwent();

  // Read group entries
  setgrent();
  struct group *gr;

  while ((gr = getgrent()) != NULL) {
    printf("%-20s: %s\n", "[-] Groupname", gr->gr_name);
    printf("%-20s: %d\n", "[-] GID", (int)gr->gr_gid);

    printf("%-20s: ", "[-] Primary Users");

    for (int i = 0; i < user_count; i++) {
      if (users[i].gid == gr->gr_gid) {
        printf("%s ", users[i].username);
      }
    }

    printf("\n");

    printf("%-20s: ", "[-] Regular Members");
    int IS_PRIMARY;
    if (gr->gr_mem) {
      for (int i = 0; gr->gr_mem[i] != NULL; i++) {
        IS_PRIMARY = 0;

        for (int j = 0; j < user_count; j++) {
          if (users[j].gid == gr->gr_gid) {
            IS_PRIMARY = 1;
            break;
          }
        }

        if (!IS_PRIMARY) {
          printf("%s ", gr->gr_mem[i]);
        }
      }
    }

    printf("\n\n");
  }

  endgrent();
}

void MAX_USERS_FUNC(void)

{
  struct passwd *pw;

  setpwent();

  while ((pw = getpwent()) != NULL) {
    MAX_USERS++;
  }

  endpwent();
}

// TASKLIST

// TODO M004 - Check User Count Function - Place appropriately
// TODO M004 - Check User Count As Well
