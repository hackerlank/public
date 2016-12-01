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

	public void PlayMusic(string Name="bg0"){
		play(Name);
	}
	
	public void PlaySound(string Name){
		play(Name,false);
	}

	void play(string Name,bool music=true){
		bool on=(music?Cache.MusicOn:Cache.SoundOn);
		float volume=(music?Cache.MusicVolume:Cache.MusicVolume);

		var src=Get(Name);
		if(src!=null){
			if(!src.isPlaying && on){
				src.Play();
			}
			else if(src.isPlaying && !on){
				src.Stop();
			}

			//volume
			if(src.isPlaying)
				src.volume=volume;
		}
	}

	public AudioSource Get(string name){
		if(dict.ContainsKey(name))
			return dict[name];
		else
			return null;
	}
}
