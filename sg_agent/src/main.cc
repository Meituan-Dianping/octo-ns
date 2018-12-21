#include <muduo/base/TimeZone.h>
#include <muduo/net/EventLoop.h>
#include <cthrift/cthrift_svr.h>
#include <iostream>
#include "mns/agent_server.h"
#include "mns/agent_init.h"
#include <TProcessor.h>
#include "ServiceAgent.h"
#include "mns/config_loader.h"
#include "mns/agent_init.h"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace meituan_cthrift;
using namespace meituan_mns;

int main(int argc, char **argv) {

  meituan_mns::AgentInit agent_init;
  std::string log_path = "";
  agent_init.PreloadLogConfigPath(log_path);
  log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(log_path));
  agent_init.Init();

  string str_svr_appkey(CXmlFile::GetStrPara(CXmlFile::AgentAppKey)); //服务端的appkey
  uint16_t u16_port = 5266;
  bool b_single_thread = false;  //当时单线程运行时，worker thread num 只能是1
  int32_t i32_timeout_ms = 30;
  int32_t i32_max_conn_num = 20000;
  int16_t i16_worker_thread_num = 4;
  switch (argc) {
    case 1:
      break;
    case 7:
      str_svr_appkey.assign(argv[1]);
      u16_port = static_cast<uint16_t>(atoi(argv[2]));
      b_single_thread = (1 == atoi(argv[3])) ? true : false;
      i32_timeout_ms = atoi(argv[4]);
      i32_max_conn_num = atoi(argv[5]);
      i16_worker_thread_num = static_cast<uint16_t>(atoi(argv[6]));
      break;
    default:
      exit(-1);
  }
  std::cout << "start server........." << std::endl;

  try {
    boost::shared_ptr<meituan_mns::SGAgentServer> handler(new meituan_mns::SGAgentServer());
    meituan_mns::ServiceAgentProcessor *processor_tmp = new
        meituan_mns::ServiceAgentProcessor(handler);
    boost::shared_ptr<apache::thrift::TProcessor> processor(processor_tmp);

    meituan_cthrift::CthriftSvr server(processor);

    if(server.Init() != 0) {
        cerr << "server init error" << endl;
        return -1;
    }

    sleep(6);
    server.serve();
    std::cout << "start naming service server..."<< std::endl;
  } catch (meituan_cthrift::TException &tx) {
    LOG_ERROR << tx.what();
    return -1;
  } catch (int e) {
    return e;
  }

  return 0;
}
