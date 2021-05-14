#pragma once
/***************************************************************************** /
/**
 * @file    dbConnecter.h
 * @author  Juan Miguel Cerda Ordaz
 * @date    13/05/2021
 * @brief   gets data from database.
 */
/*****************************************************************************/

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include <SFML/Graphics.hpp>

#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <string>
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace std;

namespace dbTool {

  struct MemoryChunk {
    char* memory;
    size_t size;
  };

  class dbConnecter
  {
   public:
    dbConnecter();
    ~dbConnecter();
    
    void
    connect(string _dbName);

    void 
    getData(vector<sf::Vector3f>& _positionList);

    void 
    exportData();

    void 
    disconnect();

   protected:

   private:

    static size_t
    WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp);

    CURL* curlHandle;
    CURLcode res;
    struct MemoryChunk chunk;

  };

}
