//
//  AlipayBridge.m
//  Unity-iPhone
//
//  Created by Vic Liu.
//  Copyright (c) 2016 Immorplay. All rights reserved.
//

#import "AlipayBridge.h"
#import <AlipaySDK/AlipaySDK.h>
#import <MOBFoundation/MOBFJson.h>

#if defined (__cplusplus)
extern "C" {
#endif
    
    /**
     *	@brief	Invoke payment api
     *
     *	@param 	appScheme       appScheme
     *  @param  orderString     orderString
     */
    extern void __iosAlipaySDKPay (void *appScheme, void* orderString, void* observer);
    
#if defined (__cplusplus)
}
#endif


#if defined (__cplusplus)
extern "C" {
#endif
    
    void __iosAlipaySDKPay (void *appScheme, void* orderString, void* observer)
    {
        if (!observer)return;
        
        NSString *observerStr = [NSString stringWithCString:observer encoding:NSUTF8StringEncoding];
        if(!orderString || !appScheme){
            NSString* error=@"invalid scheme or order string";
            NSLog(@"%@", error);
            UnitySendMessage([observerStr UTF8String], "_AliCallback", [error UTF8String]);
        }else{
            NSString* schemeStr=[NSString stringWithCString:appScheme encoding:NSUTF8StringEncoding];
            NSString* orderStr=[NSString stringWithCString:orderString encoding:NSUTF8StringEncoding];
            // NOTE: 调用支付结果开始支付
            [[AlipaySDK defaultService] payOrder:orderStr fromScheme:schemeStr callback:^(NSDictionary *resultDict) {
                NSLog(@"reslut = %@",resultDict);
                
                NSString *resultStr = [MOBFJson jsonStringFromObject:resultDict];
                UnitySendMessage([observerStr UTF8String], "_AliCallback", [resultStr UTF8String]);
            }];
        }
    }
    
#if defined (__cplusplus)
}
#endif
@implementation AlipayBridge

@end
