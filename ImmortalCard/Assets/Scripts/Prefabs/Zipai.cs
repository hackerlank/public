using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;

public class Zipai : Card{

	public GameObject body;
	public Image imageDown;

	public override State state{
		set{
			_state=value;
			var rect=body.transform as RectTransform;
			var w=rect.sizeDelta.x;
			switch(_state){
			case State.ST_NORMAL:
				rect.sizeDelta=new Vector2(w,240);
				break;
			case State.ST_MELD:
				//meld
				rect.sizeDelta=new Vector2(w,54);
				imageDown.gameObject.SetActive(false);
				break;
			case State.ST_ABANDON:
				rect.sizeDelta=new Vector2(w,48);
				imageDown.gameObject.SetActive(false);
				break;
			case State.ST_SELECT:
			case State.ST_DISCARD:
			default:
				break;
			}
		}
		get{
			return _state;
		}
	}

	public override int Value{
		get{
			return _value;
		}set{
			if(Main.Instance.gameController!=null){
				if(value>=1000){
					body.SetActive(true);

					var Clr=(int)value/1000;
					var Val=(int)value%100;
					
					var file=Main.Instance.gameController.Id2File(Clr,Val);
					if(CardCache.Ready&&CardCache.sprites.ContainsKey(file)){
						image.sprite=CardCache.sprites[file];
						imageDown.sprite=image.sprite;
					}
				}else{
					body.SetActive(false);
				}
				_value=value;
			}
		}
	}
}
