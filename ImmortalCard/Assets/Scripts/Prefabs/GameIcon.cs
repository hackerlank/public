using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class GameIcon : MonoBehaviour {

	public Image	Icon;
	public Text		Name;

	game_t game;
	public game_t Value{
		set{
			game=value;
			Name.text=id2name(game.Id);
		}
	}

	public void OnGame(){
		if(LobbyPanel.Instance!=null)
			LobbyPanel.Instance.OnGame(game);
	}

	string id2name(pb_enum id){
		switch(id){
		case pb_enum.GameMj:
			return "麻将";
		case pb_enum.GameDdz:
			return "斗地主";
		case pb_enum.GamePhz:
			return "跑胡子";
		default:
			return "???";
		}
	}
}
