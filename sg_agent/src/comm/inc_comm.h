/*
 * Copyright (c) 2011-2018, Meituan Dianping. All Rights Reserved.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef _INC_COMM_H_
#define _INC_COMM_H_

#include <map>
#include <set>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <string.h>
#include <protocol/TBinaryProtocol.h>
#include <transport/TBufferTransports.h>
#include <transport/TSocket.h>
#include <Thrift.h>
#include <TApplicationException.h>
#include <protocol/TProtocol.h>
#include <transport/TTransport.h>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <execinfo.h>
#include <stdarg.h>
#include <sys/ipc.h>
#include <iomanip>
#include <zlib.h>

#include "thrift/protocol/TBinaryProtocol.h"
#include "thrift/transport/TSocket.h"
#include "thrift/transport/TTransportUtils.h"

#include "tinyxml2.h"
#include "log4cplus.h"
#include "md5.h"
#include "../util/base_errors_consts.h"
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace boost;

namespace meituan_mns {
#define SAFE_DELETE(p) { if(p) { delete (p); (p)=NULL; } }
#define SAFE_FREE(p) { if(p) { free(p); (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p); (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

#ifdef __GNUC__
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#else
#define likely(x)       (x)
#define unlikely(x)     (x)
#endif

const int MAX_INTERFACE_NUM = 32;

//公用方法
inline int SplitStringIntoVector(const char *sContent,
                                 const char *sDivider,
                                 std::vector<std::string> &vecStr) {
  char *sNewContent = new char[strlen(sContent) + 1];
  snprintf(sNewContent, strlen(sContent) + 1, "%s", sContent);
  char *pStart = sNewContent;

  std::string strContent;
  char *pEnd = strstr(sNewContent, sDivider);
  if (pEnd == NULL && strlen(sNewContent) > 0) {
    strContent = pStart; //get the last one;
    vecStr.push_back(strContent);
  }

  while (pEnd) {
    *pEnd = '\0';
    strContent = pStart;
    vecStr.push_back(strContent);

    pStart = pEnd + strlen(sDivider);
    if ((*pStart) == '\0') {
      break;
    }

    pEnd = strstr(pStart, sDivider);

    if (pEnd == NULL) {
      strContent = pStart; //get the last one;
      vecStr.push_back(strContent);
    }
  }

  SAFE_DELETE_ARRAY(sNewContent);
  return vecStr.size();
}
//从version中反解mtime
inline int64_t GetMtimeFromVersion(const std::string &version) {

  std::vector<std::string> version_vec;
  int64_t m_time = 0;

  if (!version.empty()) {
    SplitStringIntoVector(version.c_str(), "|", version_vec);
    if (3 != version_vec.size()) {
      NS_LOG_ERROR("the wrong version concludes" << version_vec.size());
      return 0;
    }
    try {
      m_time = boost::lexical_cast<int64_t>(version_vec[0]);
    } catch (bad_lexical_cast &ex) {
      NS_LOG_ERROR("convert the mtime from version failed,m_time: " << m_time);
    }
  }
  return m_time;
}

inline std::string Convert(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  const size_t SIZE = 512;
  char buffer[SIZE] = {0};
  vsnprintf(buffer, SIZE, fmt, ap);

  va_end(ap);

  return std::string(buffer);
}

//根据mtime ,cversion, version拼接版本信息
inline std::string GetVersion(const int64_t &mtime, const int64_t &cversion, const int64_t &version) {
  const size_t SIZE = 512;
  char buffer[SIZE] = {0};
  snprintf(buffer, sizeof(buffer), "%lu|%lu|%lu", mtime, cversion, version);

  return std::string(buffer);
}

/**
 * 获取本机IP
 * @return 0: success; -1:error
 */
