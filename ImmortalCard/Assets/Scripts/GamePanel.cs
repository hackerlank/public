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

	IEnumerator Start(){
		while(!CardCache.Ready)yield return null;
	}

	void OnDestroy(){
		Instance=null;
	}

	public IEnumerator Deal(MsgNCStart value){
		//dict
		Configs.Cards=new Dictionary<uint, pawn_t>();
		for(int i=0;i<value.Cards.Count;++i){
			var card=value.Cards[i];
			Configs.Cards[card.Id]=card;
		}
		//sort
		var hands=new List<uint>(value.Hands);
		hands.Sort(delegate(uint x, uint y){
			var cx=Configs.Cards[x];
			var cy=Configs.Cards[y];
			if(cx.Value==1||cx.Value==2)
				x+=54;
			else if(cx.Value==14)
				x+=2;
			if(cy.Value==1||cy.Value==2)
				y+=54;
			else if(cy.Value==14)
				y+=2;

			if(x>y)
				return -1;
			else if(x<y)
				return 1;
			return 0;
		});
		//deal
		string str="start game:\n";
		for(int i=0;i<hands.Count;++i){
			var id=hands[i];
			var v=Configs.Cards[id];
			var fin=false;
			Card.Create(v,HandArea,delegate(Card card) {
				card.Static=false;
				fin=true;
			});
			yield return null;
			str+="("+v.Id+","+v.Color+","+v.Value+"),";
			if((i+1)%6==0)str+="\n";
			while(!fin)yield return null;
		}
		Debug.Log(str);
		yield break;
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
