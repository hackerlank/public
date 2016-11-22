using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class StoreGame{
	public int		gameType;
	public int		gameId;
	public int		robots;

	public override string ToString(){
		var dict=new Dictionary<string,string>();
		dict["type"]=gameType.ToString();
		dict["id"]=gameId.ToString();
		dict["robots"]=robots.ToString();
		var str=MiniJSON.jsonEncode(dict);
		return str;
	}

	public void FromString(string str){
		var dict=(Hashtable)MiniJSON.jsonDecode(str);
		if(dict!=null){
			gameType=int.Parse(dict["type"] as string);
			gameId=int.Parse(dict["id"] as string);
			robots=int.Parse(dict["robots"] as string);
		}
	}
}
