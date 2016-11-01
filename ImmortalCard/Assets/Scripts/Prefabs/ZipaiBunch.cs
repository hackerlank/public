using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;
using Proto3;

public class ZipaiBunch : MonoBehaviour{
	public Text			Type;
	public Text			Score;
	public Transform	Cards;

	public bool ShowType=false;

	public System.Action<ZipaiBunch> onTap=delegate(ZipaiBunch bunch){};

	bunch_t _bunch=null;
	public bunch_t Value{
		get{
			return _bunch;
		}
		set{
			_bunch=value;
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
				Card.Create(ctrl.CardPrefab,card,Cards,delegate(Card obj) {
					obj.state=Card.State.ST_MELD;
				});
			}
		}
	}

	public void OnPUP(){
		if(onTap!=null)onTap.Invoke(this);
	}
}
