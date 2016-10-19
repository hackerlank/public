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
		game_t game=new game_t();
		int total=8;
		game.Id=(int)pb_enum.GameDdz;
		addGame(game);
		game=new game_t();
		game.Id=(int)pb_enum.GameMj;
		addGame(game);
		for(int i=0;i<total-2;++i){
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

	public void OnIcon(){
	}
	
	public void OnCurrency(){
	}
	
	public void OnMail(){
		Main.Instance.share.Share("Title","Hello Wechat!",cn.sharesdk.unity3d.ContentType.Image);
	}
	
	public void OnSettings(){
	}
	
	public void OnProxy(){
		Utils.Load<ChargePanel>(gameObject.transform.parent);
	}
	
	public void OnShare(){
		Main.Instance.share.Share("Title","Hello Wechat!",cn.sharesdk.unity3d.ContentType.Webpage,"http://www.baidu.com");
	}
}
