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
					
					var file=Main.Instance.gameController.Rule.Id2File(Clr,Val);
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

	override protected void rearrange(){
		var self=transform;
		var parent=self.parent;
		var area=parent.parent;
		//find the nearest bunch
		var bunches=area.GetComponentsInChildren<ZipaiHandBunch>();
		float dist=1000000;
		ZipaiHandBunch nearest=null;
		foreach(var target in bunches){
			var x=target.transform.position.x;
			var delta=Mathf.Abs(x-transform.position.x);
			if(dist>delta){
				dist=delta;
				nearest=target;
			}
		}
		
		System.Action<Transform> discard=delegate(Transform target){
			if(target!=parent){
				DiscardTo(target,Main.Instance.gameController.Rule.DiscardScalar);
				Static=false;
			}else
				self.localPosition=beginDragLocalPosition;
		};
		var nx=nearest.transform.position.x;
		var nd=Mathf.Abs(nx-transform.position.x);
		if(nd<=(parent.transform as RectTransform).rect.width){
			//inside bunch area
			var children=nearest.GetComponentsInChildren<Card>();
			if(children.Length>=3)
				//but no space
				nearest=null;
		}else{
			//out of bunch area
			if(bunches.Length<11){
				//has bunches space
				Utils.Load<ZipaiHandBunch>(area,delegate(Component obj){
					if(nx>transform.position.x)
						//left side
						obj.transform.SetSiblingIndex(0);
					discard.Invoke(obj.transform);
				});
				return;
			}else
				nearest=null;
		}
		
		if(null==nearest)
			//put back
			self.localPosition=beginDragLocalPosition;
		else
			//reparent
			discard.Invoke(nearest.transform);
	}
}
