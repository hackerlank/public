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

		SettingsPanel.Load();
	}

	public void PlayMusic(string musicName="bg0"){
		var bg=Get(musicName);
		if(bg!=null){
			if(!bg.isPlaying && Configs.MusicOn){
				bg.Play();
			}
			else if(bg.isPlaying && !Configs.MusicOn){
				bg.Stop();
			}
		}
	}

	public AudioSource Get(string name){
		if(dict.ContainsKey(name))
			return dict[name];
		else
			return null;
	}
}
