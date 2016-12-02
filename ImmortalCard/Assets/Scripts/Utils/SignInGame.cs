using UnityEngine;
using System.Collections;

public class SignInGame : MonoBehaviour {

	// Use this for initialization
	IEnumerator Start () {
		var limited=(0==int.Parse(Config.limited));
		if(!limited){
			//if not account disable: sign in,cache and update to server
			yield return new WaitForSeconds(5);

			//always sign in to find the new user
			yield return StartCoroutine(Main.Instance.share.SignIn(delegate(string account) {
				//cache account for next time login
				PlayerPrefs.SetString(Cache.PrefsKey_Account,account);
			}));
		}

		//destroy when finish
		Destroy(this);
	}
}