inline int getLocalIp(std::string &ip) {
  struct ifaddrs *ifAddrStruct = NULL;
  void *tmpAddrPtr = NULL;

  getifaddrs(&ifAddrStruct);

  std::string eth0 = "eth0";
  std::string en0 = "en0";
  std::string em1 = "em1";
  std::string ifa_name;
  char addressBufferIPv4[INET_ADDRSTRLEN];

  while (ifAddrStruct != NULL) {
    // To check is it an IPv4
    if (ifAddrStruct->ifa_addr->sa_family == AF_INET) {
      // It is a valid IPv4 Address
      tmpAddrPtr = &((struct sockaddr_in *) ifAddrStruct->ifa_addr)->sin_addr;
      inet_ntop(AF_INET, tmpAddrPtr, addressBufferIPv4, INET_ADDRSTRLEN);
      ifa_name = std::string(ifAddrStruct->ifa_name);
      if (0 == eth0.compare(ifa_name) || 0 == en0.compare(ifa_name)
          || 0 == em1.compare(ifa_name)) {
        ip = std::string(addressBufferIPv4);
        return 0;
      }
    }
    ifAddrStruct = ifAddrStruct->ifa_next;
  }
  return -1;
}

inline int getIntranet(char ip[INET_ADDRSTRLEN], char mask[INET_ADDRSTRLEN]) {
  int ret = 0;

  struct ifaddrs *ifAddrStruct = NULL;
  struct ifaddrs *ifa = NULL;
  void *tmpAddrPtr = NULL;
  void *tmpMaskPtr = NULL;
  char addrArray[MAX_INTERFACE_NUM][INET_ADDRSTRLEN];
  char maskArray[MAX_INTERFACE_NUM][INET_ADDRSTRLEN];
  getifaddrs(&ifAddrStruct);
  int index = 0;
  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
    if ((NULL == ifa->ifa_addr) || (0 == strcmp(ifa->ifa_name, "vnic"))) {
      continue;
    }
    if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
      tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
      inet_ntop(AF_INET, tmpAddrPtr, addrArray[index], INET_ADDRSTRLEN);
      if (0 == strcmp(addrArray[index], "127.0.0.1")) {
        continue;
      }

      tmpMaskPtr = &((struct sockaddr_in *) ifa->ifa_netmask)->sin_addr;
      inet_ntop(AF_INET, tmpMaskPtr, maskArray[index], INET_ADDRSTRLEN);

      strcpy(ip, addrArray[index]);
      strcpy(mask, maskArray[index]);
      if (++index > MAX_INTERFACE_NUM - 1) {
        break;
      }

    }
  }

  if (index > 1) {
    int idx = 0;
    while (idx < index) {
      if (NULL != strstr(addrArray[idx], "10.")
          && 0 == strcmp(addrArray[idx],
                         strstr(addrArray[idx], "10."))) {
        strcpy(ip, addrArray[idx]);
        strcpy(mask, maskArray[idx]);
      }
      idx++;
    }
  } else if (0 >= index) {
    NS_LOG_ERROR("not get IP with getIntranet, index = " << index);
    ret = -1;
  }

  if (ifAddrStruct != NULL) {
    freeifaddrs(ifAddrStruct);
  }
  return ret;
}

inline int getHost(std::string &ip) {
  char hname[1024] = {0};
  gethostname(hname, sizeof(hname));
  struct hostent *hent;
  hent = gethostbyname(hname);
  if (NULL != hent) {
    ip = inet_ntoa(*(struct in_addr *) (hent->h_addr_list[0]));
  } else {
    ip = "";
    NS_LOG_ERROR("not get IP with getIntranet");
    return -1;
  }
  return 0;
}

/**
 * 获取本机IP, 因为在连接数超过1024时， 会获取失败， 与getLocalIP一起使用
 * @return 0: success; -1:error
 */
inline int getip(std::string &ip) {
  int ret = 0;
  char hname[1024] = {0};
  gethostname(hname, sizeof(hname));
  struct hostent *hent;
  hent = gethostbyname(hname);
  if (NULL != hent) {
    ip = inet_ntoa(*(struct in_addr *) (hent->h_addr_list[0]));
  } else {
    ip = "";
    // 使用另一种方式来获取ip
    ret = getLocalIp(ip);
  }
  return ret;
}

/**
 * 获取本机IP, 因为在连接数超过1024时， 会获取失败， 与getLocalIP一起使用
 * @return 0: success; -1:error
 */
inline int getip2(std::string &ip) {
  int ret = 0;
  ret = getLocalIp(ip);
  if (0 != ret || ip.empty()) {
    return getHost(ip);
  }
  return ret;
}

