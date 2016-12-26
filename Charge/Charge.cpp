// test_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Fwd.h"

#ifdef WIN32
#include <conio.h>
#else
#include <unistd.h>
#endif

#include "openapi/openapi_client.h"

using namespace keye;

std::shared_ptr<keye::logger> Server::sLogger;
std::shared_ptr<keye::logger> Server::sDebug;
Charge* Charge::sCharge=nullptr;

string buildContent(const StringMap &contentPairs);

Charge::Charge()
:Server("charge") {
    sCharge=this;
}

int Charge::quantity(float money){
    auto p=money/2.5f;
    int q=0;
    switch ((int)money) {
        case 50:
            q=2;    break;
        case 100:
            q=5;    break;
        case 500:
            q=50;   break;
        case 1000:
            q=200;   break;
        default:
            break;
    }
    return q+(int)p;
}

void Charge::order(const proto3::MsgCPOrder& imsg,proto3::MsgPCOrder& omsg){
    string appId = "2016121004101224";
    std::string privateKey="-----BEGIN RSA PRIVATE KEY-----\n"
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

    string privateKeyPKCS8="-----BEGIN PRIVATE KEY-----\n"
    "MIICdgIBADANBgkqhkiG9w0BAQEFAASCAmAwggJcAgEAAoGBAMphqXp9vaPksAhV\n"
    "iJeGXvIqLAG6qEZHBbvnateAIst//5HgUw12qV78Bzwk5J2YUTkSPkMzJDi29vs7\n"
    "ks3Wtq/gV2jnwnyBOmJBTYThHKnyjojvnHq++d91vbky7XPrNL01xwtVKZRZwf/L\n"
    "dctwubjqZEhFS24Ja2lBssBzbxIFAgMBAAECgYEAmlJ6X5M+Zplduh0u+QxXniWW\n"
    "RQNGicBimPRQPl/RKIhEI8KA0kVnxaw9ABnJ0npxhSlqHOgEeNLb5pt4yEkwJ/N/\n"
    "KwQ2DwZCPMf6/w4Gx1ifQq8o+BqhLsRkSEibWa5hzB/+B4tzNbPKgI2K5oLz/UUl\n"
    "YfFssPJ1Yh6eCS8tlgkCQQD+shsVU/M2Bbm9535/rGOg9YJHgZkB9+ixOm/g5tZM\n"
    "f9Sa/61arunK6xrrOhwnM8c2BxjWRJjH53eRzo36kQaPAkEAy2r5htDVefV68OGZ\n"
    "SHtXxQS9GnHT0SS1fJCU1YwHZOJ8y0okDdQnPQzIX4i9UvTjwT0xnKAZ6goC8LE6\n"
    "SHmIKwJAIE2GkdXr6tYfEK7jpF3A5RE+TMNrHIzMvm2i8utUEq/Dz0TOP8ofbNUU\n"
    "vPDJ0U8H3RpZVBOvLOt2hs+jwjvSkQJAIG6o5B7NuokyFc/SUaUgxE/SGrJFVFYm\n"
    "/XpL0JRn5WTbAsqKXw4RR4GPPPQfQ2e6HGg2TcOU/iSkfV20bjO6SwJAIvRut8HG\n"
    "xuI2o8FL3tM4CCNzUJtdDRn1TSrTunwLG8oBJNSNw7cXcnupOXPh32ASdomdGZDj\n"
    "sRQeq4LYAssfVw==\n"
    "-----END PRIVATE KEY-----";

    string aliPubKey = "-----BEGIN PUBLIC KEY-----\n"
    "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDKYal6fb2j5LAIVYiXhl7yKiwB\n"
    "uqhGRwW752rXgCLLf/+R4FMNdqle/Ac8JOSdmFE5Ej5DMyQ4tvb7O5LN1rav4Fdo\n"
    "58J8gTpiQU2E4Ryp8o6I75x6vvnfdb25Mu1z6zS9NccLVSmUWcH/y3XLcLm46mRI\n"
    "RUtuCWtpQbLAc28SBQIDAQAB\n"
    "-----END PUBLIC KEY-----";

    auto& pKey=imsg.pkcs8()?privateKeyPKCS8:privateKey;
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
    string method = "alipay.trade.app.pay";
    JsonMap contentMap;
    contentMap.insert(JsonMap::value_type(JsonType("out_trade_no"), JsonType(imsg.uid())));
    contentMap.insert(JsonMap::value_type(JsonType("total_amount"), JsonType(0.01)));
    contentMap.insert(JsonMap::value_type(JsonType("subject"), JsonType("ImmorCard")));
    contentMap.insert(JsonMap::value_type(JsonType("product_code"), JsonType("QUICK_MSECURITY_PAY")));


    /** ++++++++++++++++++++++++++++++++++++++++++++++++ **/
    
    /** ++++++++++++++++++++++++++++++++++++++++++++++++ **/
    /** 网关扩展参数，例如商户需要回传notify_url等，可以在extendParamMap中传入 **/
    /** 这是一个可选项，如不需要，可不传 **/
    StringMap extendParamMap;
    extendParamMap.insert(StringMap::value_type("notify_url", "http://120.77.146.47:8880"));
    
    /** ++++++++++++++++++++++++++++++++++++++++++++++++ **/

    string content = JsonUtil::objectToString(JsonType(contentMap));
    
    time_t t = time(0);
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %X", localtime(&t));

    StringMap requestPairs;
    requestPairs.insert(StringMap::value_type(OpenapiClient::KEY_APP_ID, appId));
    requestPairs.insert(StringMap::value_type(OpenapiClient::KEY_BIZ_CONTENT, content));
    requestPairs.insert(StringMap::value_type(OpenapiClient::KEY_CHARSET, OpenapiClient::default_charset));
    requestPairs.insert(StringMap::value_type(OpenapiClient::KEY_METHOD, method));
    requestPairs.insert(StringMap::value_type(OpenapiClient::KEY_SIGN_TYPE, OpenapiClient::default_sign_type));
    requestPairs.insert(StringMap::value_type(OpenapiClient::KEY_TIMESTAMP, tmp));
    requestPairs.insert(StringMap::value_type(OpenapiClient::KEY_VERSION, OpenapiClient::default_version));
    
    // 追加外部传入的网关的补充参数，如notify_url等
    for (StringMap::const_iterator iter = extendParamMap.begin(); iter != extendParamMap.end(); ++iter) {
        requestPairs.insert(StringMap::value_type(iter->first, iter->second));
    }
    
    //sign
    string wholeContent=buildContent(requestPairs);
    string sign = OpenapiClient::rsaSign(wholeContent, pKey);
    requestPairs.insert(StringMap::value_type(OpenapiClient::KEY_SIGN, sign));

    //encoding
    string requestEntity;
    CURL *curl;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if(curl){
        string item;
        for (ParamsMap::const_iterator iter = requestPairs.begin(); iter != requestPairs.end(); ++iter) {
            const char *key = iter->first.c_str();
            char *encodedKey = curl_easy_escape(curl, key, (int)strlen(key));
            if (encodedKey) {
                item = encodedKey;
            }
            item += "=";
            const char *value = iter->second.c_str();
            char *encodedValue = curl_easy_escape(curl, value, (int)strlen(value));
            if (encodedValue) {
                item += encodedValue;
            }
            if (!requestEntity.empty()) {
                requestEntity.push_back('&');
            }
            requestEntity.append(item);
            item.clear();
            if (encodedKey) {
                curl_free(encodedKey);
            }
            if (encodedValue) {
                curl_free(encodedValue);
            }
        }
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    
    //response order string
    std::string appScheme("immorpayScheme");
    omsg.set_appscheme(appScheme);
    omsg.set_orderstring(requestEntity);
}

void Charge::on_http(const http_parser& req,const std::function<void(const http_parser&)> func){
    handler.on_http(req,func);
}

bool Charge::on_timer(svc_handler&, size_t id, size_t milliseconds) {
    switch (id) {
        case TIMER::TIMER_SEC:
            break;
        case TIMER::TIMER_MIN:
            break;
        case TIMER::TIMER_HOUR:{
            time_t t=time(NULL);
            tm* aTm=localtime(&t);
            if(aTm->tm_hour==0)
                setup_log(name.c_str());

            decltype(t) thresh=60*60;  //1 hour
            for(auto i=sessions.begin();i!=sessions.end();){
                if(i->second-t>thresh){
                    i=sessions.erase(i);
                }else
                    ++i;
            }
            break;
        }
        case TIMER::TIMER_DAY:{
            break;
        }
        default:
            break;
    }
    return true;
}

/**
 *
 * STL map default sort order by key
 *
 * STL map 默认按照key升序排列
 * 这里要注意如果使用的map必须按key升序排列
 *
 */
string buildContent(const StringMap &contentPairs) {
    
    string content;
    for (StringMap::const_iterator iter = contentPairs.begin();
         iter != contentPairs.end(); ++iter) {
        if (!content.empty()) {
            content.push_back('&');
        }
        content.append(iter->first);
        content.push_back('=');
        content.append(iter->second);
    }
    return content;
}

int main(int argc, char* argv[]) {
    const char* cfg="charge.cfg";
    for(auto i=1;i<argc;++i){
        auto arg=argv[i];
        if(strlen(arg)>3&&arg[0]=='-')switch(arg[1]){
            case 'f':{
                cfg=&arg[2];
                break;
            }
            default:
                break;
        }
    }

    Charge server;
    server.run(cfg);
    server.set_timer(TIMER::TIMER_HOUR, 1000*60*60);
    server.set_timer(TIMER::TIMER_DAY, 1000*60*60*24);
    while(true)msleep(1000000);

	return 0;
}
