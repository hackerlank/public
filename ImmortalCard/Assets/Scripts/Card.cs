using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;

public class Card : MonoBehaviour,IPointerClickHandler,IDragHandler,IBeginDragHandler,IEndDragHandler {

	public LayoutElement le;
	public Image image;

	bool _static=true;
	public bool Static{
		set{_static=value;}
	}

	Proto3.pawn_t _value;
	public Proto3.pawn_t Value{
		get{
			return _value;
		}set{
			_value=value;
			//bind image
		}
	}

	public void OnPointerClick (PointerEventData eventData){
		if(_static||eventData==null||eventData.dragging||eventData.pointerEnter==null)return;
		Debug.Log("----click on card "+eventData.clickCount);
		if(eventData.clickCount==2)
			DiscardTo(GamePanel.Instance.DiscardAreas[0],0.625f);
	}
	
	//bool dragging=false;
	//Vector2 dragFrom=Vector2.zero;
	public void OnBeginDrag (PointerEventData eventData){
		//dragging=true;
		//dragFrom=transform.position;
		//Debug.Log("----begin from "+dragFrom.ToString());
	}
	
	public void OnEndDrag (PointerEventData eventData){
		if(_static)return;
		//dragging=false;
		//restrore
		//transform.position=dragFrom;
		DiscardTo(GamePanel.Instance.DiscardAreas[0],0.625f);
		//Debug.Log("----end drag");
	}
	
	public void OnDrag(PointerEventData eventData){
		if(_static)return;
		//Debug.Log("----drag on card");
		var delta=eventData.delta;
		eventData.pointerDrag.transform.position+=new Vector3(delta.x,delta.y,0);
	}

	public void DiscardTo(Transform group,float scalar=1f){
		_static=true;
		transform.SetParent(group);
		transform.localScale=Vector3.one;
		le.minWidth*=scalar;
		le.preferredWidth*=scalar;
		le.minHeight*=scalar;
		le.preferredHeight*=scalar;
	}

	public static void Create(System.Action<Card> handler){
		Utils.Load<Card>(null,delegate(Component comp) {
			Utils.SpriteCreate("h01",delegate(Sprite sprite) {
				var card=comp as Card;
				card.image.sprite=sprite;
				if(handler!=null)handler.Invoke(card);
			});
		});
	}
}
