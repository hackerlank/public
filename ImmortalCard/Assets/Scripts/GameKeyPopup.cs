﻿using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class GameKeyPopup : MonoBehaviour {
	public Text textKey;

	public void OnEnter(){
		Debug.Log("----game id="+textKey.text);
		try{
			var gameId=uint.Parse(textKey.text);
			if(CreatePanel.Instance)CreatePanel.Instance.Connect(gameId);
			Destroy(gameObject);
		}catch(System.Exception){
			Debug.LogError("Invalid key");
		}
	}
}
