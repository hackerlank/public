using UnityEngine;
using System.Collections;
using System.IO;
using System;
using Google.Protobuf;
using Proto3;

public class HttpProxy {
	private string uri="";

	public void SetUri(string _uri){
		uri=_uri;
	}

	public void Request<T>(int mid,Google.Protobuf.IMessage<T> msg) where T : IMessage<T>{
		byte[] bytes;
		MemoryStream ms=new MemoryStream();
		Google.Protobuf.CodedOutputStream co=new Google.Protobuf.CodedOutputStream(ms);
		msg.WriteTo(co);
		bytes=ms.ToArray();
		StreamReader sr=new StreamReader(ms);
		//string str = Encoding.ASCII.GetString(ms3.ToArray());

		Main.Instance.StartCoroutine(request(mid,msg.GetType().Name,sr.ReadToEnd()));
	}

	private IEnumerator request(int mid,string Name,string/*byte[]*/ bytes){
		WWWForm form=new WWWForm();
		form.AddField("msgid",mid);
		form.AddField("name",Name);
		form.AddField("body",bytes);
		//form.AddBinaryData("body",bytes);

		WWW www = (form==null?(new WWW(uri)):(new WWW(uri,form)));
		yield return www;
		
		string data_rep="";
		object error=null;
		if (!string.IsNullOrEmpty(www.error)){
			Debug.LogError("request : "+www.error+" "+www.url+" "+Name);
		}else{
			object msg_rep = null;
			bytes = null;
			try {
				string text=www.text;
				byte[] bbytes=www.bytes;
				OnResponse(mid,Name,bbytes);
			} catch (Exception e) {
				Debug.LogError("request: "+e);
				yield break;
			}
		}
	}
	
	public void OnResponse(int mid,string Name,byte[] bytes){
		switch(mid){
		case 2002:
			MsgCNEnter msg2 = MsgCNEnter.Parser.ParseFrom(bytes);
			break;
		default:
			break;
		}
		Debug.Log(mid.ToString());
	}
}
