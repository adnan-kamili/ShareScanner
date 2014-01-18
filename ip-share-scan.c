#include <libsmbclient.h>
#include <stdio.h>
#include "scan.h"

void auth_fn()
{
    
}
	
int main(int argc,char* argv[])   
{ 
  //static char data[200];
  char *data;
  int dirHandle;
  if(smbc_init(auth_fn, 0)) /* Initialize things */
     {
		 return 0;
     }
  if(*(argv[1]+0)=='e')
  {
	  dirHandle= smbc_opendir(argv[1]+1);    /* Argument is smb://<ip-address>/ */
      struct smbc_dirent* a;
      while(a=smbc_readdir(dirHandle))
      {
	  if(a->smbc_type==SMBC_DIR||a->smbc_type==SMBC_FILE_SHARE)
	  {
		  printf("%s%s<*>",a->name,"SDR");
	  }
	  else if(a->smbc_type==SMBC_FILE)
	  {
		  printf("%s%s<*>",a->name,"SFE");
	  }
	  
      }
      smbc_closedir(dirHandle);
      return 0;
	  
  }
  data = scan(argv[1]+6);
  if(strcmp(data, "NOTFOUND"))
  {
	  puts(data);
	  dirHandle= smbc_opendir(argv[1]);    /* Argument is smb://<ip-address>/ */
      struct smbc_dirent* a;
      while(a=smbc_readdir(dirHandle))
      {
	  if(a->smbc_type==SMBC_DIR||a->smbc_type==SMBC_FILE_SHARE)
	  {
		  printf("%s%s<*>",a->name,"SDR");
	  }
	  else if(a->smbc_type==SMBC_FILE)
	  {
		  printf("%s%s<*>",a->name,"SFE");
	  }
	  
  }
  smbc_closedir(dirHandle);
  }

  
  
  return 0;
}

