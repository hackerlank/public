using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;
using Proto3;

public class ZipaiHandBunch : MonoBehaviour{

	bunch_t _bunch=null;
	public bunch_t Value{
		get{
			return _bunch;
		}
		set{
			_bunch=value;
			//cards
			var ctrl=Main.Instance.gameController as GamePanel;
			foreach(var card in value.Pawns){
				Card.Create(ctrl.CardPrefab,card,transform,delegate(Card obj) {
					obj.state=Card.State.ST_NORMAL;
				});
			}
		}
	}
	public void Add(int card,bool bStatic=false){
		//cards
		var ctrl=Main.Instance.gameController as GamePanel;
		Card.Create(ctrl.CardPrefab,card,transform,delegate(Card obj) {
			obj.state=Card.State.ST_NORMAL;
			obj.Static=bStatic;
			if(bStatic)
				obj.tint.color=Color.gray;
		});
	}
}
