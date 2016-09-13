using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class GamePanel : MonoBehaviour {
	[HideInInspector]
	public int			N=3;
	public List<Card>[]	HandCards;
	public Card[]		BottomCards;

	public Transform	HandArea;
	public Transform[]	DiscardAreas;	//MRL
	public PlayerIcon[]	Players;
	public Text[]		nHandCards;
	public Text			Ante,Multiples,Infomation;

	public static GamePanel	Instance=null;
	void Awake(){
		Players=new PlayerIcon[N];

		Instance=this;
	}

	void OnDestroy(){
		Instance=null;
	}

	public MsgNCStart Data{
		set{
			Configs.Cards=new Dictionary<int, pawn_t>();
			for(int i=0;i<value.Cards.Count;++i){
				var card=value.Cards[i];
				Configs.Cards[card.Id]=card;
			}
			string str="start game:\n";
			for(int i=0;i<value.Hands.Count;++i){
				var id=value.Hands[i];
				var v=Configs.Cards[id];
				Card.Create(v,HandArea,delegate(Card card) {
					card.Static=false;
				});
				str+="("+v.Id+","+v.Color+","+v.Value+"),";
				if((i+1)%6==0)str+="\n";
			}
			Debug.Log(str);
		}
	}

	public void OnExit(){
		Utils.Load<CreatePanel>(gameObject.transform.parent,delegate(Component obj) {
			Destroy(gameObject);
		});
	}

	public static void Create(System.Action<Component> handler=null){
		Utils.Load<GamePanel>(Main.Instance.transform,delegate(Component obj){
			if(handler!=null)handler.Invoke(obj);
		});
	}
}
