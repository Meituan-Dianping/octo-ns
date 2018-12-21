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

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/foreach.hpp>

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#include "mns_sdk_common.h"

namespace mns_sdk {

extern mns_sdk::MnsConfig g_config;

const double MnsSdkCommon::kDFirstRegionMin = 1.0;
const double MnsSdkCommon::kDSecondRegionMin = 0.001;

static const int hex_table[] = {0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 1, 2, 3, 4, 5, 6,
                                7, 8, 9, 0, 0, 0, 0, 0, 0,
                                0, 10, 11, 12, 13, 14, 15, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 10,
                                11, 12, 13, 14, 15};

MnsSdkCommon::MnsSdkCommon(void) {
}

string MnsSdkCommon::SGService2String(const meituan_mns::SGService &sgservice) {
  string str_ret
      ("appkey:" + sgservice.appkey + " version:" + sgservice.version + " ip:"
           + sgservice.ip);

  string str_tmp;

  try {
    str_ret.append(" port:" + boost::lexical_cast<string>(sgservice.port));
  } catch (boost::bad_lexical_cast &e) {

    MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                              << "sgservice.port : "
                                              << sgservice.port);
  }

  try {
    str_ret.append(
        " weight:" + boost::lexical_cast<string>(sgservice.weight));
  } catch (boost::bad_lexical_cast &e) {

    MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                              << "sgservice.weight : "
                                              << sgservice.weight);
  }

  try {
    str_ret.append(
        " status:" + boost::lexical_cast<string>(sgservice.status));
  } catch (boost::bad_lexical_cast &e) {

    MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                              << "sgservice.status : "
                                              << sgservice.status);
  }

  try {
    str_ret.append(" role:" + boost::lexical_cast<string>(sgservice.role));
  } catch (boost::bad_lexical_cast &e) {

    MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                              << "sgservice.role : "
                                              << sgservice.role);
  }

  try {
    str_ret.append(
        " envir:" + boost::lexical_cast<string>(sgservice.envir));
  } catch (boost::bad_lexical_cast &e) {

    MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                              << "sgservice.envir : "
                                              << sgservice.envir);
  }

  try {
    str_ret.append(" lastUpdateTime:"
                       + boost::lexical_cast<string>(sgservice.lastUpdateTime));
  } catch (boost::bad_lexical_cast &e) {

    MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                              << "sgservice.lastUpdateTime : "
                                              << sgservice.lastUpdateTime);
  }

  try {
    str_ret.append(
        " fweight:" + boost::lexical_cast<string>(sgservice.fweight));
  } catch (boost::bad_lexical_cast &e) {

    MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                              << "sgservice.fweight : "
                                              << sgservice.fweight);
  }

  try {
    str_ret.append(" serverType:"
                       + boost::lexical_cast<string>(sgservice.serverType));
  } catch (boost::bad_lexical_cast &e) {

    MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                              << "sgservice.serverType : "
                                              << sgservice.serverType);
  }

  return str_ret;
}