//check appkey是否为字母，数字，下划线，减号，点组成
inline bool IsAppkeyLegal(const std::string &appkey) {
  char ch;
  for (int i = 0; i < appkey.length(); i++) {
    ch = appkey[i];
    if ((ch >= '0') && (ch <= '9')) {
      continue;
    } else if (((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z'))) {
      continue;
    } else if ((ch == '-') || (ch == '_') || (ch == '.')) {
      continue;
    } else {
      return false;
    }
  }
  return true;
}

//check appkey是否为字母，数字，下划线，减号，点组成
inline bool IsIpAndPortLegal(const std::string &ip, const int port) {
  //IP格式*.*.*.*, *为1~3字符
  if ((7 > ip.size()) || (15 < ip.size())) {
    return false;
  }

  if (0 >= port) {
    return false;
  }
  return true;
}

const int MAX_PATHLEN = 512;
inline int mkCommonDirs(const char *muldir) {
  if (NULL == muldir) {
    return -1;
  }
  int len = strlen(muldir);
  if (MAX_PATHLEN <= strlen(muldir)) {
    return -1;
  }

  NS_LOG_DEBUG("path = " << muldir);

  char path[MAX_PATHLEN];
  strncpy(path, muldir, len);
  path[len] = '\0';
  int ret = 0;
  for (int i = 0; i < len; i++) {
    if ('/' == muldir[i] && 0 != i) {
      path[i] = '\0';
      if (access(path, 0) != 0) {
        ret = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      }
      path[i] = '/';
    }
  }
  if (len > 0 && access(path, 0) != 0) {
    ret = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir(path, S_IREAD | S_IWRITE);
  }
  return 0;
}

inline int loadFile(std::string &filecontent, const std::string &filename, const std::string &filepath) {
  int ret = -1;
  const std::string fullname = filepath + "/" + filename;
  std::ifstream in(fullname.c_str());
  if (in.is_open()) {
    in.seekg(0, std::ios::end);
    filecontent.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&filecontent[0], filecontent.size());
    in.close();
    ret = 0;
  }

  return ret;
}

//将内容写入临时文件，若写失败返回错误码
inline int writeToTmp(const std::string &fileContent, const std::string &filename, const std::string &filepath) {

  int ret = mkCommonDirs(filepath.c_str());
  if (SUCCESS == ret) {

    const std::string tmp_file = filepath + "/" + filename + ".tmp";
    std::ofstream fout(tmp_file.c_str());
    if (!fout.is_open()) {
      NS_LOG_ERROR("fwrite fail filename = " << filename << "; filepath = " << filepath);
      return FAILURE;
    }
    fout << fileContent;
    fout.close();
    std::string tmpFileContent = "";
    std::string tmpMd5 = "";
    std::string loadName = filename + ".tmp";
    ret = loadFile(tmpFileContent, loadName, filepath);

    if (SUCCESS == ret) {
      MD5 md5fileContent(fileContent);
      MD5 md5fileContentTmp(tmpFileContent);
      if (md5fileContent.md5() != md5fileContentTmp.md5()) {
        NS_LOG_ERROR("write the tmp file failed, tmp_file filename = " << filename);
        return ERR_DISK_WRITE;
      } else {
        NS_LOG_INFO("write the tmp file success" << filepath);
      }
    } else {
      NS_LOG_INFO("load the has write file failed" << filepath);
    }
  }

  return ret;
}

inline int moveForWork(const std::string &filename, const std::string &filepath) {
  const std::string tmp_file = filepath + "/" + filename + ".tmp";
  const std::string work_file = filepath + "/" + filename;
  std::string cmd = "mv " + tmp_file + " " + work_file;
  int status = std::system(cmd.c_str());
  return status;
}


//将内容写入临时文件，若写失败返回错误码
inline int WriteToDisk(const std::string &fileContent, const std::string &filename, const std::string &filepath) {

  int ret = mkCommonDirs(filepath.c_str());
  if (0 == ret) {

    const std::string tmp_file = filepath + "/" + filename + ".tmp";
    std::ofstream fout(tmp_file.c_str());
    if (!fout.is_open()) {
      NS_LOG_ERROR("fwrite fail filename = " << filename << "; filepath = " << filepath);
      return -1;
    }
    fout << fileContent;
    fout.close();
    std::string tmpFileContent = "";
    std::string tmpMd5 = "";
    std::string loadName = filename + ".tmp";
    ret = loadFile(tmpFileContent, loadName, filepath);

    if (0 == ret) {
      MD5 md5fileContent(fileContent);
      MD5 md5fileContentTmp(tmpFileContent);
      if (md5fileContent.md5() != md5fileContentTmp.md5()) {
        NS_LOG_ERROR("write the tmp file failed, tmp_file filename = " << filename);
        return -1;
      } else {
        NS_LOG_INFO("write the tmp file success" << filepath);
        const std::string work_file = filepath + "/" + filename;
        std::string cmd = "mv " + tmp_file + " " + work_file;
        ret = std::system(cmd.c_str());
      }
    } else {
      NS_LOG_INFO("load the has write file failed" << filepath);
    }
  }
  return ret;
}

inline void getHostInfo(char hostInfo[256], char ip[INET_ADDRSTRLEN]) {
  FILE *fp;
  char hostCMD[64] = {0};
  strncpy(hostCMD, "host ", 5);
  strncpy(hostCMD + 5, ip, INET_ADDRSTRLEN);
  fp = popen(hostCMD, "r");
  fgets(hostInfo, 256, fp);
  pclose(fp);
  fp = NULL;
}

inline bool isOnlineHost(char ip[INET_ADDRSTRLEN], std::string &hostType) {
  char host[256] = {0};
  getHostInfo(host, ip);
  int isOnline = 0;
  if (strstr(host, ".office.mos") > 0) {
    isOnline = false;
    hostType = ".office.mos";
  } else if (strstr(host, "not found") > 0) {
    isOnline = false;
    hostType = "not found";
  } else if (strstr(host, ".corp.octo.com") > 0) {
    isOnline = false;
    hostType = ".corp.octo.com";
  } else if (strstr(host, ".octo.com") > 0) {
    isOnline = true;
    hostType = ".octo.com";
  }
  return isOnline;
}

inline static bool isPortOpen(const std::string &address, int port) {
  bool isOpen = false;

  boost::shared_ptr<TSocket> m_socket
      = boost::shared_ptr<TSocket>(new TSocket(address, port));
  boost::shared_ptr<TTransport> m_transport
      = boost::shared_ptr<TFramedTransport>(new TFramedTransport(m_socket));

  try {
    //设置超时30ms
    m_socket->setConnTimeout(30);
    m_socket->setLinger(0, 0);
    m_transport->open();
    if (m_transport->isOpen()) {
      isOpen = true;
    }
  } catch (TException &tx) {
    NS_LOG_ERROR("TException Error: " << tx.what());
  }
  if (NULL != m_transport) {
    m_transport->close();
  }
  return isOpen;
}
inline void unixTime2Str(int time, char str_time[], int buf_len) {
  time_t last_time = time;
  tm *last_time_tmp = localtime(&last_time);
  strftime(str_time, buf_len - 1, "%Y-%m-%d %H:%M:%S", last_time_tmp);
  int real_len = strlen(str_time);
  str_time[real_len + 1] = '\0';
  NS_LOG_INFO("unixTime2Str:lastupdatedtime = " << str_time);
}
inline int ConvertInt2String(const int &value, std::string &toString) {
  int ret = 0;
  try {
    toString = boost::lexical_cast<std::string>(value);
  } catch (bad_lexical_cast &ex) {
    NS_LOG_ERROR("convert the string to int failed,value: " << toString);
    ret = -1;
  }
  return ret;
}
inline long GetProcMemUtil(const int pid) {

  const int INDEX_SPLIT = 1;
  long value = 0;
  bool find = false;
  std::string pid_value = "";
  std::string tmp = "", tmp_value = "";
  std::string vmrss = "", vmrss_value = "";
  std::vector<std::string> vec_seg_tag;
  if (0 != ConvertInt2String(pid, pid_value)) {
    return FAILURE;
  }
  NS_LOG_INFO("the pid is " << pid << "std is: " << pid_value);
  std::string name = "/proc/" + pid_value + "/status";
  std::ifstream ifile(name.c_str());
  if (!ifile.is_open()) {
    NS_LOG_INFO("faild to open file name: " << name);
    ifile.close();
    return -1;
  }
  while (getline(ifile, tmp)) {
    if (std::string::npos != tmp.find("VmRSS")) {
      tmp_value = tmp;
      find = true;
      ifile.close();
      break;
    }
  }

  if (find) {
    boost::split(vec_seg_tag, tmp_value, boost::is_any_of(("\t")));
    if (vec_seg_tag.size() >= INDEX_SPLIT) {
      boost::trim(vec_seg_tag[INDEX_SPLIT]);
      vmrss = vec_seg_tag[INDEX_SPLIT];
      size_t pos_vmrss = tmp_value.find(" ");
      if (std::string::npos != pos_vmrss) {
        vmrss_value.assign(vmrss.c_str(), pos_vmrss - 2);
        try {
          value = boost::lexical_cast<long>(vmrss_value.c_str());
        } catch (bad_lexical_cast &ex) {
          NS_LOG_ERROR("convert the string to int failed");
          return -1;
        }
      }
    }
  }
  if (value < 0) {
    NS_LOG_ERROR("value is invalid");
    return -1;
  }
  return value;
}
inline unsigned int GetProcCpuTime(const int pid) {

  unsigned int proc_util = 0;
  const int BEGIN_INDEX = 13, END_INDEX = 16;
  std::string pid_value = "";
  std::vector<std::string> vec_seg_tag;
  if (0 != ConvertInt2String(pid, pid_value)) {
    return -1;
  }
  std::string tmpFileContent = "";
  std::string filepath = "/proc/" + pid_value + "/stat";
  std::string tmp = "";
  std::ifstream ifile(filepath.c_str());
  if (!ifile.is_open()) {
    NS_LOG_INFO("faild to open file path" << filepath);
    ifile.close();
    return -1;
  }
  while (getline(ifile, tmp)) {
    tmpFileContent.append(tmp);
  }
  boost::split(vec_seg_tag, tmpFileContent, boost::is_any_of((" ")));
  for (int pos = BEGIN_INDEX; pos <= END_INDEX; ++pos) {
    unsigned int value = 0;
    try {
      value = boost::lexical_cast<unsigned int>(vec_seg_tag[pos].c_str());
    } catch (bad_lexical_cast &ex) {
      NS_LOG_ERROR("convert the string to int failed");
      ifile.close();
      return -1;
    }
    proc_util += value;
  }
  ifile.close();
  return proc_util;
}
inline unsigned int GetTotalCpuTime() {

  const int NAME_LEN = 16;
  unsigned int user = 0, nice = 0, system = 0, idle = 0;
  char name[NAME_LEN] = {'0'};
  std::string filepath = "/proc/stat";
  std::string tmp = "";
  std::ifstream ifile(filepath.c_str());
  getline(ifile, tmp);
  if (!tmp.empty()) {
    sscanf(tmp.c_str(), "%s %u %u %u %u", name, &user, &nice, &system, &idle);
  }
  return (user + nice + system + idle);
}
inline float Round(float value, unsigned int num) {

  float round_float_num = 0.0;
  std::ostringstream f_out;
  f_out << std::setiosflags(std::ios::fixed) << std::setprecision(num) << value;
  try {
    round_float_num = boost::lexical_cast<float>(f_out.str());
  } catch (bad_lexical_cast &ex) {
    NS_LOG_ERROR("convert the f_out to float failed,round_float_num: " << round_float_num);
    return -1;
  }
  return round_float_num;
}

inline int gzCompress(const char *src, int srcLen, char *dest, int destLen) {
  z_stream c_stream;
  int err = 0;
  int windowBits = 15;
  int GZIP_ENCODING = 16;

  if (src && srcLen > 0) {
    c_stream.zalloc = (alloc_func) 0;
    c_stream.zfree = (free_func) 0;
    c_stream.opaque = (voidpf) 0;
    if (deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                     windowBits | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY) != Z_OK)
      return FAILURE;
    c_stream.next_in = (Bytef *) src;
    c_stream.avail_in = srcLen;
    c_stream.next_out = (Bytef *) dest;
    c_stream.avail_out = destLen;
    while (c_stream.avail_in != 0 && c_stream.total_out < destLen) {
      if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK) {
        NS_LOG_ERROR("Failed to deflate.");
        return -1;
      }
    }
    if (c_stream.avail_in != 0) return c_stream.avail_in;
    for (;;) {
      if ((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END) break;
      if (err != Z_OK) {
        NS_LOG_ERROR("Failed to deflate, err = " << err);
        return -1;
      }
    }
    if (deflateEnd(&c_stream) != Z_OK) {
      NS_LOG_ERROR("Failed to deflateEnd.");
      return -1;
    }
    return c_stream.total_out;
  }
  return -1;
}

