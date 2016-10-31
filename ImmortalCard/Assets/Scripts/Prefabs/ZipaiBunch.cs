﻿using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class ZipaiBunch : MonoBehaviour {
	public Text		Type;
	public Text		Score;

	public bool ShowType=false;

	public System.Action<ZipaiBunch> onTap=delegate(ZipaiBunch bunch){};

	bunch_t _bunch=null;
	public bunch_t Value{
		get{
			return _bunch;
		}
		set{
			//type and score
			if(ShowType){
				Type.text="K";
				Score.text="9";
				Type.gameObject.SetActive(true);
				Score.gameObject.SetActive(true);
			}
			//cards
			var ctrl=Main.Instance.gameController as GamePanel;
			foreach(var card in value.Pawns){
				Card.Create(ctrl.CardPrefab,card,transform,delegate(Card obj) {
					obj.state=Card.State.ST_MELD;
				});
			}
		}
	}
}
