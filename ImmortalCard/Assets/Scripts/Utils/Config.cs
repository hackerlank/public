using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using Proto3;

public class Config{
	public static string file="Config/config";

	public static string version="1.0.0";
	public static string build	="100";

	public static string uri="http://127.0.0.1:8800";
	public static string ws="ws://127.0.0.1:8820";
	public static string updateUri="";	//bundles update uri
	public static string update="0";	//force update
	public static string limited="0";	//feature limited: wechat,payment etc.
	public static string opsInterval="1";

	public static string appStore=		"http://fir.im/immorcard";
	public static string androidMarket=	"http://fir.im/immorcarda";
	public static string payAppStore=		"http://fir.im/immorpay";
	public static string payAndroidMarket=	"http://fir.im/immorpaya";

	public static Dictionary<pb_enum,Dictionary<pb_enum,ArrayList>> options;
	//ShareSDK app id
	public static string modId="180127d1c7541";

	public static float OpsInterval=1f;
	public static int invalidCard=-1;

	public static void Load(string text){
		if(text!=null){
			var dict=Utils.ParseIni(text);
			foreach(var kv in dict){
				System.Type type = typeof(Config);
				string field = kv.Key;
				FieldInfo fi = type.GetField(field);
				if(fi==null)fi=type.GetField(field,BindingFlags.Instance|BindingFlags.NonPublic);
				if(fi==null)continue;
				if(fi.FieldType!=typeof(string))continue;
				fi.SetValue(null,kv.Value);
			}
		}

		OpsInterval=float.Parse(opsInterval);
	}

	public static void LoadOptions(string json){
		var array=MiniJSON.jsonDecode(json) as ArrayList;
		if(null==array){
			Debug.LogError("load options failed");
			return;
		}

		options=new Dictionary<pb_enum, Dictionary<pb_enum, ArrayList>>();
		foreach(var arr in array){
			var hRule=arr as Hashtable;
			if(null!=hRule && hRule.Count>0){
				//rule
				var rule=Convert.ToInt32(hRule["rule"]);
				var dictCategory=new Dictionary<pb_enum, ArrayList>();
				options[(pb_enum)rule]=dictCategory;

				var aCategory=hRule["category"] as ArrayList;
				if(aCategory !=null && aCategory.Count>0){
					foreach(var arrcat in aCategory){
						var hCategory=arrcat as Hashtable;
						if(hCategory!=null && hCategory.Count>0){
							//category
							var category=Convert.ToInt32(hCategory["category"]);

							var aOptions=hCategory["options"] as ArrayList;
							if(aOptions !=null && aOptions.Count>0){
								//options
								foreach(var arropt in aOptions){
									var optionsList=new ArrayList();
									dictCategory[(pb_enum)category]=optionsList;

									var hopt=arropt as Hashtable;
									if(hopt!=null && hopt.Count>0){
										optionsList.Add(hopt);
										/*
										var aOption=hopt["option"] as ArrayList;
										if(aOption !=null){
											//group
											foreach(var option in aOption){
												var hOption=option as Hashtable;
												if(hOption!=null && hOption.Count>0){
													foreach(var key in hOption.Keys){
														//key value
														var val=hOption[key];
														int x=0;
													}
												}
											}
										}else{
											//option
											foreach(var key in hopt.Keys){
												//key value
												var val=hopt[key];
												int x=0;
											}
										}//option
										*/
									}//hopt
								}//arropt
							}//aOptions
						}//hCategory
					}//arrcat
				}//aCatetory
			}//hRule
		}//arr
	}

	public static Dictionary<pb_enum,List<game_t>> games;
	public static void parseLobby(MsgLCLobby msg){
		games=new Dictionary<pb_enum, List<game_t>>();
		foreach(game_t game in msg.Lobby.Games){
			var rule=(pb_enum)(game.Rule/100);
			//var cate=(pb_enum)(game.Rule%100);
			if(!games.ContainsKey(rule)){
				games.Add(rule,new List<game_t>());
			}
			games[rule].Add(game);
		}
	}
}

