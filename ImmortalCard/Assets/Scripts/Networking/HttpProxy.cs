using UnityEngine;
using System.Collections;
using System.IO;
using System;
using Google.Protobuf;
using Proto3;

public class HttpProxy {
	public event MsgIntepreter.ErrorHandler onError=delegate(pb_msg mid,string error){};
	public event MsgIntepreter.MessageHandler onResponse=delegate(pb_msg mid,byte[] bytes){};

	private string uri="";

	public void SetUri(string _uri){
		uri=_uri;
		Debug.Log("uri="+uri);
	}

	public void Request<T>(pb_msg mid,T msg) where T : IMessage<T>{
		string str=MsgIntepreter.Encode<T>(msg);
		Main.Instance.StartCoroutine(request(mid,msg.GetType().Name,str));
	}

	private IEnumerator request(pb_msg mid,string Name,string/*byte[]*/ bytes){
		var strmid=(int)mid;
		WWWForm form=new WWWForm();
		form.AddField("msgid",strmid);
		form.AddField("name",Name);
		form.AddField("body",bytes);
		//form.AddBinaryData("body",bytes);

		WWW www = (form==null?(new WWW(uri)):(new WWW(uri,form)));
		yield return www;
		
		if (!string.IsNullOrEmpty(www.error)){
			Debug.LogError("protocol need update: "+www.error+" "+www.url+" "+Name);
			onError.Invoke(mid,www.error);
		}else{
			try {
				var headers=www.responseHeaders;
				if(headers.ContainsKey("MSGID")){
					var msgid=(pb_msg)uint.Parse(headers["MSGID"]);
//					Debug.Log("response="+www.text);
					var content=MsgIntepreter.DecodeBytes(www.text);
					onResponse.Invoke(msgid,content);
				}else
					Debug.LogError("response: no msgid");
			} catch (Exception e) {
				Debug.LogError("response: "+e);
				yield break;
			}
		}
	}
}
