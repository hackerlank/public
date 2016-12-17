using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class GameIcon : MonoBehaviour {

	public Image	Icon;

	pb_enum game;
	public pb_enum Value{
		set{
			game=value;
			if(GameSprites.Instance!=null && 
			   GameSprites.Instance.dict.ContainsKey(game)){

				var sprite=GameSprites.Instance.dict[game];
				Icon.sprite=sprite;
			}
		}
	}

	public void OnGame(){
		if(LobbyPanel.Instance!=null)
			LobbyPanel.Instance.OnGame(game);
	}
}