int8_t MnsSdkCommon::ParseSentineSgagentList(const string &str_req,
                                             vector<meituan_mns::SGService> *p_vec_sgservice) {
  rapidjson::Document reader;
  if ((reader.Parse(str_req.c_str())).HasParseError()) {
    MNS_LOG_WARN("json parse string " << str_req << " failed");
    return -1;    //maybe NOT error
  }

  rapidjson::StringBuffer buffer;
  rapidjson::Writer <rapidjson::StringBuffer> writer(buffer);
  reader.Accept(writer);
  MNS_LOG_DEBUG("receive body " << string(buffer.GetString()));

  int i_ret;
  if (FetchInt32FromJson("ret", reader, &i_ret)) {
    return -1;
  }
  MNS_LOG_DEBUG("i_ret " << i_ret);

  if (200 != i_ret) {
    MNS_LOG_ERROR("fetch santine sgagent list failed, i_ret " << i_ret);
    return -1;
  }

  rapidjson::Value::MemberIterator it_data;
  if (FetchJsonValByKey4Doc(reader, "data", &it_data)
      || !((it_data->value).IsObject())) {
    MNS_LOG_ERROR("FetchJsonValByKey4Doc Wrong OR data is NOT object");
    return -1;
  }

  rapidjson::Value &data_val = it_data->value;

  rapidjson::Value::MemberIterator it;
  if (FetchJsonValByKey4Val(data_val, "serviceList", &it)) {
    return -1;
  }

  rapidjson::Value &srv_list = it->value;
  if (!(srv_list.IsArray())) {
    MNS_LOG_ERROR("srv_list NOT array");
    return -1;
  }

  rapidjson::Document::MemberIterator itr_sing;
  for (rapidjson::Value::ValueIterator itr = srv_list.Begin();
       itr != srv_list.End(); ++itr) {
    meituan_mns::SGService sgservice;
    const rapidjson::Value &srvlist_info = *itr;

    if (FetchJsonValByKey4Val(const_cast<rapidjson::Value &>(srvlist_info),
                              "appkey",
                              &itr_sing)
        || !((itr_sing->value).IsString())) {
      MNS_LOG_ERROR("appkey NOT EXIST OR NOT string OR NOT ");
      continue;
    }

    //sgservice.appkey.assign((itr_sing->value).GetString());
    sgservice.appkey.assign(g_config.chrion_appkey_); //replace to be sgagent appkey!!

    if (FetchJsonValByKey4Val(const_cast<rapidjson::Value &>(srvlist_info),
                              "version",
                              &itr_sing)
        || !((itr_sing->value).IsString())) {
      MNS_LOG_ERROR("version NOT EXIST OR NOT string");
      continue;
    }

    sgservice.version.assign((itr_sing->value).GetString());

    if (FetchJsonValByKey4Val(const_cast<rapidjson::Value &>(srvlist_info),
                              "ip",
                              &itr_sing)
        || !((itr_sing->value).IsString())) {
      MNS_LOG_ERROR("version NOT EXIST OR NOT string");
      continue;
    }

    sgservice.ip.assign((itr_sing->value).GetString());

    if (FetchJsonValByKey4Val(const_cast<rapidjson::Value &>(srvlist_info),
                              "port",
                              &itr_sing)
        || !((itr_sing->value).IsInt())) {
      MNS_LOG_ERROR("version NOT EXIST OR NOT int");
      continue;
    }

    sgservice.port = (itr_sing->value).GetInt();

    if (FetchJsonValByKey4Val(const_cast<rapidjson::Value &>(srvlist_info),
                              "weight",
                              &itr_sing)
        || !((itr_sing->value).IsInt())) {
      MNS_LOG_ERROR("weight NOT EXIST OR NOT int");
      continue;
    }

    sgservice.weight = (itr_sing->value).GetInt();

    if (FetchJsonValByKey4Val(const_cast<rapidjson::Value &>(srvlist_info),
                              "status",
                              &itr_sing)
        || !((itr_sing->value).IsInt())) {
      MNS_LOG_ERROR("status NOT EXIST OR NOT int");
      continue;
    }

    sgservice.status = (itr_sing->value).GetInt();

    if (FetchJsonValByKey4Val(const_cast<rapidjson::Value &>(srvlist_info),
                              "role",
                              &itr_sing)
        || !((itr_sing->value).IsInt())) {
      MNS_LOG_ERROR("role NOT EXIST OR NOT int");
      continue;
    }

    sgservice.role = (itr_sing->value).GetInt();

    if (FetchJsonValByKey4Val(const_cast<rapidjson::Value &>(srvlist_info),
                              "env",
                              &itr_sing)
        || !((itr_sing->value).IsInt())) {
      MNS_LOG_ERROR("envir NOT EXIST OR NOT int");
      continue;
    }

    sgservice.envir = (itr_sing->value).GetInt();

    if (FetchJsonValByKey4Val(const_cast<rapidjson::Value &>(srvlist_info),
                              "lastUpdateTime",
                              &itr_sing)
        || !((itr_sing->value).IsInt())) {
      MNS_LOG_ERROR("lastUpdateTime NOT EXIST OR NOT int");
      continue;
    }

    sgservice.lastUpdateTime = (itr_sing->value).GetInt();

    if (FetchJsonValByKey4Val(const_cast<rapidjson::Value &>(srvlist_info),
                              "fweight",
                              &itr_sing)
        || !((itr_sing->value).IsDouble())) {
      MNS_LOG_ERROR("fweight NOT EXIST OR NOT double");
      continue;
    }

    sgservice.fweight = (itr_sing->value).GetDouble();

    if (FetchJsonValByKey4Val(const_cast<rapidjson::Value &>(srvlist_info),
                              "serverType",
                              &itr_sing)
        || !((itr_sing->value).IsInt())) {
      MNS_LOG_ERROR("serverType NOT EXIST OR NOT int");
      continue;
    }

    sgservice.serverType = (itr_sing->value).GetInt();
    MNS_LOG_DEBUG("sgservice content: " << SGService2String(sgservice));

    p_vec_sgservice->push_back(sgservice);
  }

  return 0;
}

