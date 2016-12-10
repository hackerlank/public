using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

public class Cache{
	public const string PrefsKey_Account		="PrefsKey_Account";
	public const string PrefsKey_DefinedCards	="PrefsKey_DefinedCards";
	public const string PrefsKey_StoreGame		="PrefsKey_StoreGame";

	public const string PrefsKey_SoundVolume	="PrefsKey_SoundVolume";
	public const string PrefsKey_MusicVolume	="PrefsKey_MusicVolume";
	public const string PrefsKey_SoundOn		="PrefsKey_SoundOn";
	public const string PrefsKey_MusicOn		="PrefsKey_MusicOn";


	public static float SoundVolume=0.5f;
	public static float MusicVolume=0.5f;
	public static bool SoundOn=true;
	public static bool MusicOn=true;
}

