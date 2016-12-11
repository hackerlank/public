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

#ifdef WRITE_FREQ
#undef WRITE_FREQ
#define WRITE_FREQ 1000
#endif // WRITE_FREQ

std::shared_ptr<keye::logger> sLogger;
std::shared_ptr<keye::logger> sDebug;

Server* Server::sServer=nullptr;

Server::Server(size_t ios, size_t works, size_t rb_size)
:ws_service(ios, works, rb_size) {
    sServer=this;
    setup_log("charge");
}

void Server::run(const char* cfg){
    std::string wholeContent="{\"a\":\"123\"}";
    
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

    std::string pub="MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDKYal6fb2j5LAIVYiXhl7yKiwBuqhGRwW752rXgCLLf/+R4FMNdqle/Ac8JOSdmFE5Ej5DMyQ4tvb7O5LN1rav4Fdo58J8gTpiQU2E4Ryp8o6I75x6vvnfdb25Mu1z6zS9NccLVSmUWcH/y3XLcLm46mRIRUtuCWtpQbLAc28SBQIDAQAB";
    
    std::string sign = OpenapiClient::rsaSign(wholeContent, privateKey);

    keye::ini_cfg_file  config;
    if(cfg && config.load(cfg)){
        auto port=(short)(int)config.value("port");
        ws_service::run(port,"127.0.0.1");
        Debug<<"server start at "<<port<<endf;
        
        // e.g., 127.0.0.1:6379,127.0.0.1:6380,127.0.0.2:6379,127.0.0.3:6379,
        // standalone mode if only one node, else cluster mode.
        char db[128];
        sprintf(db,"%s:%d",(const char*)config.value("dbhost"),(int)config.value("dbport"));
        spdb=std::make_shared<redis_proxy>();
        if(!spdb->connect(db))
            spdb.reset();
    }else{
        Debug<<"server start error: no config file"<<endf;
    }
}

void Server::on_http(const http_parser& req,http_parser& resp){
    handler.on_http(req,resp);
}

void Server::setup_log(const char* file){
#if defined(WIN32) || defined(__APPLE__)
    sLogger=std::make_shared<logger>();
    sDebug=std::make_shared<logger>();
#else
    time_t t=time(NULL);
    tm* aTm=localtime(&t);
    char logfile[32];
    sprintf(logfile,"%s-%02d-%02d-%02d.log",file,aTm->tm_year%100,aTm->tm_mon+1,aTm->tm_mday);
    sLogger=std::make_shared<logger>(logfile);
    sprintf(logfile,"%sD-%02d-%02d-%02d.log",file,aTm->tm_year%100,aTm->tm_mon+1,aTm->tm_mday);
    sDebug=std::make_shared<logger>(logfile);
#endif
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

    Server server;
    server.run(cfg);
    while(true)usleep(1000);

	return 0;
}
