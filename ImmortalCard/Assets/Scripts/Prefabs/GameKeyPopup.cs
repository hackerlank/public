using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class GameKeyPopup : MonoBehaviour {
	public Text textKey;

	public void OnEnter(){
		Debug.Log("----game id="+textKey.text);
		try{
			var gameId=int.Parse(textKey.text);
			Main.Instance.StartCoroutine(Main.Instance.MainPlayer.JoinGame(gameId));
			Destroy(gameObject);
		}catch(System.Exception){
			Debug.LogError("Invalid key");
		}
	}

	public void OnClose(){
		Destroy(gameObject);
	}
}
