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
			switch(value.id){
			case pb_enum.GameMj:
				Name.text="Mahjong";	break;
			case pb_enum.GameDdz:
				Name.text="DoudeZhu";	break;
			case pb_enum.GamePhz:
			default:
				Name.text="Paohuzi";	break;
			}
		}
	}

	public void OnGame(){
		if(LobbyPanel.Instance!=null)
			LobbyPanel.Instance.OnGame(game);
	}
}
