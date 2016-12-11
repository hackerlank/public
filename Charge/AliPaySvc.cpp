#include "stdafx.h"
#include "Fwd.h"

#include <iostream>
#include "stdlib.h"

#define _DEBUG

#include "openapi/openapi_client.h"

using namespace std;

bool AliPaySvc::on_http(const http_parser& req,http_parser& resp){
    //auto head=resp.header("code");
    auto body=resp.body();
    Debug<<"alipay notify: body="<<body<<endf;
    return true;
}

/** ++++++++++++++++++++++++++++++++++++++++++++++++ **/
/** 此处替换为开发者在支付宝开放平台申请的应用ID **/
string appId = "2016121004101224";

/** 此处替换为开发者使用openssl生成的rsa私钥 **/
string pKey = "-----BEGIN RSA PRIVATE KEY-----\n"
"MIICXAIBAAKBgQDKYal6fb2j5LAIVYiXhl7yKiwBuqhGRwW752rXgCLLf/+R4FMN\n"
"dqle/Ac8JOSdmFE5Ej5DMyQ4tvb7O5LN1rav4Fdo58J8gTpiQU2E4Ryp8o6I75x6\n"
"vvnfdb25Mu1z6zS9NccLVSmUWcH/y3XLcLm46mRIRUtuCWtpQbLAc28SBQIDAQAB\n"
"AoGBAJpSel+TPmaZXbodLvkMV54llkUDRonAYpj0UD5f0SiIRCPCgNJFZ8WsPQAZ\n"
"ydJ6cYUpahzoBHjS2+abeMhJMCfzfysENg8GQjzH+v8OBsdYn0KvKPgaoS7EZEhI\n"
"m1muYcwf/geLczWzyoCNiuaC8/1FJWHxbLDydWIengkvLZYJAkEA/rIbFVPzNgW5\n"
"ved+f6xjoPWCR4GZAffosTpv4ObWTH/Umv+tWq7pyusa6zocJzPHNgcY1kSYx+d3\n"
"kc6N+pEGjwJBAMtq+YbQ1Xn1evDhmUh7V8UEvRpx09EktXyQlNWMB2TifMtKJA3U\n"
"Jz0MyF+IvVL048E9MZygGeoKAvCxOkh5iCsCQCBNhpHV6+rWHxCu46RdwOURPkzD\n"
"axyMzL5tovLrVBKvw89Ezj/KH2zVFLzwydFPB90aWVQTryzrdobPo8I70pECQCBu\n"
"qOQezbqJMhXP0lGlIMRP0hqyRVRWJv16S9CUZ+Vk2wLKil8OEUeBjzz0H0Nnuhxo\n"
"Nk3DlP4kpH1dtG4zuksCQCL0brfBxsbiNqPBS97TOAgjc1CbXQ0Z9U0q07p8CxvK\n"
"ASTUjcO3F3J7qTlz4d9gEnaJnRmQ47EUHquC2ALLH1c=\n"
"-----END RSA PRIVATE KEY-----";

/** 支付宝公钥，用来验证支付宝返回请求的合法性 **/
string aliPubKey = "-----BEGIN PUBLIC KEY-----\n"
"MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDKYal6fb2j5LAIVYiXhl7yKiwB\n"
"uqhGRwW752rXgCLLf/+R4FMNdqle/Ac8JOSdmFE5Ej5DMyQ4tvb7O5LN1rav4Fdo\n"
"58J8gTpiQU2E4Ryp8o6I75x6vvnfdb25Mu1z6zS9NccLVSmUWcH/y3XLcLm46mRI\n"
"RUtuCWtpQbLAc28SBQIDAQAB\n"
"-----END PUBLIC KEY-----";

/** 注：appid，私钥，支付宝公钥等信息建议不要写死在代码中 **/
/** 这些信息应以配置等方式保存，此处写在代码中只是为了示例的简便 **/
/** ++++++++++++++++++++++++++++++++++++++++++++++++ **/


/** some examples **/
JsonMap getPrecreateContent();

int test(int argc, char *argv[])
{

    /** 实例化OpenapiClient工具类 **/
    OpenapiClient openapiClient(appId,
                                pKey,
                                OpenapiClient::default_url,
                                OpenapiClient::default_charset,
                                aliPubKey);

    /** ++++++++++++++++++++++++++++++++++++++++++++++++ **/
    /** 各个具体业务接口参数组装模式具体参看Openapi官方文档 **/
    /** https://doc.open.alipay.com/ **/
    // demo1:当面付预下单示例
    string method = "alipay.trade.precreate";
    JsonMap contentMap = getPrecreateContent();

    /** ++++++++++++++++++++++++++++++++++++++++++++++++ **/

    /** ++++++++++++++++++++++++++++++++++++++++++++++++ **/
    /** 网关扩展参数，例如商户需要回传notify_url等，可以在extendParamMap中传入 **/
    /** 这是一个可选项，如不需要，可不传 **/
    /* StringMap extendParamMap;
    extendParamMap.insert(StringMap::value_type("notify_url", "http://api.test.alipay.net/atinterface/receive_notify.htm"));
    */

    /** ++++++++++++++++++++++++++++++++++++++++++++++++ **/

    /** 调用Openapi网关 **/
    JsonMap respMap;
    respMap = openapiClient.invoke(method, contentMap);
    /* 如果有扩展参数，则按如下方式传入
    respMap = openapiClient.invoke(method, contentMap, extendParamMap);
    */

    /** 解析支付宝返回报文 **/
    JsonMap::const_iterator iter = respMap.find("code");
    if (iter != respMap.end()) {
        string respCode = iter->second.toString();
        DebugLog("code:%s", respCode.c_str());
    } else {
        DebugLog("cannot get code from response");
    }

    iter = respMap.find("msg");
    if (iter != respMap.end()) {
        string respMsg = iter->second.toString();
        DebugLog("msg:%s", respMsg.c_str());
    } else {
        DebugLog("cannot get msg from response");
    }

    system("pause");
    return 0;
}

/**
 * 组装支付宝预下单业务请求
 */
JsonMap getPrecreateContent() {

    JsonMap contentMap;
    contentMap.insert(JsonMap::value_type(JsonType("out_trade_no"), JsonType("20160606121212")));
    contentMap.insert(JsonMap::value_type(JsonType("total_amount"), JsonType(0.01)));
    contentMap.insert(JsonMap::value_type(JsonType("subject"), JsonType("好东西")));

    return contentMap;
}
