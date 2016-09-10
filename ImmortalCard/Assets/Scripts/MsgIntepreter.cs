using UnityEngine;
using System.Collections;
using System.IO;
using Google.Protobuf;

public class MsgIntepreter{
	/*
	static ProtocolsSerializer serializer=new ProtocolsSerializer();
	
	public static string Encode(object obj){
		return Convert.ToBase64String(EncodeBytes(obj));
	}
	
	public static byte[] EncodeBytes(object obj){
		MemoryStream stream = new MemoryStream ();
		serializer.Serialize(stream,obj);
		return stream.ToArray();
	}
	
	public static object Decode(string str,Type type){
		byte[] bytes=Convert.FromBase64String(str);//from base64
		return DecodeBytes(bytes,type);
	}
	
	public static object DecodeBytes(byte[] bytes,Type type){
		MemoryStream ms = new MemoryStream (bytes);//from bytes
		return serializer.Deserialize (ms, null, type);//from stream
	}
	*/
}

