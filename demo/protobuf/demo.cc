#include <cstdio>
#include <iostream>
#include <string>

#include "bike.pb.h"

using namespace std;
using namespace bike;

// int main() {
//   string data;  // 存储序列化的消息
//   // 从客户端发送请求
//   {
//     mobile_request mr;
//     mr.set_mobile("18266666666");
//     mr.SerializeToString(&data);
//     cout << "序列化之后的数据（二进制）" << data << "data的长度为："
//          << data.length() << endl;
//     cout << hex << (int)*((char *)data.c_str()) << endl;
//     cout << hex << (int)*((char *)data.c_str() + 1) << endl;
//     // 客户端发送data，类似于 send(sockfd, data.c_str(), data.length());
//   }
//   // 服务器端接受请求
//   {
//     // receive(sockfd, data, ...);
//     mobile_request mr;
//     mr.ParseFromString(data);
//     cout << "客户端手机号码：" << mr.mobile() << endl;
//   }
//   return 0;
// }

int main() {
  string data;  // 存储序列化的消息
  // 从客户端发送请求
  {
    list_account_records_response larr;
    larr.set_code(200);
    larr.set_desc("ok");

    for (int i = 0; i < 5; i++) {
      list_account_records_response_account_record *ar = larr.add_records();
      ar->set_type(0);
      ar->set_limit(i * 100);
      ar->set_timestamp(time(NULL));
    }
    printf("records size before: %d\n", larr.records_size());
    larr.SerializeToString(&data);
  }
  // 服务器端接受请求
  {
    list_account_records_response larr;
    larr.ParseFromString(data);
    printf("records size after: %d\n", larr.records_size());
    printf("code: %d\n", larr.code());
    for (int i = 0; i < larr.records_size(); i++) {
      const list_account_records_response_account_record &ar = larr.records(i);
      printf("limit: %d\n", ar.limit());
    }
  }
  return 0;
}