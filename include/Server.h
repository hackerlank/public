//
//  Server.h
//  Server
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef Server_h
#define Server_h

class Server :public keye::ws_service {
public:
    Server(const char* name,size_t ios = 1, size_t works = 1, size_t rb_size = 510)
    :ws_service(ios, works, rb_size)
    ,tpool(100){
        setup_log(name?name:"server");
    }
    
    virtual ~Server(){}
    
    virtual bool    run(const char* cfg=nullptr){
        if(cfg && config.load(cfg)){
            auto port=(short)(int)config.value("port");
            ws_service::run(port,"0.0.0.0");
            Debug<<"server start at "<<port<<endf;
            
            //load game config
            keye::csv_file gameCfg;
            if(gameCfg.load("games.csv")){
                Debug<<"----game config\n";
                for(size_t r=0,ii=gameCfg.rows();r!=ii;++r){
                    auto& game=*gameConfig.Add();
                    game.set_rule((int)         gameCfg.value(r,0));
                    game.set_available((int)    gameCfg.value(r,1));
                    game.set_price((int)        gameCfg.value(r,2));
                    game.set_rounds((int)       gameCfg.value(r,3));
                    game.set_free((int)         gameCfg.value(r,4));
                    game.set_event((int)        gameCfg.value(r,5));
                    game.set_name((const char*) gameCfg.value(r,6));
                    game.set_desc((const char*) gameCfg.value(r,7));

                    Debug<<"rule="<<game.rule()<<
                    ",aval="<<game.available()<<
                    ",price="<<game.price()<<
                    ",name="<<game.name()<<
                    ",desc="<<game.desc()<<endl;
                }
                Debug<<endf;
            }

            // e.g., 127.0.0.1:6379,127.0.0.1:6380,127.0.0.2:6379,127.0.0.3:6379,
            // standalone mode if only one node, else cluster mode.
            char db[128];
            sprintf(db,"%s:%d",(const char*)config.value("dbhost"),(int)config.value("dbport"));
            spdb=std::make_shared<redis_proxy>();
            int retry=3;
            int retryTime=3000;
            for(auto i=retry;i;--i){
                if(spdb->connect(db))
                    return true;
                
                msleep(retryTime);
            }
            spdb.reset();
            Debug<<"server start error: db not connect"<<endf;
        }else{
            Debug<<"server start error: no config file"<<endf;
        }
        return false;
    }

    void    setup_log(const char* file){
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
    
    keye::ini_cfg_file          config;
    google::protobuf::RepeatedPtrField<proto3::game_t>    gameConfig;
    std::shared_ptr<vic_proxy>  spdb;
    keye::scheduler             tpool;
    std::map<unsigned long,long>    sessions; //[session,timestamp]
};

inline unsigned long genSession(){
    auto tt=keye::ticker();
    srand((int)tt);
    auto r=((unsigned long)rand())<<8;
    auto a=(tt>>0);
    auto b=(tt>>16);
    auto c=(tt>>32);
    auto d=(tt>>48);
    return r + (a<<48) + (d<<32) + (b<<16) + c;
}

inline void split_line(std::vector<std::string>& o,std::string& line,char c){
    std::string comma;
    comma.push_back(c);
    if(!line.empty()&&line.back()!=c)line+=comma;
    while(true){
        auto i=line.find(comma);
        if(i==std::string::npos)break;
        auto str=line.substr(0,i);
        o.push_back(str);
        line=line.substr(++i);
    }
}

inline proto3::pb_msg extractBody(std::string& body,const char* inbody){
    if(inbody){
        std::vector<std::string> params;
        std::string buf(inbody);
        split_line(params,buf,'&');
        
        std::map<std::string,std::string> kvs;
        for(auto& p:params){
            std::vector<std::string> ss;
            split_line(ss,p,'=');
            if(ss.size()>1)
                kvs[ss[0]]=ss[1];
        }
        
        if(kvs.count("body"))
            body=kvs["body"];
        if(kvs.count("msgid"))
            return (proto3::pb_msg)atoi(kvs["msgid"].c_str());
    }
    return proto3::pb_msg::MSG_RAW;
}

#endif /* Server_h */
