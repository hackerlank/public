using UnityEngine;
using System.Collections;
using System.IO;
using System;
using Google.Protobuf;
using Proto3;

public class HttpProxy {
	private string uri="";
	public event MsgHandler.MessageHandler onResponse=delegate(uint mid,byte[] bytes){};

	public void SetUri(string _uri){
		uri=_uri;
	}

	public void Request<T>(int mid,T msg) where T : IMessage<T>{
		string str=MsgIntepreter.Encode<T>(msg);
		Main.Instance.StartCoroutine(request(mid,msg.GetType().Name,str));
	}

	private IEnumerator request(int mid,string Name,string/*byte[]*/ bytes){
		WWWForm form=new WWWForm();
		form.AddField("msgid",mid);
		form.AddField("name",Name);
		form.AddField("body",bytes);
		//form.AddBinaryData("body",bytes);

		WWW www = (form==null?(new WWW(uri)):(new WWW(uri,form)));
		yield return www;
		
		if (!string.IsNullOrEmpty(www.error)){
			Debug.LogError("response : "+www.error+" "+www.url+" "+Name);
		}else{
			try {
				var headers=www.responseHeaders;
				if(headers.ContainsKey("MSGID")){
					uint msgid=uint.Parse(headers["MSGID"]);
					Debug.Log("response="+www.text);
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