int MnsSdkCommon::Hex2Decimal(const char *begin, const char *end) {
  int iret = 0;
  char *pos = const_cast<char *>(begin);
  while (pos != end) {
    iret = (iret << 4) | hex_table[static_cast<int>(*pos)];
    pos++;
  }

  return iret;
}

void MnsSdkCommon::ParseHttpChunkData(muduo::net::Buffer *pBuf,
                                      muduo::net::HttpContext *pContext) {
  char kCRLF[] = "\r\n";
  const char *crlf = std::search(pBuf->peek(),
                                 static_cast<const char *>( pBuf->beginWrite()),
                                 kCRLF,
                                 kCRLF + 2);
  if (pBuf->beginWrite() == crlf) {
    MNS_LOG_DEBUG("NOT enough chunk length");
    return;
  }

  uint32_t udwSizeLen = static_cast<uint32_t>(crlf - pBuf->peek());
  MNS_LOG_DEBUG("udwSizeLen " << udwSizeLen);

  //transfer hex to int
  uint32_t udwLen = static_cast<uint32_t>(Hex2Decimal(pBuf->peek(), crlf));
  MNS_LOG_DEBUG("udwLen " << udwLen);

  if (0 == udwLen) {
    char kDoubleCRLF[] = "\r\n\r\n";
    const char *doubleCrlf = std::search(crlf,
                                         static_cast<const char *>( pBuf->beginWrite()),
                                         kDoubleCRLF,
                                         kDoubleCRLF + 4);
    if (pBuf->beginWrite() == doubleCrlf) {
      MNS_LOG_DEBUG("NOT enough chunk length");
      return;
    }

    MNS_LOG_DEBUG("chunk data recv completely!");

    pBuf->retrieveUntil(doubleCrlf + 4);
    pContext->receiveBody();        //END

    return;
  }

  //assume NO other data but size/CRLF/data
  if (pBuf->readableBytes()
      < static_cast<size_t>(udwSizeLen + 2 + udwLen
          + 2)) {    //size+CRLF+len+CRLF
    MNS_LOG_DEBUG("NOT enough chunk length");
    return;
  }

  muduo::net::HttpRequest &request = pContext->request();

  if (0 < request.body().size()) {        //append body
    string strTmp(request.body());
    strTmp.append(crlf + 2, udwLen);
    request.setBody(strTmp.c_str(), strTmp.c_str() + strTmp.size());
  } else {
    request.setBody(crlf + 2, crlf + 2 + udwLen);
  }

  pBuf->retrieve(udwSizeLen + 2 + udwLen + 2);

  if (pBuf->readableBytes()) {
    ParseHttpChunkData(pBuf, pContext); //recursion!!
  }
}

