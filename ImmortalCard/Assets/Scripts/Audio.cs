using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Audio : MonoBehaviour {
	public const int CATEGORY_MAN	=0;
	public const int CATEGORY_WOMAN	=1;

	public static Audio	Instance=null;

	Dictionary<string,AudioSource> dict;

	void Awake(){
		Instance=this;
	}

	void Start () {
		dict=new Dictionary<string, AudioSource>();
		var srcs=GetComponents<AudioSource>();
		foreach(AudioSource src in srcs){
			dict[src.clip.name]=src;
		}
	}

	public AudioSource Get(string name){
		if(dict.ContainsKey(name))
			return dict[name];
		else
			return null;
	}
}
