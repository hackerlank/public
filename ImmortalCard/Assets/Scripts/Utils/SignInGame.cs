using UnityEngine;
using System.Collections;

public class SignInGame : MonoBehaviour {

	// Use this for initialization
	IEnumerator Start () {
		//if not account disable: sign in,cache and update to server
		yield return new WaitForSeconds(5);

		var udid=SystemInfo.deviceUniqueIdentifier;
		var account=PlayerPrefs.GetString(Cache.PrefsKey_Account);
		if(udid==account){
			//need sign up
		}

		//destroy when finish
		Destroy(this);
	}
}
