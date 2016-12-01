using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class SettingsPanel : MonoBehaviour {
	public Slider	sound;
	public Slider	music;
	public Toggle	soundOn;
	public Toggle	musicOn;
	public Text		version;

	public const string PrefsKey_SoundVolume	="PrefsKey_SoundVolume";
	public const string PrefsKey_MusicVolume	="PrefsKey_MusicVolume";
	public const string PrefsKey_SoundOn		="PrefsKey_SoundOn";
	public const string PrefsKey_MusicOn		="PrefsKey_MusicOn";

	public static void Load(){
		Cache.SoundVolume=PlayerPrefs.GetFloat("PrefsKey_SoundVolume",0.5f);
		Cache.MusicVolume=PlayerPrefs.GetFloat("PrefsKey_MusicVolume",0.5f);
		Cache.SoundOn=(1==PlayerPrefs.GetInt("PrefsKey_SoundOn",1));
		Cache.MusicOn=(1==PlayerPrefs.GetInt("PrefsKey_MusicOn",1));

		Audio.Instance.PlayMusic();
	}
	
	public static void Save(){
		PlayerPrefs.SetFloat("PrefsKey_SoundVolume",Cache.SoundVolume);
		PlayerPrefs.SetFloat("PrefsKey_MusicVolume",Cache.MusicVolume);
		PlayerPrefs.SetInt("PrefsKey_SoundOn",Cache.SoundOn?1:0);
		PlayerPrefs.SetInt("PrefsKey_MusicOn",Cache.MusicOn?1:0);

		Audio.Instance.PlayMusic();
	}

	void Start(){
		soundOn.isOn=Cache.SoundOn;
		musicOn.isOn=Cache.MusicOn;
		sound.value=Cache.SoundVolume;
		music.value=Cache.MusicVolume;
	}

	public void OnSoundVolume(float x){
		var v=sound.value;
		Cache.SoundVolume=v;
	}
	
	public void OnMusicVolume(float x){
		var v=music.value;
		Cache.MusicVolume=v;
	}
	
	public void OnSoundOn(bool x){
		var v=soundOn.isOn;
		Cache.SoundOn=v;
	}

	public void OnMusicOn(bool x){
		var v=musicOn.isOn;
		Cache.MusicOn=v;
	}

	public void OnDismiss(){
	}

	public void OnLogout(){
	}

	public void OnClose(){
		Save();
		Destroy(gameObject);
	}
}
