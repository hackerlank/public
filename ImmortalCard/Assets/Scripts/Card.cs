using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;

public class Card : MonoBehaviour,IPointerClickHandler,IDragHandler,IBeginDragHandler,IEndDragHandler {

	public LayoutElement le;

	void Start () {
	
	}
	
	void Update () {
	
	}

	public void OnPointerClick (PointerEventData eventData){
		if(eventData==null||eventData.dragging||eventData.pointerEnter==null)return;
		Debug.Log("----click on card "+eventData.clickCount);
		if(eventData.clickCount==2)
			DiscardTo(GamePanel.Instance.LDiscardArea,0.66666f);
	}
	
	bool dragging=false;
	Vector2 dragFrom=Vector2.zero;
	public void OnBeginDrag (PointerEventData eventData){
		dragging=true;
		dragFrom=transform.position;
		//Debug.Log("----begin from "+dragFrom.ToString());
	}
	
	public void OnEndDrag (PointerEventData eventData){
		dragging=false;
		var lg=transform.parent.GetComponent<HorizontalLayoutGroup>();
		lg.CalculateLayoutInputVertical();
		lg.CalculateLayoutInputHorizontal();

		//restrore
		//transform.position=dragFrom;
		DiscardTo(GamePanel.Instance.LDiscardArea,0.66666f);
		//Debug.Log("----end drag");
	}
	
	public void OnDrag(PointerEventData eventData){
		//Debug.Log("----drag on card");
		var delta=eventData.delta;
		eventData.pointerDrag.transform.position+=new Vector3(delta.x,delta.y,0);
	}

	public void DiscardTo(Transform group,float scalar=1f){
		transform.SetParent(group);
		transform.localScale=Vector3.one;
		le.minWidth*=scalar;
		le.preferredWidth*=scalar;
		le.minHeight*=scalar;
		le.preferredHeight*=scalar;
	}
}