inline int32_t Ip2Int(const char *ip) {
  if (NULL == ip) {
    NS_LOG_ERROR("ip is null");
    return 0;
  }
  std::vector<std::string> ips;
  SplitStringIntoVector(ip, ".", ips);
  if (4 != ips.size()) {
    NS_LOG_ERROR("ip form is wrroy, ip = " << ip);
    return 0;
  }
  int32_t res =
      (atoi(ips[0].c_str()) << 24) + (atoi(ips[1].c_str()) << 16) + (atoi(ips[2].c_str()) << 8) + atoi(ips[3].c_str());
  return res;
}

inline long DeltaTime(timeval end, timeval start) {
  return (end.tv_sec - start.tv_sec) * 1000000L
      + (end.tv_usec - start.tv_usec);
}
/*
 * Function name : GetUInt32
 * Creating Time : 2018-3-29 12:04:36
 * Description   : 从网络字节顺序的源buf中获得一个UInt32数据，转换成机器字节顺序，
                    源buf后移4字节
 * Return type   :
 * Argument      : char const* &pchInSour
*/
inline unsigned int GetUInt32(const unsigned char *pchInSour) {
  unsigned int nRtn;
  memcpy(&nRtn, pchInSour, sizeof(nRtn));
  nRtn = ntohl(nRtn);
  return nRtn;
}
/*
 * Function name : GetUInt16
 * Creating Time : 2018-3-29 12:04:36
 * Description   : 从网络字节顺序的源buf中获得一个UInt16数据，转换成机器字节顺序，
                   源buf后移2字节
 * Return type   :
 * Argument      : char const* &pchInSour
*/
inline unsigned short GetUInt16(const unsigned char *pchInSour) {
  unsigned short nRtn;
  memcpy(&nRtn, pchInSour, 2);
  pchInSour += 2;
  nRtn = ntohs(nRtn);
  return nRtn;
}
inline void SetUInt16(char *&pchIODes, unsigned short u16In) {
  u16In = htons(u16In);
  memcpy(pchIODes, &u16In, 2);
  pchIODes += 2;
}

