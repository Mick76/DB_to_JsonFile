#include "dbConnecter.h"

namespace dbTool {

  dbConnecter::dbConnecter() {

  }

  dbConnecter::~dbConnecter() {

  }

  void 
  dbConnecter::connect(string _dbName) {
    //"http://localhost:9200/objpos/_search?pretty" -H "Content-Type: application/json" -d "{\"from\" : 0, \"size\" : 1000, \"query\": {\"query_string\": {\"query\": \"*\"}}}"
    chunk.memory = static_cast<char*>(malloc(1));
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);

    //init curl
    curlHandle = curl_easy_init();

    //specify url
    std::string sUrl = "http://localhost:9200/" + _dbName + "/_search?pretty";
    curl_easy_setopt(curlHandle, CURLOPT_URL, sUrl.c_str());

    //function to send recieved data
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    //pass chunk to callback function
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, (void*)&chunk);
  }

  void 
  dbConnecter::getData(vector<sf::Vector3f>& _positionList) {

    free(chunk.memory);
    chunk.memory = static_cast<char*>(malloc(1));
    chunk.size = 0;

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

      //clear old data
      _positionList.clear();
      //write data into the position list
      int hitCount = d["hits"]["hits"].GetArray().Size();
      for (int i = 0; i < hitCount; ++i) {
        //since its top down view, ignore y position
        sf::Vector3f newPos(std::stof(d["hits"]["hits"][i]["_source"]["pos"][1].GetString()),//x
                            std::stof(d["hits"]["hits"][i]["_source"]["pos"][5].GetString()),//z
                            std::stof(d["hits"]["hits"][i]["_source"]["pos"][3].GetString()));//y
        _positionList.push_back(newPos);
      }

    }
  }

  void 
  dbConnecter::exportData()
  {
    free(chunk.memory);
    chunk.memory = static_cast<char*>(malloc(1));
    chunk.size = 0;

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
        writer.String(d["hits"]["hits"][i]["_source"]["pos"][1].GetString());
        writer.Key("y");
        writer.String(d["hits"]["hits"][i]["_source"]["pos"][3].GetString());
        writer.Key("z");
        writer.String(d["hits"]["hits"][i]["_source"]["pos"][5].GetString());
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
  }

  void 
  dbConnecter::disconnect() {
    //close connections the handle openned
    curl_easy_cleanup(curlHandle);

    //deallocate
    free(chunk.memory);

    //clean libcurl
    curl_global_cleanup();
  }

  size_t 
  dbConnecter::WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
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
}