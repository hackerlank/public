using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class LobbyPanel : MonoBehaviour {

	public Text			Bulletin;
	public Transform	GameRoot;

	public static LobbyPanel Instance=null;
	void Awake(){Instance=this;}
	void OnDestroy(){Instance=null;}

	void Start(){
		game_t game=new game_t();
		game.Id=(int)pb_enum.GameDdz;
		addGame(game);
		game=new game_t();
		game.Id=(int)pb_enum.GameMj;
		addGame(game);
		for(int i=0;i<10;++i){
			game=new game_t();
			game.Id=(int)pb_enum.GameMj;
			addGame(game);
		}
	}

	void addGame(game_t game){
		Utils.Load<GameIcon>(GameRoot,delegate(Component obj){
			var icon=obj as GameIcon;
			icon.GameId=(pb_enum)game.Id;
			icon.Name.text=icon.GameId.ToString();
		});
	}
}