bool MnsSdkCommon::ParseHttpRequest(uint32_t *pudwWantLen,
                                    muduo::net::Buffer *buf,
                                    muduo::net::HttpContext *context,
                                    muduo::Timestamp receiveTime) {
  bool ok = true;
  bool hasMore = true;
  uint32_t udwBodyLen = 0;
  string strBodyLen;

  while (hasMore) {
    if (context->expectRequestLine()) {
      const char *crlf = buf->findCRLF();
      if (crlf) {
        context->request().setReceiveTime(receiveTime);
        buf->retrieveUntil(crlf + 2);
        context->receiveRequestLine();
      } else {
        hasMore = false;
      }
    } else if (context->expectHeaders()) {
      const char *crlf = buf->findCRLF();
      if (crlf) {
        const char *colon = std::find(buf->peek(), crlf, ':');
        if (colon != crlf) {
          context->request().addHeader(buf->peek(), colon, crlf);
        } else {
          //print header
          map<string, string>::const_iterator
              it = (context->request()).headers().begin();
          while (it != (context->request()).headers().end()) {
            MNS_LOG_DEBUG("key " << it->first << " value " << it->second);
            it++;
          }

          strBodyLen = (context->request()).getHeader("Content-Length");
          if (0 == strBodyLen.size()) {
            MNS_LOG_DEBUG("No body length");

            // empty line, end of header
            context->receiveAll();
            hasMore = false;
          } else {

            try {
              udwBodyLen = boost::lexical_cast<uint32_t>(strBodyLen);
            } catch (boost::bad_lexical_cast &e) {

              MNS_LOG_DEBUG("boost::bad_lexical_cast :" << e.what()
                                                        << "strBodyLen : "
                                                        << strBodyLen);

              return false;
            }

            MNS_LOG_DEBUG("udwBodyLen " << udwBodyLen);
            context->receiveHeaders();
          }
        }

        if ("\r\n\r\n" == string(crlf, 4)) { //two "/r/n" means header over
          buf->retrieveUntil(crlf + 2 + 2);

          strBodyLen = (context->request()).getHeader("Content-Length");
          if (0 == strBodyLen.size()) {
            string strTransEncode
                ((context->request()).getHeader("Transfer-Encoding"));
            if (string::npos == strTransEncode.find("chunked")) {
              MNS_LOG_DEBUG("No body length and NOT chunk data");

              // empty line, end of header
              context->receiveAll();
              hasMore = false;
            } else {
              MNS_LOG_DEBUG("chunk data, NO body length");
              context->receiveHeaders();
            }
          } else {

            try {
              udwBodyLen = boost::lexical_cast<uint32_t>(strBodyLen);
            } catch (boost::bad_lexical_cast &e) {

              MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                                        << "strBodyLen : "
                                                        << strBodyLen);

              return false;
            }

            MNS_LOG_DEBUG("udwBodyLen " << udwBodyLen);
            context->receiveHeaders();
          }
        } else {
          buf->retrieveUntil(crlf + 2);
        }
      } else {
        hasMore = false;
      }
    } else if (context->expectBody()) {
      hasMore = false;    //whatever, this is end

      strBodyLen = (context->request()).getHeader("Content-Length");
      if (0 < strBodyLen.size()) {

        try {
          udwBodyLen = boost::lexical_cast<uint32_t>(strBodyLen);
        } catch (boost::bad_lexical_cast &e) {

          MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                                    << "strBodyLen : "
                                                    << strBodyLen);

          return false;
        }

        MNS_LOG_DEBUG("udwBodyLen " << udwBodyLen);

        if (MNS_UNLIKELY(0 == udwBodyLen)) {
          MNS_LOG_WARN("Content-Length 0, maybe Bad.Request");
          context->receiveBody();
        } else {
          if (buf->readableBytes() >= udwBodyLen) {    //enough
            //string strTmp(buf->peek() + 12, buf->peek() + udwBodyLen - 17);
            //strTmp = UrlDecode(strTmp);
            (context->request()).setBody(buf->peek(),
                                         buf->peek() + udwBodyLen);
            MNS_LOG_DEBUG("body is " << (context->request()).body());

            context->receiveBody();
            buf->retrieve(udwBodyLen);
          }
        }
      } else {
        string
            strTransEncode
            ((context->request()).getHeader("Transfer-Encoding"));
        if (string::npos == strTransEncode.find("chunked")) {
          MNS_LOG_ERROR("NO content length, NOT chunk");
          return false;
        }

        ParseHttpChunkData(buf, context);
      }
    }
  }

  return ok;
}

