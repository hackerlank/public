using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class LobbyPanel : MonoBehaviour {

	public Text			Bulletin;
	public Transform	GameRoot;
	public ContentSizeFitter	Content;

	public static LobbyPanel Instance=null;
	void Awake(){Instance=this;}
	void OnDestroy(){Instance=null;}

	void Start(){
		var games=new pb_enum[]{pb_enum.GameDdz,pb_enum.GameMj,pb_enum.GamePhz};
		foreach(var id in games){
			game_t game=new game_t();
			game.Id=(int)id;
			addGame(game);
		}
	}

	void addGame(game_t game){
		Utils.Load<GameIcon>(GameRoot,delegate(Component obj){
			var icon=obj as GameIcon;
			icon.game=(pb_enum)game.Id;
			switch(icon.game){
			case pb_enum.GameMj:
				icon.Name.text="Mahjong";	break;
			case pb_enum.GameDdz:
				icon.Name.text="DoudeZhu";	break;
			case pb_enum.GamePhz:
			default:
				icon.Name.text="Paohuzi";	break;
			}
		});
	}

	public void OnIcon(){
	}
	
	public void OnCurrency(){
	}
	
	public void OnMail(){
		Main.Instance.share.Share("Share","I'm fun!",cn.sharesdk.unity3d.ContentType.Image);
	}
	
	public void OnSettings(){
	}
	
	public void OnProxy(){
		Utils.Load<ChargePanel>(Main.Instance.RootPanel);
	}
	
	public void OnShare(){
		Main.Instance.share.Share("Share App","Hello Player!",cn.sharesdk.unity3d.ContentType.Webpage,"http://www.baidu.com");
	}
}