inline void SetChars(char *&pDes, const char *pSour, int nSetNum) {
  if (NULL == pSour || 0 == nSetNum) {
    NS_LOG_INFO("the pSouur is null,or nSetNum is less 0");
    return;
  }
  memcpy(pDes, pSour, nSetNum);
  pDes += nSetNum;
}

inline void SetUInt32(char *&pchIODes, int u32In) {
  u32In = htonl(u32In);
  memcpy(pchIODes, &u32In, 4);
  pchIODes += 4;
}
inline unsigned int GetBit(unsigned int value, unsigned int index) {
  if (index < 0 || index > 31) {
    return FAILURE; //如果传入参数有问题，则返回0xff，表示异常
  }
  return (value >> index) & 1UL;
}
inline void SetBit(unsigned int &value, unsigned int index, unsigned int set_value) {

  if (index < 0 || index > 31) {
    return;
  }
  if (set_value == 1) {
    value |= (1UL << index);        // 将index的bit位设置为1，其他位不变
  } else if (set_value == 0) {
    value &= ~(1UL << index);       // 将index的bit位设置为0，其他位不变
  } else {
    NS_LOG_ERROR("the set value is invalid");
  }
}
inline void printfBufferSec(unsigned char *buffer_msg, unsigned int msgLen) {
  for (int iter = 0; iter < msgLen; iter++) {
    if (iter % 16 == 0) {
      NS_LOG_INFO("\n");
    }
    NS_LOG_INFO(" %02x " << *(buffer_msg + iter));
  }
  NS_LOG_INFO("\n");
}

inline void GetString(char const *&pchInSour, char *pOut, int nNum) {
  memcpy(pOut, pchInSour, nNum);
  pchInSour += nNum;
}

inline int64_t GetCurrentTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (int64_t) tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

inline std::string CalculateMd5(const std::string &file_name,
                                const std::string &file_path) {
  std::string context = "";
  int32_t ret = loadFile(context, file_name, file_path);
  if (SUCCESS != ret) {
    NS_LOG_ERROR("failed to load " << file_name);
    return "";
  }
  MD5 md5_string(context);
  return md5_string.md5();
}


}
#endif

