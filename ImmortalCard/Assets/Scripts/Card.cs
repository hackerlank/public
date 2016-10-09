using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;

public class Card : MonoBehaviour,IPointerClickHandler,IDragHandler,IBeginDragHandler,IEndDragHandler {

	public LayoutElement le;
	public Image image;

	public enum State{
		ST_NORMAL,
		ST_SELECT,
		ST_DISCARD,
		ST_DEAD
	}
	State _state=State.ST_NORMAL;
	public State state{
		set{
			_state=value;
		}
	}

	bool _static=true;
	public bool Static{
		set{_static=value;}
	}

	public int Id,Clr,Val;	//only for debug
	uint _value;
	public uint Value{
		get{
			return _value;
		}set{
			if(Main.Instance.gameController!=null){
				//remember debug value
				Id=(int)value;
				Clr=(int)value/1000;
				Val=(int)value%100;

				var file=Main.Instance.gameController.Id2File((uint)Clr,(uint)Val);
				if(CardCache.Ready&&CardCache.sprites.ContainsKey(file))
					image.sprite=CardCache.sprites[file];
				_value=value;
				/*
			string str="load card("+value.Id+","+value.Color+","+value.Value+")";
			_value=value;
			//bind image
			string[] Colors={"c","d","h","s"};
			string file="back";
			if(value.Value>=0)
				file=string.Format("{0}{1:00}",Colors[value.Color],value.Value);
			//Debug.Log(str+"file="+file);
			Utils.SpriteCreate(file,delegate(Sprite sprite) {
				image.sprite=sprite;
			});
			*/
			}
		}
	}

	public void Tap(){
		//animation only
		float delta=32f;
		if(_state==State.ST_NORMAL){
			_state=State.ST_SELECT;
			transform.localPosition+=delta*Vector3.up;
		}else if(_state==State.ST_SELECT){
			_state=State.ST_NORMAL;
			transform.localPosition-=delta*Vector3.up;
		}
	}

	public void OnPointerClick (PointerEventData eventData){
		if(_static||eventData==null||eventData.dragging||eventData.pointerEnter==null)return;
		//Debug.Log("----click on card "+eventData.clickCount);
		if(eventData.clickCount==2)
			Main.Instance.gameController.Discard(this);
		else if(eventData.clickCount==1){
			if(_state==State.ST_NORMAL||_state==State.ST_SELECT)
				Main.Instance.gameController.TapCard(this,_state==State.ST_NORMAL);
		}
	}
	
	public void OnBeginDrag (PointerEventData eventData){
	}
	
	public void OnEndDrag (PointerEventData eventData){
		if(_static)return;
		Main.Instance.gameController.Discard(this);
		//Debug.Log("----end drag");
	}
	
	public void OnDrag(PointerEventData eventData){
		if(_static)return;
		//Debug.Log("----drag on card");
		var delta=eventData.delta;
		eventData.pointerDrag.transform.position+=new Vector3(delta.x,delta.y,0);
	}

	public void DiscardTo(Transform group){
		float scalar=(Main.Instance.gameController==null?1f:Main.Instance.gameController.DiscardScalar);
		_static=true;
		transform.SetParent(group);
		transform.localScale=Vector3.one;
		transform.localEulerAngles=Vector3.zero;
		le.minWidth*=scalar;
		le.preferredWidth*=scalar;
		le.minHeight*=scalar;
		le.preferredHeight*=scalar;
	}

	public static void Create(string path,uint data=0,Transform parent=null,System.Action<Card> handler=null){
		if(CardCache.cards[path]){
			var go=GameObject.Instantiate(CardCache.cards[path]) as GameObject;
			go.SetActive(true);
			var card=go.GetComponent<Card>();
			if(card!=null){
				if(data>1000)
					card.Value=data;
				//parent
				if(parent!=null){
					card.transform.SetParent(parent);
					card.transform.localScale=Vector3.one;
				}
			}
			if(handler!=null)
				handler.Invoke(card);
		}
	}
}
