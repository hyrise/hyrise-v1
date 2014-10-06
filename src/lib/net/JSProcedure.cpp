#ifdef WITH_V8

#include "net/JSProcedure.h"
#include <iostream>
#include <fstream>
#include "ebb/ebb.h"
#include <helper/HttpHelper.h>
#include "helper/vector_helpers.h"

#include <access/ScriptOperation.h>
#include "access/system/ResponseTask.h"

#include <boost/filesystem.hpp>

#include <helper/Settings.h>

#include <taskscheduler/AbstractTaskScheduler.h>
#include <taskscheduler/SharedScheduler.h>

namespace hyrise {
namespace net {

bool JSProcedure::registered = Router::registerRoute<JSProcedure>("/JSProcedure/");


std::string JSProcedure::name() { return "JSProcedure"; }

const std::string JSProcedure::vname() { return "JSProcedure"; }

std::string JSProcedure::readFile(std::string fileName) {
  std::string data;
  std::ifstream in(fileName);
  getline(in, data, std::string::traits_type::to_char_type(std::string::traits_type::eof()));

  return data;
}

void JSProcedure::showError(AsyncConnection* ac, const std::string errorMessage) {
  ac->respond(errorMessage);
  std::cerr << errorMessage << std::endl;
}

std::string exec(std::string cmd) {
  FILE* pipe = popen(cmd.c_str(), "r");
  if (!pipe)
    return "ERROR";
  char buffer[128];
  std::string result = "";
  while (!feof(pipe)) {
    if (fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  return result;
}

void JSProcedure::operator()() {
  if (auto ac = dynamic_cast<AsyncConnection*>(_connection)) {
    std::map<std::string, std::string> body_data = parseHTTPFormData(_connection->getBody());

    Json::Value request_data;
    Json::Reader reader;

    auto scheduler = taskscheduler::SharedScheduler::getInstance().getScheduler();

    const std::string scriptPath = Settings::getInstance()->getScriptPath() + "/";

    const std::string& query_string = urldecode(body_data["procedure"]);

    const std::string& parameterString = urldecode(body_data["parameter"]);

    std::vector<std::string> parameters;
    if (reader.parse(parameterString, request_data)) {
      Json::FastWriter writer;
      for (Json::Value::iterator it = request_data.begin(); it != request_data.end(); ++it) {
        parameters.push_back(writer.write(*it));
      }
    }

    if (reader.parse(query_string, request_data)) {
      if (request_data["action"].asString() == "execute") {
        auto rt = std::make_shared<hyrise::access::ResponseTask>(_connection);
        _connection->setResponseTask(rt);
        auto op = std::make_shared<hyrise::access::ScriptOperation>();

        op->setPlanOperationName("StoredProcedureExecution");
        op->setOperatorId("__StoredProcedureExecution");
        op->setParameters(parameters);

        if (request_data.isMember("procedureName")) {
          op->setScriptName(request_data["procedureName"].asString());
        } else if (request_data.isMember("procedureSource")) {
          op->setScriptSource(request_data["procedureSource"].asString());
        } else {
          showError(ac, "Neither procedureName or procedureSource provided for executing a JSProcedure.");
          return;
        }

        if (request_data.isMember("papiEvent")) {
          op->setEvent(request_data["papiEvent"].asString());
        }

        bool recordPerformance = getOrDefault(body_data, "performance", "false") == "true";
        rt->setRecordPerformanceData(recordPerformance);
        if (recordPerformance)
          rt->enableGetSubQueryPerformanceData(op);

        rt->registerPlanOperation(op);
        rt->addDependency(op);
        try {
          (*op)();
          (*rt)();
        }
        catch (std::exception& ex) {
          showError(ac, ex.what());  //@todo
        }
      } else if (request_data["action"].asString() == "get") {
        if (request_data.isMember("procedureName")) {
          // Be our guest, path traversal attack
          ac->respond(readFile(scriptPath + request_data["procedureName"].asString() + ".js"));
        } else {
          boost::filesystem::path path(scriptPath);
          if (boost::filesystem::is_directory(path)) {
            Json::Value fileNames(Json::arrayValue);
            Json::StyledWriter writer;

            boost::filesystem::recursive_directory_iterator it(path);
            boost::filesystem::recursive_directory_iterator endit;
            while (it != endit) {
              if (boost::filesystem::is_regular_file(*it) && it->path().extension() == ".js") {
                std::string fileName = it->path().filename().string();

                fileNames.append(fileName.substr(0, fileName.find(".js")));
              }
              ++it;
            }

            ac->respond(writer.write(fileNames));
          }
        }
      } else if (request_data["action"].asString() == "create") {
        if (request_data.isMember("procedureName") && request_data.isMember("procedureSource")) {
          std::ofstream procedureFile(scriptPath + request_data["procedureName"].asString() + ".js",
                                      std::ios::out | std::ios::trunc);
          procedureFile << request_data["procedureSource"].asString();
          procedureFile.close();
          ac->respond("OK");
        } else {
          showError(ac, "Missing procedureName or procedureSource for creating a JSProcedure.");
        }
      } else if (request_data["action"].asString() == "delete") {
        if (request_data.isMember("procedureName")) {
          std::string fileName(scriptPath + request_data["procedureName"].asString() + ".js");
          int returnOfRemove;

          if ((returnOfRemove = remove(fileName.c_str())) != 0)
            ac->respond("Could not delete " + request_data["procedureName"].asString() + " " +
                        std::to_string(returnOfRemove));
          else
            ac->respond("OK");
        } else {
          showError(ac, "Missing procedureName for deleting a JSProcedure.");
        }
      } else if (request_data["action"].asString() == "papiEventsAvailable") {
        Json::Value papiEvents(Json::arrayValue);
        Json::StyledWriter writer;

        std::stringstream papiEventsString(exec("/usr/local/bin/papi_avail | grep \"Yes   No\" | cut -d ' ' -f1"));
        std::string papiEvent;

        while (std::getline(papiEventsString, papiEvent)) {
          if (papiEvent != "PAPI_TOT_CYC")
            papiEvents.append(papiEvent);
        }

        ac->respond(writer.write(papiEvents));
      } else {
        showError(ac, "Unknown/no action for JSProcedure executer.");
      }
    }
  }
}
}
}

#endif