int MnsSdkCommon::Httpgzdecompress(Byte *zdata, uLong nzdata,
                                   Byte *data, uLong *ndata) {
  int err = 0;
  z_stream d_stream = {0}; /* decompression stream */
  static char dummy_head[2] =
      {
          0x8 + 0x7 * 0x10,
          (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
      };
  d_stream.zalloc = reinterpret_cast<alloc_func>(0);
  d_stream.zfree = reinterpret_cast<free_func>(0);
  d_stream.opaque = reinterpret_cast<voidpf>(0);
  d_stream.next_in = zdata;
  d_stream.avail_in = 0;
  d_stream.next_out = data;
  if (inflateInit2(&d_stream, 47) != Z_OK) return -1;
  while (d_stream.total_out < *ndata && d_stream.total_in < nzdata) {
    d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
    if ((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) break;
    if (err != Z_OK) {
      if (err == Z_DATA_ERROR) {
        d_stream.next_in = reinterpret_cast<Byte *>(dummy_head);
        d_stream.avail_in = sizeof(dummy_head);
        if ((inflate(&d_stream, Z_NO_FLUSH)) != Z_OK) {
          return -1;
        }
      } else return -1;
    }
  }
  if (inflateEnd(&d_stream) != Z_OK) return -1;
  *ndata = d_stream.total_out;
  return 0;
}

string MnsSdkCommon::strToLower(const string &str_tmp) {
  string str_lower(str_tmp);
  transform(str_lower.begin(), str_lower.end(), str_lower.begin(), ::tolower);

  return str_lower;
}

void MnsSdkCommon::replace_all_distinct(const string &old_value,
                                        const string &new_value,
                                        string *p_str) {
  for (string::size_type pos(0); pos != string::npos;
       pos += new_value.length()) {
    if ((pos = p_str->find(old_value, pos)) != string::npos)
      p_str->replace(pos, old_value.length(), new_value);
    else break;
  }
}

int8_t MnsSdkCommon::FetchInt32FromJson(const string &strKey,
                                        rapidjson::Value &data_single,
                                        int32_t *p_i32_value) {
  rapidjson::Value::MemberIterator it;
  if (FetchJsonValByKey4Val(data_single, strKey, &it)) {
    MNS_LOG_ERROR("NO " << strKey << " exist");
    return -1;
  }

  if (!((it->value).IsInt())) {
    MNS_LOG_ERROR(strKey << " NOT int32");
    return -1;
  }

  *p_i32_value = (it->value).GetInt();
  MNS_LOG_DEBUG(strKey << " = " << *p_i32_value);

  return 0;
}

int MnsSdkCommon::FetchJsonValByKey4Doc(rapidjson::Document &reader,
                                        const string &strKey,
                                        rapidjson::Document::MemberIterator *pitr) {
  *pitr = reader.FindMember(strKey.c_str());
  if (*pitr == reader.MemberEnd()) {
    MNS_LOG_WARN("No " << strKey);
    /*SEND_LOG_ERROR("coral.interface.error",7338731, ("No " + strKey).c_str()
            ,4, -1, ("No " + strKey).c_str() );*/
    return -1;
  }

  if ("content" != strKey && CheckEmptyJsonStringVal(*pitr)) {
    MNS_LOG_WARN(strKey << " has empty string value");
    return -1;
  }

  return 0;
}

int MnsSdkCommon::FetchJsonValByKey4Val(rapidjson::Value &reader,
                                        const string &strKey,
                                        rapidjson::Value::MemberIterator *pitr) {
  *pitr = reader.FindMember(strKey.c_str());
  if (*pitr == reader.MemberEnd()) {
    MNS_LOG_WARN("No " << strKey);
    /*SEND_LOG_ERROR("coral.interface.error",7338731, ("No " + strKey).c_str()
      ,4, -1, ("No " + strKey).c_str() );*/
    return -1;
  }

  if (CheckEmptyJsonStringVal(*pitr) && "extend" != strKey) {
    MNS_LOG_WARN(strKey << " has empty string value");
    return 1;
  }

  return 0;
}

int MnsSdkCommon::CheckEmptyJsonStringVal(const rapidjson::Document::MemberIterator &itr) {
  if ((itr->value).IsString()
      && 0 == string((itr->value).GetString()).size()) {
    return 1;
  }

  return 0;
}

void MnsSdkCommon::IntranetIp(char ip[INET_ADDRSTRLEN]) {
  struct ifaddrs *ifAddrStruct = NULL;
  struct ifaddrs *ifa = NULL;
  void *tmpAddrPtr = NULL;
  int addrArrayLen = 32;
  char addrArray[addrArrayLen][INET_ADDRSTRLEN];
  getifaddrs(&ifAddrStruct);
  int index = 0;
  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr) {
      continue;
    }
    if (0 == strcmp(ifa->ifa_name, "vnic"))
      continue;
    if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
      //tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
      tmpAddrPtr =
          &(reinterpret_cast<struct sockaddr_in *>(ifa->ifa_addr))->sin_addr;
      inet_ntop(AF_INET, tmpAddrPtr, addrArray[index], INET_ADDRSTRLEN);
      if (0 == strcmp(addrArray[index], "127.0.0.1"))
        continue;
      strcpy(ip, addrArray[index]);
      if (++index >= addrArrayLen - 1)
        break;
    }
  }
  if (index > 1) {
    int idx = 0;
    while (idx < index) {
      if (NULL != strstr(addrArray[idx], "10.")
          && 0 == strcmp(addrArray[idx], strstr(addrArray[idx], "10."))) {
        strcpy(ip, addrArray[idx]);
      }
      idx++;
    }
  }
  if (ifAddrStruct != NULL)
    freeifaddrs(ifAddrStruct);
  return;
}

