using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;
using Proto3;

public class MahjongBunch : Bunch{
	public Card				ExtraCard;

	override public bunch_t Value{
		get{
			return _bunch;
		}
		set{
			_bunch=value;
			if(value.Pawns.Count<=0)return;
			//cards
			var prefab=(Main.Instance.gameController as GamePanel).Rule.CardPrefab;
			var start=0;
			if(value.Type==pb_enum.BunchAaaa){
				++start;
				ExtraCard.Value=value.Pawns[0];
				ExtraCard.state=Card.State.ST_MELD;
				ExtraCard.gameObject.SetActive(true);
			}
			for(int i=start;i<value.Pawns.Count;++i){
				var card=value.Pawns[i];
				Card.Create(prefab,card,Cards,delegate(Card obj) {
					obj.state=Card.State.ST_MELD;
					if(obj.Value<1000)obj.tint.color=Color.clear;
				});
			}
		}
	}
}
