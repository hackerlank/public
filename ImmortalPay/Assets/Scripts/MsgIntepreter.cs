using UnityEngine;
using System.Collections;
using System.IO;
using System;
using Google.Protobuf;

public class MsgIntepreter{

	public static byte[] EncodeBytes<T>(T msg)where T:IMessage<T>{
		MemoryStream ms=new MemoryStream();
		Google.Protobuf.CodedOutputStream co=new Google.Protobuf.CodedOutputStream(ms);
		msg.WriteTo(co);
		co.Flush();
		return ms.ToArray();
	}

	public static string Encode<T>(T msg)where T:IMessage<T>{
		var bytes=EncodeBytes<T>(msg);
		string str=Convert.ToBase64String(bytes);
		Debug.Log("----encode="+str);
		return str;
	}
	
	public static byte[] DecodeBytes(string str){
		return Convert.FromBase64String(str);
	}
	/*
	public static T Decode<T>(string str)where T:IMessage<T>{
		//Proto3.MsgCNEnter msg2 = Proto3.MsgCNEnter.Parser.ParseFrom(ms.ToArray());
		byte[] bytes=Convert.FromBase64String(str);
		MessageParser<T> parser = new MessageParser<T>(() => new T());
		return parser.ParseFrom(bytes);
	}
	*/
}

