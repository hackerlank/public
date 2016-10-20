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
			var grid=GetComponentInParent<GridLayoutGroup>();
			var rect=transform as RectTransform;
			switch(_state){
			case State.ST_NORMAL:
				if(grid!=null){
					grid.cellSize=new Vector2(86,78);
					rect.sizeDelta=new Vector2(86,240);
				}
				break;
			case State.ST_SELECT:
				break;
			case State.ST_DISCARD:
				//meld
				if(grid!=null){
					grid.cellSize=new Vector2(54,48);
					rect.sizeDelta=new Vector2(54,54);
					imageDown.gameObject.SetActive(false);
				}
				break;
			case State.ST_DEAD:
				if(grid!=null){
					grid.cellSize=new Vector2(48,48);
					rect.sizeDelta=new Vector2(48,48);
					imageDown.gameObject.SetActive(false);
				}
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
