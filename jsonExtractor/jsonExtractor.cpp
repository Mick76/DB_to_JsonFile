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
#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <string>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
using std::cout;

struct MemoryChunk {
  char* memory;
  size_t size;
};

static size_t
WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
  size_t realsize = size * nmemb;
  struct MemoryChunk* mem = (struct MemoryChunk*)userp;

  char* ptr = static_cast<char*>(realloc(mem->memory, mem->size + realsize + 1));
  if (!ptr) {
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

int 
main(int numArgs, char* args[]) {

  std::string database;
  cout << "database: ";
  std::cin >> database;

  CURL* curlHandle;
  CURLcode res;

  struct MemoryChunk chunk;

  chunk.memory = static_cast<char*>(malloc(1));
  chunk.size = 0;

  curl_global_init(CURL_GLOBAL_ALL);

  //init curl
  curlHandle = curl_easy_init();

  //specify url
  std::string sUrl = "http://localhost:9200/" + database + "/_search?pretty";
  curl_easy_setopt(curlHandle, CURLOPT_URL, sUrl.c_str());

  //function to send recieved data
  curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  //pass chunk to callback function
  curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, (void*)&chunk);

  //performs request
  res = curl_easy_perform(curlHandle);

  //check errors
  if (CURLE_OK != res) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
      curl_easy_strerror(res));
  }
  else {
    printf("\nConnected\n");
    printf("%lu bytes retrieved\n", (unsigned long)chunk.size);

    //parse json
    Document d;
    d.Parse(chunk.memory);

    //Stringify
    StringBuffer buffer;
    Writer<StringBuffer> writerQuery(buffer);
    d.Accept(writerQuery);
    std::cout << buffer.GetString() << std::endl;

    //build new json
    StringBuffer s;
    Writer<StringBuffer> writer(s);

    writer.StartObject();
    writer.Key("positions");
    writer.StartArray();
    int hitCount = d["hits"]["hits"].GetArray().Size();
    for (int i = 0; i < hitCount; ++i) {
      writer.StartObject();
      writer.Key("name");
      writer.String(d["hits"]["hits"][i]["_source"]["name"].GetString());

      writer.Key("pos");
      writer.StartArray();
      writer.Key("x");
      writer.String(d["hits"]["hits"][i]["_source"]["pos"][0].GetString());
      writer.Key("y");
      writer.String(d["hits"]["hits"][i]["_source"]["pos"][1].GetString());
      writer.Key("z");
      writer.String(d["hits"]["hits"][i]["_source"]["pos"][2].GetString());
      writer.EndArray();

      writer.Key("timestamp");
      writer.String(d["hits"]["hits"][i]["_source"]["timestamp"].GetString());
      writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();
    cout << std::endl << s.GetString() << std::endl;

    //create file
    std::ofstream myfile;
    myfile.open("positions.json");
    myfile << s.GetString();
    myfile.close();
  }

  //close connections the handle openned
  curl_easy_cleanup(curlHandle);

  //deallocate
  free(chunk.memory);

  //clean libcurl
  curl_global_cleanup();

  //Done
  std::string path(args[0]);
  cout << "\n\nDone! json file created in: " << path << std::endl;
  system("pause");
  return 0;
}