double MnsSdkCommon::FetchOctoWeight(const double &fweight,
                                     const double &weight) {
  return (!CheckDoubleEqual(fweight, weight)
      && !CheckDoubleEqual(fweight, static_cast<double>(0))) ? fweight : weight;
}

bool MnsSdkCommon::CheckDoubleEqual(const double &d1, const double &d2) {
  return fabs(d1 - d2) < numeric_limits<double>::epsilon();
}

void MnsSdkCommon::PackDefaultSgservice(const string &str_svr_appkey,
                                        const string &str_local_ip,
                                        const uint16_t &u16_port,
                                        meituan_mns::SGService *p_sgservice) {
  p_sgservice->__set_appkey(str_svr_appkey);
  p_sgservice->__set_version("origin");
  p_sgservice->__set_ip(str_local_ip);
  p_sgservice->__set_port(u16_port);
  p_sgservice->__set_weight(10);
  p_sgservice->__set_status(2);
  p_sgservice->__set_lastUpdateTime(static_cast<int32_t>(time(0)));
  p_sgservice->__set_fweight(10.0);
  p_sgservice->__set_serverType(0);
  p_sgservice->__set_heartbeatSupport(2);
}

bool MnsSdkCommon::CheckOverTime(const muduo::Timestamp &timestamp, const double
&d_overtime_secs, double *p_d_left_secs) {
  double
      d_time_diff_secs =
      muduo::timeDifference(muduo::Timestamp::now(), timestamp);

  MNS_LOG_DEBUG("d_time_diff_secs " << d_time_diff_secs);

  if (p_d_left_secs) {
    *p_d_left_secs =
        d_overtime_secs > d_time_diff_secs ? d_overtime_secs - d_time_diff_secs
                                           : 0;
  }

  if (d_overtime_secs < d_time_diff_secs
      || (CheckDoubleEqual(
          d_overtime_secs,
          d_time_diff_secs))) {
    MNS_LOG_WARN("overtime " << d_overtime_secs << "secs, timediff "
                             << d_time_diff_secs << " secs");

    return true;
  }

  return false;
}

}
