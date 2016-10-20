using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;

public class Card : MonoBehaviour,IDragHandler,IEndDragHandler
		,IPointerEnterHandler,IPointerDownHandler,IPointerUpHandler{

	public LayoutElement le;
	public Image image;

	public enum State{
		ST_NORMAL,
		ST_SELECT,
		ST_DISCARD,
		ST_DEAD
	}
	protected State _state=State.ST_NORMAL;
	public virtual State state{
		set{
			_state=value;
		}
		get{
			return _state;
		}
	}

	protected bool _static=true;
	public bool Static{
		set{_static=value;}
	}

	public int _value;	//only for debug
	public virtual int Value{
		get{
			return _value;
		}set{
			if(Main.Instance.gameController!=null){
				var Clr=(int)value/1000;
				var Val=(int)value%100;

				var file=Main.Instance.gameController.Id2File(Clr,Val);
				if(CardCache.Ready&&CardCache.sprites.ContainsKey(file))
					image.sprite=CardCache.sprites[file];
				_value=value;
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

	public void OnPointerDown (PointerEventData eventData){
		if(_static||(_state!=State.ST_NORMAL&&_state!=State.ST_SELECT))return;
		
		var panel=Main.Instance.gameController as GamePanel;
		if(panel!=null){
			panel.OnPointerDown(null);
			panel.TapCard(this,_state!=State.ST_SELECT);
		}
	}

	public void OnPointerUp (PointerEventData eventData){
		if(_static||(_state!=State.ST_NORMAL&&_state!=State.ST_SELECT))return;

		if(eventData.clickCount>=2)
			Main.Instance.StartCoroutine(Main.Instance.gameController.Discard(this));
		var panel=Main.Instance.gameController as GamePanel;
		if(panel!=null)
			panel.OnPointerUp(null);
	}

	public void OnPointerEnter (PointerEventData eventData){
		if(_static||(_state!=State.ST_NORMAL&&_state!=State.ST_SELECT)||Main.Instance.gameController.CardDrag)return;

		var panel=Main.Instance.gameController as GamePanel;
		panel.OnCardEnter(this);
	}

	public void OnEndDrag (PointerEventData eventData){
		if(_static||(_state!=State.ST_NORMAL&&_state!=State.ST_SELECT)||!Main.Instance.gameController.CardDrag)return;
		Main.Instance.StartCoroutine(Main.Instance.gameController.Discard(this));
	}
	
	public void OnDrag(PointerEventData eventData){
		if(_static||(_state!=State.ST_NORMAL&&_state!=State.ST_SELECT)||!Main.Instance.gameController.CardDrag)return;
		var delta=eventData.delta;
		eventData.pointerDrag.transform.position+=new Vector3(delta.x,delta.y,0);
	}

	public void DiscardTo(Transform group,float scalar){
		_static=true;
		transform.SetParent(group);
		transform.localScale=Vector3.one;
		transform.localEulerAngles=Vector3.zero;
		le.minWidth*=scalar;
		le.preferredWidth*=scalar;
		le.minHeight*=scalar;
		le.preferredHeight*=scalar;
	}

	public static void Create(string path,int data=0,Transform parent=null,System.Action<Card> handler=null){
		if(CardCache.cards[path]){
			var go=GameObject.Instantiate(CardCache.cards[path]) as GameObject;
			go.SetActive(true);
			var card=go.GetComponent<Card>();
			if(card!=null){
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
