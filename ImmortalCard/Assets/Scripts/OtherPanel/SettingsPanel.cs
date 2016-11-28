using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class SettingsPanel : MonoBehaviour {
	public Slider	sound;
	public Slider	music;
	public Toggle	soundOn;
	public Toggle	musicOn;

	public const string PrefsKey_SoundVolume	="PrefsKey_SoundVolume";
	public const string PrefsKey_MusicVolume	="PrefsKey_MusicVolume";
	public const string PrefsKey_SoundOn		="PrefsKey_SoundOn";
	public const string PrefsKey_MusicOn		="PrefsKey_MusicOn";

	public static void Load(){
		Configs.SoundVolume=PlayerPrefs.GetFloat("PrefsKey_SoundVolume",0.5f);
		Configs.MusicVolume=PlayerPrefs.GetFloat("PrefsKey_MusicVolume",0.5f);
		Configs.SoundOn=(1==PlayerPrefs.GetInt("PrefsKey_SoundOn",1));
		Configs.MusicOn=(1==PlayerPrefs.GetInt("PrefsKey_MusicOn",1));

		Audio.Instance.PlayMusic();
	}
	
	public static void Save(){
		PlayerPrefs.SetFloat("PrefsKey_SoundVolume",Configs.SoundVolume);
		PlayerPrefs.SetFloat("PrefsKey_MusicVolume",Configs.MusicVolume);
		PlayerPrefs.SetInt("PrefsKey_SoundOn",Configs.SoundOn?1:0);
		PlayerPrefs.SetInt("PrefsKey_MusicOn",Configs.MusicOn?1:0);

		Audio.Instance.PlayMusic();
	}

	void Start(){
		soundOn.isOn=Configs.SoundOn;
		musicOn.isOn=Configs.MusicOn;
		sound.value=Configs.SoundVolume;
		music.value=Configs.MusicVolume;
	}

	public void OnSoundVolume(float x){
		var v=sound.value;
		Configs.SoundVolume=v;
	}
	
	public void OnMusicVolume(float x){
		var v=music.value;
		Configs.MusicVolume=v;
	}
	
	public void OnSoundOn(bool x){
		var v=soundOn.isOn;
		Configs.SoundOn=v;
	}

	public void OnMusicOn(bool x){
		var v=musicOn.isOn;
		Configs.MusicOn=v;
	}

	public void OnClose(){
		Save();
		Destroy(gameObject);
	}
}
