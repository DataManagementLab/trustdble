#include <assert.h>
#include <ctype.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <mutex>
#include <new>
#include <regex>
#include <string>
#include <vector>

#include "mysql/udf_registration_types.h"
typedef std::string (*pointer)();

extern "C" long long call_data_contract(UDF_INIT *initid , UDF_ARGS *args ) {
   char buf[300];
   char* data_contract_name= ((char*) args->args[0]);
   char* parameters= ((char*) args->args[1]);
   sprintf(buf, "python3 -m jurigged -v client.py %s %s",data_contract_name,parameters );
   int err;
   // if execution fails return 1
   if ((err = system(buf)))
   {
      printf("There was an error (%i)\n", err);
      return 1;
   }
return 0;
}

extern "C" bool call_data_contract_init(UDF_INIT *initid, UDF_ARGS *args,
                                   char *message) {
  if (args->arg_count != 2)
    {
        strcpy(message, "wrong number of arguments: call_data_contract() requires 2 columns as arguments");
        return 1;
    }

  struct call_data_contract_data *data;
  initid->ptr = (char *)data;

  initid->const_item=1;
  return 0;
}

extern "C" void call_data_contract_deinit(UDF_INIT *initid) {
  return; }

