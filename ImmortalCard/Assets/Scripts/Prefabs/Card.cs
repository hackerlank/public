using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;

public class Card : MonoBehaviour,IDragHandler,IEndDragHandler,IBeginDragHandler
		,IPointerEnterHandler,IPointerDownHandler,IPointerUpHandler{

	public LayoutElement le;
	public Image tint;
	public Image image;

	public enum State{
		ST_NORMAL,
		ST_SELECT,
		ST_DISCARD,
		ST_MELD,
		ST_ABANDON
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
				if(value>1000){
					var Clr=(int)value/1000;
					var Val=(int)value%100;
					
					var file=Main.Instance.gameController.Id2File(Clr,Val);
					if(CardCache.Ready&&CardCache.sprites.ContainsKey(file))
						image.sprite=CardCache.sprites[file];
				}
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

		var panel=Main.Instance.gameController as GamePanel;
		if(panel!=null){
			if(eventData.clickCount>=2 && panel.VerifyDiscard(this))
				Main.Instance.StartCoroutine(Main.Instance.gameController.Discard(this));
			panel.OnPointerUp(null);
		}
	}

	public void OnPointerEnter (PointerEventData eventData){
		if(_static||(_state!=State.ST_NORMAL&&_state!=State.ST_SELECT)||Main.Instance.gameController.CardDrag)return;

		var panel=Main.Instance.gameController as GamePanel;
		panel.OnCardEnter(this);
	}

	Vector3 beginDragLocalPosition=Vector3.zero;
	public void OnBeginDrag (PointerEventData eventData){
		if(_static||(_state!=State.ST_NORMAL&&_state!=State.ST_SELECT)||!Main.Instance.gameController.CardDrag)return;

		beginDragLocalPosition=eventData.pointerDrag.transform.localPosition;
	}

	public void OnEndDrag (PointerEventData eventData){
		if(_static||(_state!=State.ST_NORMAL&&_state!=State.ST_SELECT)||!Main.Instance.gameController.CardDrag)return;

		var self=eventData.pointerDrag.transform;
		var parent=self.parent;
		var area=parent.parent;
		var rect=(area as RectTransform).rect;

		var panel=Main.Instance.gameController as GamePanel;
		if(panel==null)return;

		if(self.localPosition.y>rect.height+64 && panel.VerifyDiscard(this))
			//pass the line,discard
			Main.Instance.StartCoroutine(Main.Instance.gameController.Discard(this));
		else{
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
					DiscardTo(target,Main.Instance.gameController.DiscardScalar);
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
	
	public void OnDrag(PointerEventData eventData){
		if(_static||(_state!=State.ST_NORMAL&&_state!=State.ST_SELECT)||!Main.Instance.gameController.CardDrag)return;
		var delta=eventData.delta;
		eventData.pointerDrag.transform.position+=new Vector3(delta.x,delta.y,0);
	}

	public void DiscardTo(Transform group,float scalar){
		var parent=GetComponentInParent<ZipaiHandBunch>();

		_static=true;
		transform.SetParent(group);
		transform.localScale=Vector3.one;
		transform.localEulerAngles=Vector3.zero;
		le.minWidth*=scalar;
		le.preferredWidth*=scalar;
		le.minHeight*=scalar;
		le.preferredHeight*=scalar;

		if(parent==null)return;
		var sibling=parent.GetComponentsInChildren<Card>().Length;
		if(sibling<=0)
			Destroy(parent.gameObject);
	}

	public void RemoveFromHand(){
		var parent=GetComponentInParent<ZipaiHandBunch>();
		if(parent==null)return;
		var sibling=parent.GetComponentsInChildren<Card>().Length;
		if(sibling<=1)
			Destroy(parent.gameObject);
		DestroyImmediate(gameObject);
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
					card.transform.localEulerAngles=Vector3.zero;
				}
			}
			if(handler!=null)
				handler.Invoke(card);
		}
	}
}
