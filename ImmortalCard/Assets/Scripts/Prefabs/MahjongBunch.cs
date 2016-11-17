using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;
using Proto3;

public class MahjongBunch : MonoBehaviour{
	public Transform	Cards;
	public Card			ExtraCard;

	public System.Action<MahjongBunch> onTap=delegate(MahjongBunch bunch){};

	bunch_t _bunch=null;
	public bunch_t Value{
		get{
			return _bunch;
		}
		set{
			_bunch=value;
			if(value.Pawns.Count<=0)return;
			//cards
			var prefab=(Main.Instance.gameController as GamePanel).CardPrefab;
			var start=0;
			if(value.Type==pb_enum.BunchAaaa){
				++start;
				ExtraCard.Value=value.Pawns[0];
				ExtraCard.state=Card.State.ST_MELD;
			}
			for(int i=start;i<value.Pawns.Count;++i){
				var card=value.Pawns[i];
				Card.Create(prefab,card,Cards,delegate(Card obj) {
					obj.state=Card.State.ST_MELD;
				});
			}
		}
	}

	public void OnPUP(){
		if(onTap!=null)onTap.Invoke(this);
	}
}
