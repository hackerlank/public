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
				Type.text=type2str(value.Type);
				Score.text=PaohuziRule.CalcPoints(value).ToString();
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

	string type2str(pb_enum type){
		switch(type){
		case pb_enum.PhzAa:
			return "将";
		case pb_enum.PhzAbc:
			return "吃";
		case pb_enum.PhzAaawei:
		case pb_enum.PhzAaachou:
			return "偎";
		case pb_enum.PhzAaaa:
		case pb_enum.PhzAaaadesk:
		case pb_enum.PhzAaaastart:
			return "提";
		case pb_enum.PhzBbb:
			return "碰";
		case pb_enum.PhzBbbB:
		case pb_enum.PhzB4B3:
		case pb_enum.PhzBbbbdesk:
			return "跑";
		case pb_enum.PhzAaa:
			return "坎";
		default:
			return "";
		}
	}

	public void OnPUP(){
		if(onTap!=null)onTap.Invoke(this);
	}
}
