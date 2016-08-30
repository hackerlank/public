//
//  robot.h
//  robot
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef robot_h
#define robot_h

class robot{
public:
                    robot();
    MsgHandler      handler;

    class login_client :public keye::ws_client{
    public:
        login_client();
        virtual void	on_open(keye::svc_handler& sh);
        virtual void	on_read(keye::svc_handler& sh, void* buf, size_t sz);
        virtual bool	on_timer(keye::svc_handler& sh, size_t id, size_t milliseconds);
        //test
        void login();
        
        std::shared_ptr<keye::svc_handler> spsh;
    };

    class lobby_client :public keye::ws_client{
    public:
        lobby_client();
        virtual void	on_open(keye::svc_handler& sh);
        virtual void	on_read(keye::svc_handler& sh, void* buf, size_t sz);
        virtual bool	on_timer(keye::svc_handler& sh, size_t id, size_t milliseconds);
        //test
        void login();
        
        std::shared_ptr<keye::svc_handler> spsh;
    };

    class node_client :public keye::ws_client{
    public:
        node_client();
        virtual void	on_open(keye::svc_handler& sh);
        virtual void	on_read(keye::svc_handler& sh, void* buf, size_t sz);
        virtual bool	on_timer(keye::svc_handler& sh, size_t id, size_t milliseconds);
        //test
        void login();

        std::shared_ptr<keye::svc_handler> spsh;
    };
    
    class http_client :public keye::http_client{
    public:
        http_client();
        virtual void	on_read(keye::svc_handler& sh, void* buf, size_t sz);
    };
    
    login_client    login;
    lobby_client    lobby;
    node_client     node;
    http_client     http;
    
    proto3::user_t  user;
    
    static robot* sRobot;
};

#endif /* robot_h */
