/***************************************************************************** /
/**
 * @file    jsonExtractor.cpp
 * @author  Juan Miguel Cerda Ordaz
 * @date    11/05/2021
 * @brief   get data from database and creates a json file.
 */
/*****************************************************************************/


/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "dbApp.h"

using namespace dbTool;

int 
main(int numArgs, char* args[]) {

  dbApp app;
  app.run();

  return 0;
}
